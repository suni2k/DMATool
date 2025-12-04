# Test script to verify DMA device is properly released after DMATool closes

Write-Host "===============================================" -ForegroundColor Cyan
Write-Host "DMA Device Release Test" -ForegroundColor Cyan
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Test Steps:" -ForegroundColor Yellow
Write-Host "1. This script will launch DMATool" -ForegroundColor White
Write-Host "2. You should:" -ForegroundColor White
Write-Host "   - Go to Data Port tab" -ForegroundColor Gray
Write-Host "   - Run a Quick Speed Test" -ForegroundColor Gray
Write-Host "   - Let it complete or stop it" -ForegroundColor Gray
Write-Host "   - CLOSE DMATool" -ForegroundColor Gray
Write-Host "3. After closing, this script will check if device is released" -ForegroundColor White
Write-Host ""

# Find DMATool executable in current directory (Desktop)
$scriptDir = $PSScriptRoot
if ([string]::IsNullOrEmpty($scriptDir)) {
    $scriptDir = Get-Location
}

$exePath = Join-Path $scriptDir "DMATool.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "[ERROR] DMATool.exe not found in: $scriptDir" -ForegroundColor Red
    Write-Host "[INFO] Please ensure DMATool.exe is in the same directory as this script" -ForegroundColor Yellow
    Write-Host "[INFO] Expected location: $exePath" -ForegroundColor Gray
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "[INFO] Found DMATool at: $exePath" -ForegroundColor Green
Write-Host ""

# Launch DMATool
Write-Host "[INFO] Launching DMATool..." -ForegroundColor Cyan
$process = Start-Process -FilePath $exePath -PassThru

# Wait for user to close it
Write-Host "[INFO] Waiting for you to test and close DMATool..." -ForegroundColor Yellow
$process.WaitForExit()

Write-Host ""
Write-Host "[SUCCESS] DMATool closed" -ForegroundColor Green
Write-Host ""

# Small delay to ensure all cleanup completes
Start-Sleep -Seconds 2

# Check if any DMATool processes are still running
Write-Host "[CHECK] Checking for lingering DMATool processes..." -ForegroundColor Cyan
$lingering = Get-Process -Name "DMATool" -ErrorAction SilentlyContinue
if ($lingering) {
    Write-Host "[WARNING] Found lingering DMATool processes:" -ForegroundColor Yellow
    $lingering | Format-Table Id, ProcessName, CPU, WorkingSet -AutoSize
} else {
    Write-Host "[SUCCESS] No lingering DMATool processes" -ForegroundColor Green
}

Write-Host ""

# Check for any processes holding FT601/FTDI devices
Write-Host "[CHECK] Checking for processes using FTDI/FT601 device..." -ForegroundColor Cyan
Write-Host "[INFO] Attempting to query device status via devcon..." -ForegroundColor Gray

# Try to find devcon (Windows Device Console)
$devconPath = "devcon.exe"
$devconFound = $false
try {
    $null = Get-Command devcon -ErrorAction Stop
    $devconFound = $true
} catch {
    Write-Host "[INFO] devcon.exe not found in PATH (this is normal)" -ForegroundColor Gray
}

if ($devconFound) {
    Write-Host "[INFO] Querying FTDI devices..." -ForegroundColor Cyan
    & devcon status "*VID_0403*" 2>&1 | Out-String | Write-Host -ForegroundColor Gray
} else {
    Write-Host "[INFO] Skipping devcon check (not installed)" -ForegroundColor Gray
    Write-Host "[TIP] Install devcon from Windows SDK for device diagnostics" -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host "Manual Verification Test" -ForegroundColor Cyan
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "NOW TESTING: Can another DMA tool access the device?" -ForegroundColor Yellow
Write-Host ""
Write-Host "To verify the fix:" -ForegroundColor White
Write-Host "1. Try running PCILeech or another DMA tool now" -ForegroundColor Gray
Write-Host "2. If it connects successfully, the fix is working!" -ForegroundColor Green
Write-Host "3. If it says 'device in use', there's still an issue" -ForegroundColor Red
Write-Host ""

# Try to run a simple LeechCore test
Write-Host "[TEST] Attempting to initialize LeechCore device..." -ForegroundColor Cyan

$tempDir = [System.IO.Path]::GetTempPath() + "DMATool_PCILeech\"
$pcileechExe = Join-Path $tempDir "pcileech.exe"

if (Test-Path $pcileechExe) {
    Write-Host "[INFO] Found PCILeech at: $pcileechExe" -ForegroundColor Green
    Write-Host "[INFO] Running quick device test..." -ForegroundColor Cyan
    
    # Run a simple probe command
    $output = & $pcileechExe "probe" 2>&1 | Out-String
    
    if ($output -match "FPGA") {
        Write-Host "[SUCCESS] PCILeech can access the device!" -ForegroundColor Green
        Write-Host "[SUCCESS] DMA device was properly released by DMATool!" -ForegroundColor Green
        Write-Host ""
        Write-Host $output -ForegroundColor Gray
    } elseif ($output -match "Failed") {
        Write-Host "[ERROR] PCILeech failed to access device" -ForegroundColor Red
        Write-Host "[INFO] This might indicate device is still locked" -ForegroundColor Yellow
        Write-Host ""
        Write-Host $output -ForegroundColor Gray
    } else {
        Write-Host "[INFO] PCILeech output:" -ForegroundColor Cyan
        Write-Host $output -ForegroundColor Gray
    }
} else {
    Write-Host "[INFO] PCILeech not extracted yet - run DMATool once to extract it" -ForegroundColor Yellow
    Write-Host "[INFO] You can manually test with other DMA tools" -ForegroundColor White
}

Write-Host ""
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host "Test Complete" -ForegroundColor Cyan
Write-Host "===============================================" -ForegroundColor Cyan
Write-Host ""
Read-Host "Press Enter to exit"
