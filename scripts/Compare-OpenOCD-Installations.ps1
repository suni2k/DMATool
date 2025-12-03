# Compare OpenOCD installations to find missing files
# This compares the fresh download with your working copy

param(
    [Parameter(Mandatory=$false)]
    [string]$SourceDir = "C:\Users\suni\Desktop\dma\CH347FPGATool\OpenOCD_CH347",
    
    [Parameter(Mandatory=$false)]
    [string]$DestDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\OpenOCD_CH347"
)

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " OpenOCD Installation Comparison" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $SourceDir)) {
    Write-Host "ERROR: Source directory not found: $SourceDir" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $DestDir)) {
    Write-Host "ERROR: Destination directory not found: $DestDir" -ForegroundColor Red
    exit 1
}

Write-Host "Comparing directories..." -ForegroundColor Yellow
Write-Host "Source:      $SourceDir" -ForegroundColor Cyan
Write-Host "Destination: $DestDir" -ForegroundColor Cyan
Write-Host ""

# Get all files from source
$sourceFiles = Get-ChildItem -Path $SourceDir -Recurse -File | ForEach-Object {
    [PSCustomObject]@{
        RelativePath = $_.FullName.Substring($SourceDir.Length + 1)
        FullPath = $_.FullName
        Size = $_.Length
    }
}

# Get all files from destination
$destFiles = Get-ChildItem -Path $DestDir -Recurse -File | ForEach-Object {
    [PSCustomObject]@{
        RelativePath = $_.FullName.Substring($DestDir.Length + 1)
        FullPath = $_.FullName
        Size = $_.Length
    }
}

# Find missing files
$missingFiles = @()
$sizeMismatchFiles = @()
$matchingFiles = 0

foreach ($sourceFile in $sourceFiles) {
    $destFile = $destFiles | Where-Object { $_.RelativePath -eq $sourceFile.RelativePath }
    
    if (-not $destFile) {
        $missingFiles += $sourceFile
    } elseif ($destFile.Size -ne $sourceFile.Size) {
        $sizeMismatchFiles += [PSCustomObject]@{
            Path = $sourceFile.RelativePath
            SourceSize = $sourceFile.Size
            DestSize = $destFile.Size
        }
    } else {
        $matchingFiles++
    }
}

# Report missing files
if ($missingFiles.Count -gt 0) {
    Write-Host "Missing Files ($($missingFiles.Count)):" -ForegroundColor Red
    Write-Host ""
    
    # Group by directory
    $missingFiles | Group-Object { Split-Path $_.RelativePath -Parent } | ForEach-Object {
        Write-Host "  $($_.Name)\" -ForegroundColor Yellow
        $_.Group | ForEach-Object {
            Write-Host "    - $(Split-Path $_.RelativePath -Leaf)" -ForegroundColor Red
        }
        Write-Host ""
    }
} else {
    Write-Host "? No missing files!" -ForegroundColor Green
    Write-Host ""
}

# Report size mismatches
if ($sizeMismatchFiles.Count -gt 0) {
    Write-Host "Size Mismatches ($($sizeMismatchFiles.Count)):" -ForegroundColor Yellow
    Write-Host ""
    
    $sizeMismatchFiles | ForEach-Object {
        Write-Host "  $($_.Path)" -ForegroundColor Yellow
        Write-Host "    Source: $($_.SourceSize) bytes" -ForegroundColor Gray
        Write-Host "    Dest:   $($_.DestSize) bytes" -ForegroundColor Gray
    }
    Write-Host ""
}

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Total files in source:      $($sourceFiles.Count)" -ForegroundColor White
Write-Host "  Total files in destination: $($destFiles.Count)" -ForegroundColor White
Write-Host "  Matching files:             $matchingFiles" -ForegroundColor Green
Write-Host "  Missing files:              $($missingFiles.Count)" -ForegroundColor $(if ($missingFiles.Count -gt 0) { "Red" } else { "Green" })
Write-Host "  Size mismatches:            $($sizeMismatchFiles.Count)" -ForegroundColor $(if ($sizeMismatchFiles.Count -gt 0) { "Yellow" } else { "Green" })
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($missingFiles.Count -eq 0 -and $sizeMismatchFiles.Count -eq 0) {
    Write-Host "? Your OpenOCD installation is complete!" -ForegroundColor Green
} else {
    Write-Host "? Your OpenOCD installation has differences" -ForegroundColor Yellow
    
    if ($missingFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "To copy missing files, run:" -ForegroundColor Yellow
        Write-Host "  robocopy `"$SourceDir`" `"$DestDir`" /E /XC /XN /XO" -ForegroundColor White
    }
}
