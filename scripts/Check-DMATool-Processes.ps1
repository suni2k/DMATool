# Check-DMATool-Processes.ps1
# Check for running DMATool processes and optionally terminate them

$ErrorActionPreference = 'Stop'

Write-Host "`n=== DMATool Process Monitor ===" -ForegroundColor Cyan
Write-Host "Checking for running DMATool-related processes...`n" -ForegroundColor Yellow

# Define all DMATool-related process names
$dmatolProcesses = @(
    "DMATool",           # Main application
    "pcileech",          # PCILeech benchmark
    "leechcore",         # LeechCore (if standalone)
    "openocd"            # OpenOCD JTAG interface
)

# Track found processes
$foundProcesses = @()
$totalProcessCount = 0

# Check each process type
foreach ($processName in $dmatolProcesses) {
    $processes = Get-Process -Name $processName -ErrorAction SilentlyContinue
    
    if ($processes) {
        $count = ($processes | Measure-Object).Count
        $totalProcessCount += $count
        
        Write-Host "[$processName]" -ForegroundColor Yellow
        foreach ($proc in $processes) {
            $foundProcesses += $proc
            
            # Get process details
            $pid = $proc.Id
            $startTime = $proc.StartTime
            $cpu = [math]::Round($proc.CPU, 2)
            $memory = [math]::Round($proc.WorkingSet64 / 1MB, 2)
            $path = try { $proc.Path } catch { "N/A" }
            
            Write-Host "  PID: $pid" -ForegroundColor White
            Write-Host "  Start Time: $startTime" -ForegroundColor Gray
            Write-Host "  CPU Time: $cpu seconds" -ForegroundColor Gray
            Write-Host "  Memory: $memory MB" -ForegroundColor Gray
            Write-Host "  Path: $path" -ForegroundColor Gray
            Write-Host ""
        }
    }
}

# Summary
Write-Host "=== Summary ===" -ForegroundColor Cyan
if ($totalProcessCount -eq 0) {
    Write-Host "? No DMATool processes found - all clean!" -ForegroundColor Green
    Write-Host ""
    exit 0
}
else {
    Write-Host "? Found $totalProcessCount DMATool process(es) still running!" -ForegroundColor Yellow
    Write-Host ""
    
    # Ask user if they want to terminate
    $response = Read-Host "Do you want to terminate these processes? (Y/N)"
    
    if ($response -eq 'Y' -or $response -eq 'y') {
        Write-Host "`nTerminating processes..." -ForegroundColor Yellow
        
        foreach ($proc in $foundProcesses) {
            try {
                $procName = $proc.ProcessName
                $procId = $proc.Id
                
                Write-Host "  Stopping $procName (PID: $procId)..." -ForegroundColor Gray
                Stop-Process -Id $procId -Force -ErrorAction Stop
                Write-Host "  ? Stopped $procName" -ForegroundColor Green
            }
            catch {
                Write-Host "  ? Failed to stop $procName (PID: $procId): $($_.Exception.Message)" -ForegroundColor Red
            }
        }
        
        # Wait a moment and verify
        Start-Sleep -Seconds 1
        
        Write-Host "`nVerifying..." -ForegroundColor Yellow
        $remainingProcesses = @()
        foreach ($processName in $dmatolProcesses) {
            $remaining = Get-Process -Name $processName -ErrorAction SilentlyContinue
            if ($remaining) {
                $remainingProcesses += $remaining
            }
        }
        
        if ($remainingProcesses.Count -eq 0) {
            Write-Host "? All processes terminated successfully!" -ForegroundColor Green
        }
        else {
            Write-Host "? Warning: $($remainingProcesses.Count) process(es) still running" -ForegroundColor Yellow
            foreach ($proc in $remainingProcesses) {
                Write-Host "  - $($proc.ProcessName) (PID: $($proc.Id))" -ForegroundColor Yellow
            }
            Write-Host "`nNote: Some processes may require Administrator privileges to terminate." -ForegroundColor Gray
        }
    }
    else {
        Write-Host "Processes left running (user choice)" -ForegroundColor Gray
    }
}

Write-Host ""
