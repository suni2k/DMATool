# Check for hanging OpenOCD processes during flash operations
# Run this while the flash operation is stuck

Write-Host "=== OpenOCD Process Diagnostic ===" -ForegroundColor Cyan
Write-Host ""

# Find all OpenOCD processes
$openocdProcesses = Get-Process -Name "openocd" -ErrorAction SilentlyContinue

if (!$openocdProcesses) {
    Write-Host "[INFO] No OpenOCD processes found" -ForegroundColor Yellow
    Write-Host "This means OpenOCD may have crashed or failed to start" -ForegroundColor Yellow
    exit 0
}

$count = ($openocdProcesses | Measure-Object).Count
Write-Host "[FOUND] $count OpenOCD process(es) running" -ForegroundColor Green
Write-Host ""

foreach ($proc in $openocdProcesses) {
    Write-Host "Process ID: $($proc.Id)" -ForegroundColor White
    Write-Host "  Start Time: $($proc.StartTime)"
    Write-Host "  CPU Time: $($proc.CPU) seconds"
    Write-Host "  Memory (MB): $([math]::Round($proc.WorkingSet64 / 1MB, 2))"
    Write-Host "  Responding: $($proc.Responding)"
    
    # Get command line
    try {
        $cmdLine = (Get-CimInstance Win32_Process -Filter "ProcessId = $($proc.Id)").CommandLine
        Write-Host "  Command: $cmdLine" -ForegroundColor Cyan
    } catch {
        Write-Host "  Command: Unable to retrieve" -ForegroundColor Red
    }
    
    Write-Host ""
}

Write-Host "=== Suggestions ===" -ForegroundColor Yellow
Write-Host ""
Write-Host "If CPU time is increasing:" -ForegroundColor White
Write-Host "  - OpenOCD is actively working (normal during flash)" -ForegroundColor Gray
Write-Host ""
Write-Host "If CPU time is NOT increasing:" -ForegroundColor White
Write-Host "  - OpenOCD may be stuck/deadlocked" -ForegroundColor Gray
Write-Host "  - Try killing processes with: .\Kill-OpenOCD-Admin.ps1" -ForegroundColor Gray
Write-Host "  - Check CH347 adapter connection" -ForegroundColor Gray
Write-Host ""
Write-Host "If 'Responding' is False:" -ForegroundColor White
Write-Host "  - Process is definitely hung" -ForegroundColor Gray
Write-Host "  - Kill it immediately with: .\Kill-OpenOCD-Admin.ps1" -ForegroundColor Gray
Write-Host ""
