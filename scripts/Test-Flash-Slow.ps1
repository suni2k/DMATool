# Flash Test - Slower Clock Speed
# Attempts to fix OpenOCD crash by reducing JTAG clock speed

$toolsDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
$openocdExe = Join-Path $toolsDir "OpenOCD_CH347\bin\openocd.exe"
$openocdBin = Join-Path $toolsDir "OpenOCD_CH347\bin"
$openocdScripts = Join-Path $toolsDir "OpenOCD_CH347\share\openocd\scripts"
$binFile = Join-Path $toolsDir "002ced811686a854_ACE_75T.bin"
$bscanFile = Join-Path $openocdScripts "cpld\xilinx\bscan_spi_xc7a75t.bit"

$env:OPENOCD_SCRIPTS = $openocdScripts

Write-Host "Testing with REDUCED clock speed (5 MHz instead of 10 MHz)" -ForegroundColor Yellow
Write-Host "This may fix the OpenOCD crash issue." -ForegroundColor Yellow
Write-Host ""

Write-Host "Checking files..." -ForegroundColor Yellow
Write-Host "  Binary file: $binFile" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $binFile)" -ForegroundColor $(if (Test-Path $binFile) { "Green" } else { "Red" })
Write-Host "  BSCAN file: $bscanFile" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $bscanFile)" -ForegroundColor $(if (Test-Path $bscanFile) { "Green" } else { "Red" })
Write-Host ""

if (-not (Test-Path $bscanFile)) {
    Write-Host "ERROR: BSCAN file not found!" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $binFile)) {
    Write-Host "ERROR: Firmware file not found!" -ForegroundColor Red
    exit 1
}

# Create config with SLOWER clock (5 MHz instead of 10 MHz)
$configFile = Join-Path $env:TEMP "slow_flash.cfg"
$config = @"
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
transport select jtag
adapter speed 5000000
source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]
"@
$config | Out-File -FilePath $configFile -Encoding ASCII -Force

Write-Host "Config file created: $configFile" -ForegroundColor Green
Write-Host "Contents:" -ForegroundColor Cyan
Get-Content $configFile
Write-Host ""

Write-Host "Running OpenOCD with 5 MHz clock (slower, more stable)..." -ForegroundColor Yellow
Write-Host ""

# Change to OpenOCD binary directory
Push-Location $openocdBin

# Convert paths to Unix-style
$bscanFileUnix = $bscanFile -replace '\\', '/'
$binFileUnix = $binFile -replace '\\', '/'

Write-Host "Using paths:" -ForegroundColor Cyan
Write-Host "  BSCAN (Unix): $bscanFileUnix" -ForegroundColor Gray
Write-Host "  Binary (Unix): $binFileUnix" -ForegroundColor Gray
Write-Host ""

& $openocdExe `
    -f $configFile `
    -c "init" `
    -c "jtagspi_init 0 \`"$bscanFileUnix\`"" `
    -c "jtagspi_program \`"$binFileUnix\`" 0x0" `
    -c "xc7_program xc7.tap" `
    -c "shutdown"

$exitCode = $LASTEXITCODE

Pop-Location

Write-Host ""
Write-Host "Exit code: $exitCode" -ForegroundColor $(if ($exitCode -eq 0) { "Green" } else { "Red" })

if ($exitCode -eq 0) {
    Write-Host ""
    Write-Host "SUCCESS! Flash programming completed!" -ForegroundColor Green
    Write-Host "The FPGA firmware has been written to SPI flash." -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. Test with DMATool DNA ID tab" -ForegroundColor White
    Write-Host "  2. Verify FPGA detection works" -ForegroundColor White
} elseif ($exitCode -eq -1073741819) {
    Write-Host ""
    Write-Host "CRASH AGAIN! OpenOCD is crashing even at 5 MHz." -ForegroundColor Red
    Write-Host ""
    Write-Host "This suggests a deeper issue with:" -ForegroundColor Yellow
    Write-Host "  - OpenOCD binary compatibility" -ForegroundColor White
    Write-Host "  - DLL version conflicts (libusb)" -ForegroundColor White
    Write-Host "  - Memory corruption in OpenOCD" -ForegroundColor White
    Write-Host "  - Hardware incompatibility" -ForegroundColor White
    Write-Host ""
    Write-Host "Recommended alternatives:" -ForegroundColor Yellow
    Write-Host "  1. Use CH347FpgaDownloadTool.exe (GUI tool) for now" -ForegroundColor White
    Write-Host "  2. Try different OpenOCD version" -ForegroundColor White
    Write-Host "  3. Report issue to OpenOCD/CH347 developers" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "Flash programming failed with unexpected error." -ForegroundColor Red
    Write-Host "Check output above for details." -ForegroundColor Yellow
}

# Cleanup
Remove-Item $configFile -Force
