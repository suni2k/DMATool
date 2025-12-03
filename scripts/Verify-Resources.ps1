# Verify-Resources.ps1
# Check which resources are defined vs actually embedded

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host " DMATool Resource Verification" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan

$rcFile = "DMATool.rc"
$resourceHeader = "src\resource.h"

# Read resource definitions
$resourceDefs = Get-Content $resourceHeader | Where-Object { $_ -match "^#define IDR_" }

Write-Host "[1/3] Resources defined in resource.h:" -ForegroundColor Yellow
$resourceDefs | ForEach-Object {
    if ($_ -match "#define\s+(IDR_\w+)\s+(\d+)") {
        Write-Host "  $($matches[1]) = $($matches[2])" -ForegroundColor Gray
    }
}

Write-Host "`n[2/3] Resources in DMATool.rc:" -ForegroundColor Yellow
$rcContent = Get-Content $rcFile
$embeddedResources = $rcContent | Where-Object { $_ -match "^IDR_" }
$embeddedResources | ForEach-Object {
    Write-Host "  $_" -ForegroundColor Gray
}

Write-Host "`n[3/3] Missing resources:" -ForegroundColor Yellow
$missing = @()

# Check for missing LeechCore DLLs
if ($rcContent -notmatch "IDR_LEECHCORE_DLL") {
    $missing += "IDR_LEECHCORE_DLL (leechcore.dll)"
}
if ($rcContent -notmatch "IDR_FTD3XX_DLL") {
    $missing += "IDR_FTD3XX_DLL (FTD3XX.dll)"
}
if ($rcContent -notmatch "IDR_FTD3XXWU_DLL") {
    $missing += "IDR_FTD3XXWU_DLL (FTD3XXWU.dll)"
}
if ($rcContent -notmatch "IDR_LEECHCORE_DEVICE_HVSAVED") {
    $missing += "IDR_LEECHCORE_DEVICE_HVSAVED (leechcore_device_hvsavedstate.dll)"
}
if ($rcContent -notmatch "IDR_LEECHCORE_DEVICE_RAWTCP") {
    $missing += "IDR_LEECHCORE_DEVICE_RAWTCP (leechcore_device_rawtcp.dll)"
}
if ($rcContent -notmatch "IDR_LEECHCORE_DRIVER") {
    $missing += "IDR_LEECHCORE_DRIVER (leechcore_driver.dll)"
}

if ($missing.Count -eq 0) {
    Write-Host "  ? All resources are embedded!" -ForegroundColor Green
} else {
    Write-Host "  ? Missing $($missing.Count) resources:" -ForegroundColor Red
    $missing | ForEach-Object {
        Write-Host "    - $_" -ForegroundColor Red
    }
    
    Write-Host "`n[ACTION REQUIRED]" -ForegroundColor Yellow
    Write-Host "You need to manually add these lines to DMATool.rc:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "// LeechCore DLLs for Benchmark Tab" -ForegroundColor Cyan
    Write-Host "IDR_LEECHCORE_DLL RCDATA `"tools\\leechcore\\leechcore.dll`"" -ForegroundColor Cyan
    Write-Host "IDR_FTD3XX_DLL RCDATA `"tools\\leechcore\\FTD3XX.dll`"" -ForegroundColor Cyan
    Write-Host "IDR_FTD3XXWU_DLL RCDATA `"tools\\leechcore\\FTD3XXWU.dll`"" -ForegroundColor Cyan
    Write-Host "IDR_LEECHCORE_DEVICE_HVSAVED RCDATA `"tools\\leechcore\\leechcore_device_hvsavedstate.dll`"" -ForegroundColor Cyan
    Write-Host "IDR_LEECHCORE_DEVICE_RAWTCP RCDATA `"tools\\leechcore\\leechcore_device_rawtcp.dll`"" -ForegroundColor Cyan
    Write-Host "IDR_LEECHCORE_DRIVER RCDATA `"tools\\leechcore\\leechcore_driver.dll`"" -ForegroundColor Cyan
    Write-Host ""
}

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host " Verification Complete" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan
