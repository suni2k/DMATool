# Simple Flash Test - Debug Version
# Shows real-time output without redirection

# FIX: Use UPPERCASE CH347 path - this has the working OpenOCD 0.11.0!
$toolsDir = "C:\Users\suni\source\repos\DMATool\dmafiles\CH347FPGATool"
$openocdExe = Join-Path $toolsDir "OpenOCD_CH347\bin\openocd.exe"
$openocdBin = Join-Path $toolsDir "OpenOCD_CH347\bin"
$openocdScripts = Join-Path $toolsDir "OpenOCD_CH347\share\openocd\scripts"
$binFile = Join-Path $toolsDir "002ced811686a854_ACE_75T.bin"
$bscanFile = Join-Path $openocdScripts "cpld\xilinx\bscan_spi_xc7a75t.bit"

$env:OPENOCD_SCRIPTS = $openocdScripts

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Using CORRECT OpenOCD (0.11.0)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Path: $toolsDir" -ForegroundColor Yellow
Write-Host ""

# Verify we're using the correct version
Write-Host "Checking OpenOCD version..." -ForegroundColor Yellow
& $openocdExe --version | Select-Object -First 1
Write-Host ""

Write-Host "Checking files..." -ForegroundColor Yellow
Write-Host "  Binary file: $binFile" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $binFile)" -ForegroundColor $(if (Test-Path $binFile) { "Green" } else { "Red" })
Write-Host "  BSCAN file: $bscanFile" -ForegroundColor Cyan
Write-Host "  Exists: $(Test-Path $bscanFile)" -ForegroundColor $(if (Test-Path $bscanFile) { "Green" } else { "Red" })
Write-Host ""

if (-not (Test-Path $bscanFile)) {
    Write-Host "ERROR: BSCAN file not found!" -ForegroundColor Red
    Write-Host "Please run: .\scripts\Sync-OpenOCD-Files.ps1" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $binFile)) {
    Write-Host "ERROR: Firmware file not found!" -ForegroundColor Red
    exit 1
}

# Create simple config
$configFile = Join-Path $env:TEMP "simple_flash.cfg"
$config = @"
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
transport select jtag
adapter speed 10000000
source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]
"@
$config | Out-File -FilePath $configFile -Encoding ASCII -Force

Write-Host "Config file created: $configFile" -ForegroundColor Green
Write-Host "Contents:" -ForegroundColor Cyan
Get-Content $configFile
Write-Host ""

Write-Host "Running OpenOCD 0.11.0 (WORKING VERSION)..." -ForegroundColor Green
Write-Host "Expect proper erase times (~230-260ms per sector)" -ForegroundColor Yellow
Write-Host ""

# Change to OpenOCD binary directory
Push-Location $openocdBin

# Run OpenOCD with FULL PATH to BSCAN file - use FORWARD SLASHES for paths
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
} else {
    Write-Host ""
    Write-Host "Flash programming failed. Check output above for errors." -ForegroundColor Red
}

# Cleanup
Remove-Item $configFile -Force
