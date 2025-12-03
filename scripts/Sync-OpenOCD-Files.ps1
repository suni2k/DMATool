# Sync OpenOCD Files from Fresh Download
# This script copies missing files from the fresh CH347FPGATool download

param(
    [Parameter(Mandatory=$false)]
    [string]$SourceDir = "C:\Users\suni\Desktop\dma\CH347FPGATool",
    
    [Parameter(Mandatory=$false)]
    [string]$DestDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
)

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " OpenOCD File Sync Utility" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Verify source directory exists
if (-not (Test-Path $SourceDir)) {
    Write-Host "ERROR: Source directory not found: $SourceDir" -ForegroundColor Red
    Write-Host "Please update the -SourceDir parameter" -ForegroundColor Yellow
    exit 1
}

# Verify destination directory exists
if (-not (Test-Path $DestDir)) {
    Write-Host "WARNING: Destination directory not found: $DestDir" -ForegroundColor Yellow
    Write-Host "Creating destination directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $DestDir -Force | Out-Null
}

Write-Host "Source:      $SourceDir" -ForegroundColor Cyan
Write-Host "Destination: $DestDir" -ForegroundColor Cyan
Write-Host ""

# Critical files to sync
$criticalFiles = @{
    "BSCAN Bitstreams" = @(
        @{ Source = "FPGABit\bscan_spi_xc7a35t.bit"; Dest = "OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a35t.bit" },
        @{ Source = "FPGABit\bscan_spi_xc7a50t.bit"; Dest = "OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a50t.bit" },
        @{ Source = "FPGABit\bscan_spi_xc7a75t.bit"; Dest = "OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a75t.bit" },
        @{ Source = "FPGABit\bscan_spi_xc7a100t.bit"; Dest = "OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a100t.bit" }
    )
    "OpenOCD Scripts" = @(
        @{ Source = "OpenOCD_CH347\share\openocd\scripts\cpld\jtagspi.cfg"; Dest = "OpenOCD_CH347\share\openocd\scripts\cpld\jtagspi.cfg" },
        @{ Source = "OpenOCD_CH347\share\openocd\scripts\cpld\xilinx-xc7.cfg"; Dest = "OpenOCD_CH347\share\openocd\scripts\cpld\xilinx-xc7.cfg" },
        @{ Source = "OpenOCD_CH347\share\openocd\scripts\fpga\xilinx-dna.cfg"; Dest = "OpenOCD_CH347\share\openocd\scripts\fpga\xilinx-dna.cfg" }
    )
    "Firmware Files" = @(
        @{ Source = "002ced811686a854_ACE_75T.bin"; Dest = "002ced811686a854_ACE_75T.bin" },
        @{ Source = "003ccd8c77d04854_BEEAC_100T.bin"; Dest = "003ccd8c77d04854_BEEAC_100T.bin" }
    )
}

$filesCopied = 0
$filesSkipped = 0
$filesMissing = 0

foreach ($category in $criticalFiles.Keys) {
    Write-Host "[$category]" -ForegroundColor Yellow
    
    foreach ($fileEntry in $criticalFiles[$category]) {
        $sourcePath = Join-Path $SourceDir $fileEntry.Source
        $destPath = Join-Path $DestDir $fileEntry.Dest
        
        if (Test-Path $sourcePath) {
            # Create destination directory if needed
            $destDirPath = Split-Path $destPath -Parent
            if (-not (Test-Path $destDirPath)) {
                New-Item -ItemType Directory -Path $destDirPath -Force | Out-Null
            }
            
            # Check if file already exists
            if (Test-Path $destPath) {
                $sourceSize = (Get-Item $sourcePath).Length
                $destSize = (Get-Item $destPath).Length
                
                if ($sourceSize -eq $destSize) {
                    Write-Host "  ? $($fileEntry.Source) (already exists, same size)" -ForegroundColor Gray
                    $filesSkipped++
                } else {
                    Write-Host "  ? $($fileEntry.Source) (updating, size mismatch)" -ForegroundColor Yellow
                    Copy-Item $sourcePath $destPath -Force
                    $filesCopied++
                }
            } else {
                Write-Host "  + $($fileEntry.Source) (copying)" -ForegroundColor Green
                Copy-Item $sourcePath $destPath -Force
                $filesCopied++
            }
        } else {
            Write-Host "  ? $($fileEntry.Source) (NOT FOUND in source)" -ForegroundColor Red
            $filesMissing++
        }
    }
    
    Write-Host ""
}

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Files copied:  $filesCopied" -ForegroundColor Green
Write-Host "  Files skipped: $filesSkipped" -ForegroundColor Gray
Write-Host "  Files missing: $filesMissing" -ForegroundColor $(if ($filesMissing -gt 0) { "Red" } else { "Gray" })
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($filesCopied -gt 0) {
    Write-Host "? Files synced successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. Run: .\scripts\Test-OpenOCD-Simple.ps1" -ForegroundColor White
    Write-Host "  2. Run: .\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t" -ForegroundColor White
    Write-Host ""
} elseif ($filesSkipped -gt 0) {
    Write-Host "? All files already up to date!" -ForegroundColor Green
    Write-Host ""
} else {
    Write-Host "? No files were copied!" -ForegroundColor Yellow
    Write-Host ""
}

if ($filesMissing -gt 0) {
    Write-Host "? WARNING: Some files are missing from source!" -ForegroundColor Red
    Write-Host "The fresh download might be incomplete or corrupted." -ForegroundColor Yellow
    Write-Host ""
}
