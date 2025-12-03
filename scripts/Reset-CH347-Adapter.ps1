# CH347 Adapter Recovery Script
# Fixes CH347 communication errors by resetting the device

Write-Host "=== CH347 Adapter Recovery ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "[INFO] This script will reset the CH347 adapter to fix communication errors" -ForegroundColor Yellow
Write-Host "[INFO] Errors like 'CH347_Read read data failure' indicate the adapter needs to be reset" -ForegroundColor Yellow
Write-Host ""

# Step 1: Check current status
Write-Host "=== Step 1: Current CH347 Status ===" -ForegroundColor Cyan
$device = Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'} | Select-Object -First 1

if ($device) {
    Write-Host "[OK] Found CH347 device:" -ForegroundColor Green
    Write-Host "  Name: $($device.FriendlyName)" -ForegroundColor White
    Write-Host "  Status: $($device.Status)" -ForegroundColor White
    Write-Host "  Instance ID: $($device.InstanceId)" -ForegroundColor Gray
}
else {
    Write-Host "[ERROR] No CH347 device found!" -ForegroundColor Red
    Write-Host "[INFO] Please plug in the CH347 adapter" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "=== Step 2: Kill OpenOCD Processes ===" -ForegroundColor Cyan

$openocdProcs = Get-Process -Name "openocd" -ErrorAction SilentlyContinue
if ($openocdProcs) {
    $count = ($openocdProcs | Measure-Object).Count
    Write-Host "[INFO] Found $count OpenOCD process(es), terminating..." -ForegroundColor Yellow
    taskkill /F /IM openocd.exe /T 2>&1 | Out-Null
    Start-Sleep -Seconds 1
    Write-Host "[OK] OpenOCD processes terminated" -ForegroundColor Green
}
else {
    Write-Host "[OK] No OpenOCD processes running" -ForegroundColor Green
}

Write-Host ""
Write-Host "=== Step 3: Reset CH347 Device ===" -ForegroundColor Cyan
Write-Host "[INFO] This will disable and re-enable the CH347 adapter" -ForegroundColor Yellow
Write-Host ""

# Need admin rights for device disable/enable
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "[WARNING] This requires Administrator privileges" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please run this script as Administrator to reset the device" -ForegroundColor Cyan
    Write-Host "OR manually reset:" -ForegroundColor Cyan
    Write-Host "  1. Unplug the CH347 USB cable" -ForegroundColor White
    Write-Host "  2. Wait 5 seconds" -ForegroundColor White
    Write-Host "  3. Plug it back in" -ForegroundColor White
    Write-Host ""
    
    $response = Read-Host "Do you want to manually unplug/replug now? (y/n)"
    if ($response -eq 'y') {
        Write-Host ""
        Write-Host "[ACTION REQUIRED] Unplug the CH347 USB cable NOW" -ForegroundColor Yellow
        Write-Host "Press any key after unplugging..."
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        
        Write-Host ""
        Write-Host "Waiting 5 seconds..." -ForegroundColor Gray
        Start-Sleep -Seconds 5
        
        Write-Host ""
        Write-Host "[ACTION REQUIRED] Plug the CH347 USB cable back in NOW" -ForegroundColor Yellow
        Write-Host "Press any key after plugging in..."
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        
        Write-Host ""
        Write-Host "Waiting for device to initialize..." -ForegroundColor Gray
        Start-Sleep -Seconds 3
    }
}
else {
    Write-Host "[INFO] Running with admin privileges, attempting automatic reset..." -ForegroundColor Green
    
    # Disable device
    Write-Host "[STEP] Disabling CH347..." -ForegroundColor Yellow
    $disableResult = pnputil /disable-device "$($device.InstanceId)" 2>&1
    Start-Sleep -Seconds 2
    
    # Enable device
    Write-Host "[STEP] Enabling CH347..." -ForegroundColor Yellow
    $enableResult = pnputil /enable-device "$($device.InstanceId)" 2>&1
    Start-Sleep -Seconds 3
    
    Write-Host "[OK] Device reset complete" -ForegroundColor Green
}

Write-Host ""
Write-Host "=== Step 4: Verify Device Status ===" -ForegroundColor Cyan

$deviceAfter = Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'} | Select-Object -First 1

if ($deviceAfter) {
    Write-Host "[OK] CH347 device detected:" -ForegroundColor Green
    Write-Host "  Name: $($deviceAfter.FriendlyName)" -ForegroundColor White
    Write-Host "  Status: $($deviceAfter.Status)" -ForegroundColor White
    
    if ($deviceAfter.Status -eq 'OK') {
        Write-Host ""
        Write-Host "[SUCCESS] CH347 adapter is ready!" -ForegroundColor Green
        Write-Host ""
        Write-Host "You can now try:" -ForegroundColor Cyan
        Write-Host "  1. Run DMATool.exe and click 'Detect FPGA & Read DNA'" -ForegroundColor White
        Write-Host "  2. Or use the official CH347FPGATool" -ForegroundColor White
    }
    else {
        Write-Host ""
        Write-Host "[WARNING] Device status is: $($deviceAfter.Status)" -ForegroundColor Yellow
        Write-Host "[INFO] You may need to:" -ForegroundColor Cyan
        Write-Host "  - Manually unplug/replug the USB cable" -ForegroundColor White
        Write-Host "  - Check Device Manager for driver issues" -ForegroundColor White
        Write-Host "  - Restart your computer" -ForegroundColor White
    }
}
else {
    Write-Host "[ERROR] CH347 device not found after reset" -ForegroundColor Red
    Write-Host "[INFO] Try manually unplugging and replugging the USB cable" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== Additional Checks ===" -ForegroundColor Cyan

# Check JTAG connection
Write-Host ""
Write-Host "[TIP] Hardware Checklist:" -ForegroundColor Yellow
Write-Host "  ? CH347 USB cable plugged in" -ForegroundColor White
Write-Host "  ? JTAG cable connected from CH347 to FPGA" -ForegroundColor White
Write-Host "  ? FPGA board is powered on" -ForegroundColor White
Write-Host "  ? JTAG pins correct (TDI, TDO, TCK, TMS, GND)" -ForegroundColor White
Write-Host ""

Write-Host "[INFO] If problems persist:" -ForegroundColor Cyan
Write-Host "  1. Check connections with a multimeter (continuity test)" -ForegroundColor White
Write-Host "  2. Try a different USB port (USB 2.0 ports sometimes work better)" -ForegroundColor White
Write-Host "  3. Restart your computer" -ForegroundColor White
Write-Host "  4. Check if another application is using the CH347" -ForegroundColor White
Write-Host ""
