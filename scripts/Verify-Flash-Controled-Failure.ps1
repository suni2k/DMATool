# Verify Flash Contents
# Reads back the flash and compares with the original BIN file

$toolsDir = "C:\Users\suni\source\repos\DMATool\dmafiles\CH347FPGATool"
$openocdExe = Join-Path $toolsDir "OpenOCD_CH347\bin\openocd.exe"
$openocdBin = Join-Path $toolsDir "OpenOCD_CH347\bin"
$openocdScripts = Join-Path $toolsDir "OpenOCD_CH347\share\openocd\scripts"
$binFile = Join-Path $toolsDir "003ccd8c77d04854_BEEAC_100T.bin"
$bscanFile = Join-Path $openocdScripts "cpld\xilinx\bscan_spi_xc7a75t.bit"
$readbackFile = Join-Path $env:TEMP "flash_readback.bin"

$env:OPENOCD_SCRIPTS = $openocdScripts

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Flash Verification Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "This will read back the flash contents and compare with the original BIN file" -ForegroundColor Yellow
Write-Host ""

# Create config
$configFile = Join-Path $env:TEMP "verify_flash.cfg"
$config = @"
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
transport select jtag
adapter speed 10000000
source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]
"@
$config | Out-File -FilePath $configFile -Encoding ASCII -Force

# Get file size
$binSize = (Get-Item $binFile).Length

Write-Host "Reading back flash contents..." -ForegroundColor Yellow
Write-Host "  Original file: $binFile" -ForegroundColor Cyan
Write-Host "  Size: $([math]::Round($binSize / 1MB, 2)) MB" -ForegroundColor Cyan
Write-Host "  Reading to: $readbackFile" -ForegroundColor Cyan
Write-Host ""

Push-Location $openocdBin

# Convert paths
$bscanFileUnix = $bscanFile -replace '\\', '/'
$readbackFileUnix = $readbackFile -replace '\\', '/'

# Read flash
& $openocdExe `
    -f $configFile `
    -c "init" `
    -c "jtagspi_init 0 \`"$bscanFileUnix\`"" `
    -c "flash read_bank 0 \`"$readbackFileUnix\`" 0x0 $binSize" `
    -c "shutdown"

$exitCode = $LASTEXITCODE
Pop-Location

Write-Host ""

if ($exitCode -eq 0) {
    Write-Host "Flash readback completed!" -ForegroundColor Green
    Write-Host ""
    
    # Compare files
    Write-Host "Comparing files..." -ForegroundColor Yellow
    
    if (-not (Test-Path $readbackFile)) {
        Write-Host "ERROR: Readback file not created!" -ForegroundColor Red
        exit 1
    }
    
    $originalHash = (Get-FileHash $binFile -Algorithm SHA256).Hash
    $readbackHash = (Get-FileHash $readbackFile -Algorithm SHA256).Hash
    
    Write-Host ""
    Write-Host "Original SHA256:  $originalHash" -ForegroundColor Cyan
    Write-Host "Readback SHA256:  $readbackHash" -ForegroundColor Cyan
    Write-Host ""
    
    if ($originalHash -eq $readbackHash) {
        Write-Host "============================================" -ForegroundColor Green
        Write-Host " VERIFICATION PASSED!" -ForegroundColor Green
        Write-Host "============================================" -ForegroundColor Green
        Write-Host ""
        Write-Host "? Flash contents match the original BIN file exactly" -ForegroundColor Green
        Write-Host "? The firmware was written correctly" -ForegroundColor Green
        Write-Host ""
    } else {
        Write-Host "============================================" -ForegroundColor Red
        Write-Host " VERIFICATION FAILED!" -ForegroundColor Red
        Write-Host "============================================" -ForegroundColor Red
        Write-Host ""
        Write-Host "? Flash contents DO NOT match!" -ForegroundColor Red
        Write-Host "? The flash may be corrupted" -ForegroundColor Red
        Write-Host ""
        Write-Host "Troubleshooting:" -ForegroundColor Yellow
        Write-Host "  1. Try reflashing with slower clock speed" -ForegroundColor White
        Write-Host "  2. Check JTAG connections" -ForegroundColor White
        Write-Host "  3. Verify hardware is working correctly" -ForegroundColor White
    }
} else {
    Write-Host "Flash readback failed!" -ForegroundColor Red
    Write-Host "Exit code: $exitCode" -ForegroundColor Red
}

# Cleanup
Remove-Item $configFile -Force
if (Test-Path $readbackFile) {
    Write-Host ""
    Write-Host "Readback file saved at: $readbackFile" -ForegroundColor Gray
}
