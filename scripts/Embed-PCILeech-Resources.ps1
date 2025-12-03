# Embed-PCILeech-Resources.ps1
# Add all PCILeech files from tools\PCILeech to DMATool.rc

$ErrorActionPreference = 'Stop'

Write-Host "`n=== Embedding PCILeech Resources ===" -ForegroundColor Cyan

$rcFile = "DMATool.rc"
$resourceHeader = "src\resource.h"
$pcileechDir = "tools\PCILeech"

if (-not (Test-Path $pcileechDir)) {
    Write-Host "[ERROR] PCILeech directory not found: $pcileechDir" -ForegroundColor Red
    Write-Host "[INFO] Run: .\scripts\Copy-PCILeech-To-Solution.ps1 first" -ForegroundColor Yellow
    exit 1
}

# Define all files to embed
$filesToEmbed = @(
    @{File="pcileech.exe"; ID=120; Name="IDR_PCILEECH_EXE"},
    @{File="leechcore.dll"; ID=200; Name="IDR_LEECHCORE_DLL"},
    @{File="FTD3XX.dll"; ID=201; Name="IDR_FTD3XX_DLL"},
    @{File="vmm.dll"; ID=121; Name="IDR_VMM_DLL"},
    @{File="dbghelp.dll"; ID=122; Name="IDR_DBGHELP_DLL"},
    @{File="vcruntime140.dll"; ID=123; Name="IDR_VCRUNTIME140_DLL"},
    @{File="leechcore_driver.dll"; ID=205; Name="IDR_LEECHCORE_DRIVER"},
    @{File="leechcore_device_hvsavedstate.dll"; ID=203; Name="IDR_LEECHCORE_DEVICE_HVSAVED"},
    @{File="leechcore_device_rawtcp.dll"; ID=204; Name="IDR_LEECHCORE_DEVICE_RAWTCP"},
    @{File="symsrv.dll"; ID=124; Name="IDR_SYMSRV_DLL"},
    @{File="vmmyara.dll"; ID=125; Name="IDR_VMMYARA_DLL"},
    @{File="FTD3XXWU.dll"; ID=202; Name="IDR_FTD3XXWU_DLL"}
)

# Backup files
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
Copy-Item $rcFile "$rcFile.backup_$timestamp" -Force
Copy-Item $resourceHeader "$resourceHeader.backup_$timestamp" -Force

Write-Host "[BACKUP] Created backups" -ForegroundColor Gray

# Read current content
$rcContent = Get-Content $rcFile -Raw
$headerContent = Get-Content $resourceHeader -Raw

# Remove old PCILeech entries
Write-Host "`n[INFO] Removing old PCILeech entries..." -ForegroundColor Yellow
$rcContent = $rcContent -replace '(?m)^// PCILeech.*?(?=\r?\n(?://|/\*|[A-Z]|\r?\n))', ''
$rcContent = $rcContent -replace '(?m)^IDR_PCILEECH.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_LEECHCORE.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_VMM.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_DBGHELP.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_VCRUNTIME.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_SYMSRV.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_VMMYARA.*?\r?\n', ''
$rcContent = $rcContent -replace '(?m)^IDR_FTD3XXWU.*?\r?\n', ''

# Build new resource entries
$newRcEntries = "`n// PCILeech Files for Benchmark Tests (embedded from tools\PCILeech)`n"
$totalSize = 0

foreach ($item in $filesToEmbed) {
    $filePath = Join-Path $pcileechDir $item.File
    
    if (Test-Path $filePath) {
        $size = (Get-Item $filePath).Length / 1KB
        $totalSize += $size
        
        # Add to RC file
        $rcPath = "tools\\PCILeech\\$($item.File)"
        $newRcEntries += "$($item.Name) RCDATA `"$rcPath`"`n"
        
        Write-Host "  [OK] $($item.File) (ID: $($item.ID), $([Math]::Round($size, 0)) KB)" -ForegroundColor Green
    } else {
        Write-Host "  [SKIP] $($item.File) (not found)" -ForegroundColor Gray
    }
}

# Insert into RC file after FT601 entries
$insertPoint = $rcContent.IndexOf("IDR_FT601_CAT")
if ($insertPoint -ne -1) {
    $lineEnd = $rcContent.IndexOf("`n", $insertPoint)
    $rcContent = $rcContent.Insert($lineEnd + 1, $newRcEntries)
} else {
    # Fallback: append at end
    $rcContent += "`n$newRcEntries"
}

# Save RC file
$rcContent | Set-Content $rcFile -NoNewline

Write-Host "`n[SUCCESS] Embedded $([Math]::Round($totalSize / 1024, 1)) MB into DMATool.rc" -ForegroundColor Green
Write-Host "[INFO] Total files: $($filesToEmbed.Count)" -ForegroundColor Cyan
Write-Host ""

# Verify resource.h has all IDs
Write-Host "Verifying resource.h..." -ForegroundColor Yellow
$missingIds = @()
foreach ($item in $filesToEmbed) {
    if ($headerContent -notmatch $item.Name) {
        $missingIds += $item
    }
}

if ($missingIds.Count -eq 0) {
    Write-Host "[OK] All resource IDs present in resource.h" -ForegroundColor Green
} else {
    Write-Host "[WARNING] Missing resource IDs in resource.h:" -ForegroundColor Yellow
    foreach ($item in $missingIds) {
        Write-Host "  - $($item.Name) (ID: $($item.ID))" -ForegroundColor Gray
    }
    Write-Host "`nManually add these to src\resource.h" -ForegroundColor Yellow
}

Write-Host ""
