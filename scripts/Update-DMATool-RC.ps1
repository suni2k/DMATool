# Update-DMATool-RC.ps1
# Adds missing LeechCore DLL resources to DMATool.rc

$rcFile = "DMATool.rc"

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host " Updating DMATool.rc" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan

# Read current content
$content = Get-Content $rcFile -Raw

# Check if LeechCore resources already exist
if ($content -match "IDR_LEECHCORE_DLL") {
    Write-Host "[INFO] LeechCore resources already exist in DMATool.rc" -ForegroundColor Yellow
    Write-Host "[INFO] No changes needed" -ForegroundColor Green
    exit 0
}

Write-Host "[INFO] Adding LeechCore DLL resources..." -ForegroundColor Yellow

# Find the insertion point (after FT601 driver files, before BSCAN section)
$insertionPoint = $content.IndexOf("// BSCAN Bitstreams")

if ($insertionPoint -eq -1) {
    # Fallback: insert at end of file
    $insertionPoint = $content.Length
    Write-Host "[WARNING] Could not find BSCAN section, appending to end" -ForegroundColor Yellow
}

# LeechCore resources to add
$leechcoreSection = @"

// LeechCore DLLs for Benchmark Tab
IDR_LEECHCORE_DLL RCDATA "vendor\\leechcore\\leechcore.dll"
IDR_FTD3XX_DLL RCDATA "vendor\\leechcore\\FTD3XX.dll"
IDR_FTD3XXWU_DLL RCDATA "vendor\\leechcore\\FTD3XXWU.dll"
IDR_LEECHCORE_DEVICE_HVSAVED RCDATA "vendor\\leechcore\\leechcore_device_hvsavedstate.dll"
IDR_LEECHCORE_DEVICE_RAWTCP RCDATA "vendor\\leechcore\\leechcore_device_rawtcp.dll"
IDR_LEECHCORE_DRIVER RCDATA "vendor\\leechcore\\leechcore_driver.dll"

"@

# Insert the section
$newContent = $content.Insert($insertionPoint, $leechcoreSection)

# Write back to file
$newContent | Set-Content $rcFile -NoNewline

Write-Host "[SUCCESS] Added 6 LeechCore DLL resources to DMATool.rc" -ForegroundColor Green
Write-Host ""
Write-Host "Resources added:" -ForegroundColor Cyan
Write-Host "  - IDR_LEECHCORE_DLL" -ForegroundColor Gray
Write-Host "  - IDR_FTD3XX_DLL" -ForegroundColor Gray
Write-Host "  - IDR_FTD3XXWU_DLL" -ForegroundColor Gray
Write-Host "  - IDR_LEECHCORE_DEVICE_HVSAVED" -ForegroundColor Gray
Write-Host "  - IDR_LEECHCORE_DEVICE_RAWTCP" -ForegroundColor Gray
Write-Host "  - IDR_LEECHCORE_DRIVER" -ForegroundColor Gray
Write-Host ""

Write-Host "================================" -ForegroundColor Cyan
Write-Host " Next: Rebuild DMATool" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan
