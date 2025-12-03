# Full CH347FPGATool Comparison
# Compares the entire fresh download with your working copy

param(
    [Parameter(Mandatory=$false)]
    [string]$SourceDir = "C:\Users\suni\Desktop\dma\CH347FPGATool",
    
    [Parameter(Mandatory=$false)]
    [string]$DestDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
)

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Full CH347FPGATool Comparison" -ForegroundColor Cyan
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

Write-Host "Analyzing directories..." -ForegroundColor Yellow
Write-Host "Source:      $SourceDir" -ForegroundColor Cyan
Write-Host "Destination: $DestDir" -ForegroundColor Cyan
Write-Host ""

# Exclude patterns (things we don't need to sync)
$excludePatterns = @(
    '*.log',
    '*.tmp',
    'Thumbs.db',
    '.DS_Store',
    '__pycache__',
    '*.pyc',
    '.git',
    '.gitignore'
)

function Test-ShouldExclude {
    param([string]$Path)
    
    foreach ($pattern in $excludePatterns) {
        if ($Path -like $pattern) {
            return $true
        }
    }
    return $false
}

# Get all files from source (excluding patterns)
Write-Host "Scanning source directory..." -ForegroundColor Yellow
$sourceFiles = Get-ChildItem -Path $SourceDir -Recurse -File | Where-Object {
    -not (Test-ShouldExclude $_.Name)
} | ForEach-Object {
    [PSCustomObject]@{
        RelativePath = $_.FullName.Substring($SourceDir.Length + 1)
        FullPath = $_.FullName
        Size = $_.Length
        Extension = $_.Extension
    }
}

# Get all files from destination (excluding patterns)
Write-Host "Scanning destination directory..." -ForegroundColor Yellow
$destFiles = Get-ChildItem -Path $DestDir -Recurse -File | Where-Object {
    -not (Test-ShouldExclude $_.Name)
} | ForEach-Object {
    [PSCustomObject]@{
        RelativePath = $_.FullName.Substring($DestDir.Length + 1)
        FullPath = $_.FullName
        Size = $_.Length
        Extension = $_.Extension
    }
}

Write-Host ""

# Analyze differences
$missingFiles = @()
$sizeMismatchFiles = @()
$matchingFiles = 0
$extraFiles = @()

# Check for missing and mismatched files
foreach ($sourceFile in $sourceFiles) {
    $destFile = $destFiles | Where-Object { $_.RelativePath -eq $sourceFile.RelativePath }
    
    if (-not $destFile) {
        $missingFiles += $sourceFile
    } elseif ($destFile.Size -ne $sourceFile.Size) {
        $sizeMismatchFiles += [PSCustomObject]@{
            Path = $sourceFile.RelativePath
            SourceSize = $sourceFile.Size
            DestSize = $destFile.Size
            Diff = $destFile.Size - $sourceFile.Size
        }
    } else {
        $matchingFiles++
    }
}

# Check for extra files (in destination but not in source)
foreach ($destFile in $destFiles) {
    $sourceFile = $sourceFiles | Where-Object { $_.RelativePath -eq $destFile.RelativePath }
    
    if (-not $sourceFile) {
        $extraFiles += $destFile
    }
}

# Report missing files (grouped by category)
if ($missingFiles.Count -gt 0) {
    Write-Host "Missing Files ($($missingFiles.Count)):" -ForegroundColor Red
    Write-Host ""
    
    # Group by top-level directory
    $missingFiles | Group-Object { $_.RelativePath.Split('\')[0] } | Sort-Object Name | ForEach-Object {
        Write-Host "  [$($_.Name)] ($($_.Count) files)" -ForegroundColor Yellow
        
        # Show first 10 files in each category
        $_.Group | Select-Object -First 10 | ForEach-Object {
            $size = if ($_.Size -lt 1KB) { "$($_.Size) B" } 
                    elseif ($_.Size -lt 1MB) { "{0:N1} KB" -f ($_.Size / 1KB) }
                    else { "{0:N1} MB" -f ($_.Size / 1MB) }
            Write-Host "    - $($_.RelativePath) ($size)" -ForegroundColor Red
        }
        
        if ($_.Count -gt 10) {
            Write-Host "    ... and $($_.Count - 10) more files" -ForegroundColor Gray
        }
        Write-Host ""
    }
} else {
    Write-Host "? No missing files!" -ForegroundColor Green
    Write-Host ""
}

# Report size mismatches (top 20)
if ($sizeMismatchFiles.Count -gt 0) {
    Write-Host "Size Mismatches ($($sizeMismatchFiles.Count)):" -ForegroundColor Yellow
    Write-Host ""
    
    $sizeMismatchFiles | Sort-Object { [Math]::Abs($_.Diff) } -Descending | Select-Object -First 20 | ForEach-Object {
        $srcSize = if ($_.SourceSize -lt 1KB) { "$($_.SourceSize) B" }
                   elseif ($_.SourceSize -lt 1MB) { "{0:N1} KB" -f ($_.SourceSize / 1KB) }
                   else { "{0:N1} MB" -f ($_.SourceSize / 1MB) }
        
        $dstSize = if ($_.DestSize -lt 1KB) { "$($_.DestSize) B" }
                   elseif ($_.DestSize -lt 1MB) { "{0:N1} KB" -f ($_.DestSize / 1KB) }
                   else { "{0:N1} MB" -f ($_.DestSize / 1MB) }
        
        Write-Host "  $($_.Path)" -ForegroundColor Yellow
        Write-Host "    Source: $srcSize | Dest: $dstSize" -ForegroundColor Gray
    }
    
    if ($sizeMismatchFiles.Count -gt 20) {
        Write-Host ""
        Write-Host "  ... and $($sizeMismatchFiles.Count - 20) more mismatches" -ForegroundColor Gray
    }
    Write-Host ""
}

# Report extra files (in destination but not source)
if ($extraFiles.Count -gt 0) {
    Write-Host "Extra Files (in destination but not source): $($extraFiles.Count)" -ForegroundColor Cyan
    Write-Host "(These might be auto-generated or from older versions)" -ForegroundColor Gray
    Write-Host ""
}

# Calculate total sizes
$totalSourceSize = ($sourceFiles | Measure-Object -Property Size -Sum).Sum
$totalDestSize = ($destFiles | Measure-Object -Property Size -Sum).Sum
$missingSize = ($missingFiles | Measure-Object -Property Size -Sum).Sum

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Total files in source:      $($sourceFiles.Count) ($("{0:N1} MB" -f ($totalSourceSize / 1MB)))" -ForegroundColor White
Write-Host "  Total files in destination: $($destFiles.Count) ($("{0:N1} MB" -f ($totalDestSize / 1MB)))" -ForegroundColor White
Write-Host "  Matching files:             $matchingFiles" -ForegroundColor Green
Write-Host "  Missing files:              $($missingFiles.Count) ($("{0:N1} MB" -f ($missingSize / 1MB)))" -ForegroundColor $(if ($missingFiles.Count -gt 0) { "Red" } else { "Green" })
Write-Host "  Size mismatches:            $($sizeMismatchFiles.Count)" -ForegroundColor $(if ($sizeMismatchFiles.Count -gt 0) { "Yellow" } else { "Green" })
Write-Host "  Extra files:                $($extraFiles.Count)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Recommendations
if ($missingFiles.Count -eq 0 -and $sizeMismatchFiles.Count -eq 0) {
    Write-Host "? Your installation is complete!" -ForegroundColor Green
} else {
    Write-Host "Recommendations:" -ForegroundColor Yellow
    Write-Host ""
    
    if ($missingFiles.Count -gt 0) {
        Write-Host "  1. Critical missing files detected!" -ForegroundColor Red
        Write-Host "     Run this to copy all missing files:" -ForegroundColor Yellow
        Write-Host "     robocopy `"$SourceDir`" `"$DestDir`" /E /XC /XN /XO /R:0 /W:0" -ForegroundColor White
        Write-Host ""
    }
    
    if ($sizeMismatchFiles.Count -gt 0) {
        Write-Host "  2. Size mismatches detected (possibly newer versions)" -ForegroundColor Yellow
        Write-Host "     To update mismatched files:" -ForegroundColor Yellow
        Write-Host "     robocopy `"$SourceDir`" `"$DestDir`" /E /R:0 /W:0" -ForegroundColor White
        Write-Host ""
    }
}

# Ask if user wants details on a specific category
if ($missingFiles.Count -gt 0 -or $sizeMismatchFiles.Count -gt 0) {
    Write-Host "For detailed analysis of specific file types, you can filter the results:" -ForegroundColor Cyan
    Write-Host "  - .exe files: `$missingFiles | Where-Object { `$_.Extension -eq '.exe' }" -ForegroundColor Gray
    Write-Host "  - .dll files: `$missingFiles | Where-Object { `$_.Extension -eq '.dll' }" -ForegroundColor Gray
    Write-Host "  - .bit files: `$missingFiles | Where-Object { `$_.Extension -eq '.bit' }" -ForegroundColor Gray
}
