# Force kill all OpenOCD processes with admin rights
# Run this as Administrator

Write-Host "=== Force Killing OpenOCD Processes (Admin) ===" -ForegroundColor Cyan
Write-Host ""

# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "[ERROR] This script must be run as Administrator!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Right-click PowerShell and select 'Run as Administrator', then run this script again." -ForegroundColor Yellow
    exit 1
}

# Use taskkill with force
Write-Host "Terminating all openocd.exe processes..." -ForegroundColor Yellow
$result = taskkill /F /IM openocd.exe /T 2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Host "[SUCCESS] All OpenOCD processes terminated" -ForegroundColor Green
}
elseif ($result -like "*not found*") {
    Write-Host "[OK] No OpenOCD processes running" -ForegroundColor Green
}
else {
    Write-Host "[ERROR] Failed to terminate some processes:" -ForegroundColor Red
    Write-Host $result
}

Write-Host ""
Write-Host "Verifying..."
Start-Sleep -Seconds 1

$remaining = Get-Process -Name "openocd" -ErrorAction SilentlyContinue
if ($remaining) {
    $count = ($remaining | Measure-Object).Count
    Write-Host "[WARNING] $count OpenOCD process(es) still running:" -ForegroundColor Yellow
    $remaining | Format-Table Id, StartTime, CPU -AutoSize
}
else {
    Write-Host "[SUCCESS] All OpenOCD processes have been terminated" -ForegroundColor Green
}
