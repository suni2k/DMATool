# Flash Test - Using WORKING OpenOCD Version
# Uses the OpenOCD 0.11.0 binary from CH347FpgaDownloadTool which actually works!

$toolsDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"

# CRITICAL: Use the CH347FpgaDownloadTool's OpenOCD, NOT the OpenOCD_CH347 version!
# The CH347FpgaDownloadTool version is OpenOCD 0.11.0 and works correctly
# The OpenOCD_CH347 version is 0.12.0+dev and has a flash erase bug

$openocdExe = Join-Path $toolsDir "CH347FpgaDownloadTool\openocd.exe"
$openocdScripts = Join-Path $toolsDir "CH347FpgaDownloadTool\openocd-scripts"
$binFile = Join-Path $toolsDir "002ced811686a854_ACE_75T.bin"
$bscanFile = Join-Path $openocdScripts "cpld\xilinx\bscan_spi_xc7a75t.bit"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " USING WORKING OPENOCD VERSION (0.11.0)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Discovery: OpenOCD 0.12.0+dev has a flash erase bug!" -ForegroundColor Yellow
Write-Host "Solution: Using OpenOCD 0.11.0 from CH347FpgaDownloadTool" -ForegroundColor Green
Write-Host ""

# Check if CH347FpgaDownloadTool directory exists
if (-not (Test-Path (Join-Path $toolsDir "CH347FpgaDownloadTool"))) {
    Write-Host "ERROR: CH347FpgaDownloadTool directory not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Expected location:" -ForegroundColor Yellow
    Write-Host "  $toolsDir\CH347FpgaDownloadTool\" -ForegroundColor White
    Write-Host ""
    Write-Host "This directory should contain:" -ForegroundColor Yellow
    Write-Host "  - openocd.exe (version 0.11.0)" -ForegroundColor White
    Write-Host "  - openocd-scripts\ folder" -ForegroundColor White
    Write-Host "  - CH347FpgaDownloadTool.exe" -ForegroundColor White
    Write-Host ""
    Write-Host "Please copy from your fresh download!" -ForegroundColor Red
    exit 1
}

# Set OpenOCD scripts path
$env:OPENOCD_SCRIPTS = $openocdScripts

Write-Host "Checking files..." -ForegroundColor Yellow
Write-Host "  OpenOCD executable: $openocdExe" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $openocdExe)" -ForegroundColor $(if (Test-Path $openocdExe) { "Green" } else { "Red" })
Write-Host "  Binary file: $binFile" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $binFile)" -ForegroundColor $(if (Test-Path $binFile) { "Green" } else { "Red" })
Write-Host "  BSCAN file: $bscanFile" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $bscanFile)" -ForegroundColor $(if (Test-Path $bscanFile) { "Green" } else { "Red" })
Write-Host ""

if (-not (Test-Path $openocdExe)) {
    Write-Host "ERROR: OpenOCD 0.11.0 not found!" -ForegroundColor Red
    Write-Host "Please copy CH347FpgaDownloadTool folder from fresh download" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $bscanFile)) {
    Write-Host "ERROR: BSCAN file not found!" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $binFile)) {
    Write-Host "ERROR: Firmware file not found!" -ForegroundColor Red
    exit 1
}

# Create config matching the working tool's settings
$configFile = Join-Path $env:TEMP "working_flash.cfg"
$config = @"
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
transport select jtag
adapter speed 10000
source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]
"@
$config | Out-File -FilePath $configFile -Encoding ASCII -Force

Write-Host "Config file created: $configFile" -ForegroundColor Green
Write-Host "Contents:" -ForegroundColor Cyan
Get-Content $configFile
Write-Host ""

Write-Host "Running WORKING OpenOCD (0.11.0) - THIS SHOULD WORK!" -ForegroundColor Green
Write-Host "Watch for proper erase times (~230-260ms per sector)" -ForegroundColor Yellow
Write-Host ""

# Change to CH347FpgaDownloadTool directory
Push-Location (Join-Path $toolsDir "CH347FpgaDownloadTool")

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
    Write-Host "============================================" -ForegroundColor Green
    Write-Host " SUCCESS! FLASH PROGRAMMING COMPLETED!" -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "The FPGA firmware has been written to SPI flash!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Key Finding:" -ForegroundColor Yellow
    Write-Host "  - OpenOCD 0.12.0+dev has a flash erase bug" -ForegroundColor White
    Write-Host "  - OpenOCD 0.11.0 works correctly" -ForegroundColor White
    Write-Host "  - Use CH347FpgaDownloadTool version for DMATool integration" -ForegroundColor White
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. Test with DMATool DNA ID tab" -ForegroundColor White
    Write-Host "  2. Update backend to use OpenOCD 0.11.0" -ForegroundColor White
    Write-Host "  3. Implement C++ FlashInterface class" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "Flash programming failed!" -ForegroundColor Red
    Write-Host "Exit code: $exitCode" -ForegroundColor Red
    Write-Host ""
    Write-Host "This is unexpected - the official tool's OpenOCD should work!" -ForegroundColor Yellow
    Write-Host "Check output above for errors." -ForegroundColor Yellow
}

# Cleanup
Remove-Item $configFile -Force
