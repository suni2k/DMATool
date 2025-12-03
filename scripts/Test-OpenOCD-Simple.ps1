# Test OpenOCD with CH347 - Simple Version
# This tests basic connectivity without complex flash operations

# Paths - Updated to use dmafiles directory
$toolsDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
$openocdExe = Join-Path $toolsDir "OpenOCD_CH347\bin\openocd.exe"
$openocdBin = Join-Path $toolsDir "OpenOCD_CH347\bin"
$openocdScripts = Join-Path $toolsDir "OpenOCD_CH347\share\openocd\scripts"

# Set environment variable for OpenOCD scripts
$env:OPENOCD_SCRIPTS = $openocdScripts

Write-Host "OpenOCD executable: $openocdExe" -ForegroundColor Cyan
Write-Host "Scripts directory: $openocdScripts" -ForegroundColor Cyan
Write-Host ""

# Create minimal config
$configFile = Join-Path $env:TEMP "test_openocd.cfg"
$config = @"
# Minimal CH347 test config
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
transport select jtag
adapter speed 10000
"@

$config | Out-File -FilePath $configFile -Encoding ASCII -Force

Write-Host "Config file created at: $configFile" -ForegroundColor Green
Write-Host "Config contents:" -ForegroundColor Yellow
Get-Content $configFile
Write-Host ""

# Change to OpenOCD directory
Push-Location $openocdBin

try {
    Write-Host "Running OpenOCD with minimal config..." -ForegroundColor Cyan
    Write-Host "Command: openocd.exe -f $configFile -c init -c 'scan_chain' -c shutdown" -ForegroundColor Yellow
    Write-Host ""
    
    & $openocdExe -f $configFile -c "init" -c "scan_chain" -c "shutdown"
    
    $exitCode = $LASTEXITCODE
    Write-Host ""
    Write-Host "Exit code: $exitCode" -ForegroundColor $(if ($exitCode -eq 0) { "Green" } else { "Red" })
}
finally {
    Pop-Location
    if (Test-Path $configFile) {
        Remove-Item $configFile -Force
    }
}
