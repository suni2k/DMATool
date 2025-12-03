# Kill all OpenOCD processes
Write-Host "=== Killing all OpenOCD processes ===" -ForegroundColor Cyan
Write-Host ""

$processes = Get-Process -Name "openocd" -ErrorAction SilentlyContinue

if ($processes) {
    $count = ($processes | Measure-Object).Count
    Write-Host "Found $count OpenOCD process(es) running" -ForegroundColor Yellow
    Write-Host ""
    
    foreach ($proc in $processes) {
        try {
            $processId = $proc.Id
            $startTime = $proc.StartTime
            $runTime = (Get-Date) - $startTime
            
            Write-Host "[KILL] PID: $processId | Running for: $($runTime.ToString('hh\:mm\:ss'))" -ForegroundColor Red
            Stop-Process -Id $processId -Force
            Write-Host "       Killed successfully" -ForegroundColor Green
        }
        catch {
            Write-Host "       Failed to kill: $_" -ForegroundColor Red
        }
    }
    
    Write-Host ""
    Write-Host "=== All OpenOCD processes terminated ===" -ForegroundColor Green
}
else {
    Write-Host "[OK] No OpenOCD processes running" -ForegroundColor Green
}

Write-Host ""
Write-Host "Waiting 2 seconds for cleanup..."
Start-Sleep -Seconds 2

# Verify they're gone
$remaining = Get-Process -Name "openocd" -ErrorAction SilentlyContinue
if ($remaining) {
    Write-Host "[WARNING] Some OpenOCD processes are still running:" -ForegroundColor Yellow
    $remaining | Format-Table Id, StartTime, CPU -AutoSize
    
    Write-Host ""
    Write-Host "Attempting forceful termination with taskkill..." -ForegroundColor Yellow
    taskkill /F /IM openocd.exe /T 2>&1 | Out-Null
    
    Start-Sleep -Seconds 1
    $stillRemaining = Get-Process -Name "openocd" -ErrorAction SilentlyContinue
    if ($stillRemaining) {
        Write-Host "[ERROR] Failed to kill all processes. You may need to restart your computer." -ForegroundColor Red
    }
    else {
        Write-Host "[SUCCESS] All processes forcefully terminated" -ForegroundColor Green
    }
}
else {
    Write-Host "[SUCCESS] All OpenOCD processes terminated" -ForegroundColor Green
}
