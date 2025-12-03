# Add BSCAN Bitstreams to DMATool.rc
# This script adds the BSCAN bitstream files as embedded resources

$rcFile = "DMATool.rc"
$bscanSourcePath = ".\dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx"

# BSCAN files to embed
$bscanFiles = @{
    "bscan_spi_xc7a35t.bit" = "IDR_BSCAN_XC7A35T"
    "bscan_spi_xc7a50t.bit" = "IDR_BSCAN_XC7A50T"
    "bscan_spi_xc7a75t.bit" = "IDR_BSCAN_XC7A75T"
    "bscan_spi_xc7a100t.bit" = "IDR_BSCAN_XC7A100T"
    "bscan_spi_xc7a200t.bit" = "IDR_BSCAN_XC7A200T"
}

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host " Adding BSCAN Files to Resources" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan

# Check if files exist
Write-Host "[1/3] Checking BSCAN files..." -ForegroundColor Yellow
$allExist = $true
foreach ($file in $bscanFiles.Keys) {
    $fullPath = Join-Path $bscanSourcePath $file
    if (Test-Path $fullPath) {
        $size = (Get-Item $fullPath).Length
        Write-Host "  ? Found: $file ($size bytes)" -ForegroundColor Green
    } else {
        Write-Host "  ? Missing: $file" -ForegroundColor Red
        $allExist = $false
    }
}

if (-not $allExist) {
    Write-Host "`n[ERROR] Some BSCAN files are missing!" -ForegroundColor Red
    exit 1
}

# Read existing RC file
Write-Host "`n[2/3] Reading DMATool.rc..." -ForegroundColor Yellow
$rcContent = Get-Content $rcFile -Raw

# Check if BSCAN section already exists
if ($rcContent -match "// BSCAN Bitstreams") {
    Write-Host "  ! BSCAN section already exists in RC file" -ForegroundColor Yellow
    Write-Host "  ! Skipping to avoid duplicates" -ForegroundColor Yellow
} else {
    # Add BSCAN section before the end of file
    Write-Host "`n[3/3] Adding BSCAN resources..." -ForegroundColor Yellow
    
    $bscanSection = @"

/////////////////////////////////////////////////////////////////////////////
//
// BSCAN Bitstreams for FPGA Flash Programming
//

"@
    
    foreach ($file in $bscanFiles.Keys) {
        $resourceId = $bscanFiles[$file]
        $relativePath = Join-Path $bscanSourcePath $file
        $relativePath = $relativePath.Replace('\', '\\')
        
        $bscanSection += "$resourceId                RCDATA                  `"$relativePath`"`r`n"
        Write-Host "  ? Added: $resourceId ? $file" -ForegroundColor Green
    }
    
    # Insert before the last line (usually empty or end comment)
    $rcContent = $rcContent.TrimEnd()
    $rcContent += "`r`n$bscanSection`r`n"
    
    # Write back to file
    $rcContent | Set-Content $rcFile -NoNewline -Encoding UTF8
    
    Write-Host "`n? Successfully added BSCAN resources to DMATool.rc" -ForegroundColor Green
}

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host " Done! Rebuild DMATool to embed" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan
