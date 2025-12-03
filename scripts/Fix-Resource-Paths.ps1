# Fix-Resource-Paths.ps1
# Fix resource paths and ensure all files are in the correct locations

$ErrorActionPreference = 'Stop'

Write-Host "`n??????????????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?  Fixing Resource Paths                             ?" -ForegroundColor White
Write-Host "??????????????????????????????????????????????????????`n" -ForegroundColor Cyan

# Issue 1: FTDI driver paths in DMATool.rc
Write-Host "[1] Fixing FTDI driver paths in DMATool.rc..." -ForegroundColor Yellow

$rcFile = "DMATool.rc"
$content = Get-Content $rcFile -Raw

# Create backup
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
Copy-Item $rcFile "$rcFile.backup_$timestamp" -Force
Write-Host "  [BACKUP] Created: $rcFile.backup_$timestamp" -ForegroundColor Gray

# Fix FT601 paths
$oldPath1 = 'IDR_FT601_INF RCDATA "tools\\ftdi601\\drivers\\FTD3XXWU.Inf"'
$newPath1 = 'IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"'

$oldPath2 = 'IDR_FT601_CAT RCDATA "tools\\ftdi601\\drivers\\FTD3XXWU.cat"'
$newPath2 = 'IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"'

if ($content -match [regex]::Escape($oldPath1)) {
    $content = $content -replace [regex]::Escape($oldPath1), $newPath1
    Write-Host "  [FIXED] IDR_FT601_INF path updated" -ForegroundColor Green
}

if ($content -match [regex]::Escape($oldPath2)) {
    $content = $content -replace [regex]::Escape($oldPath2), $newPath2
    Write-Host "  [FIXED] IDR_FT601_CAT path updated" -ForegroundColor Green
}

$content | Set-Content $rcFile -NoNewline

# Verify files exist
Write-Host "`n[2] Verifying resource files exist..." -ForegroundColor Yellow

$filesToCheck = @(
    @{Path="dmafiles\pcileech\pcileech.exe"; Name="PCILeech executable"},
    @{Path="dmafiles\pcileech\leechcore.dll"; Name="LeechCore DLL"},
    @{Path="dmafiles\pcileech\FTD3XX.dll"; Name="FTD3XX DLL"},
    @{Path="dmafiles\pcileech\vmm.dll"; Name="VMM DLL"},
    @{Path="dmafiles\pcileech\dbghelp.dll"; Name="DbgHelp DLL"},
    @{Path="dmafiles\Winusb_D3XX_Release_1.4.0.1\WU_FTD3XX_Driver\FTD3XXWU.Inf"; Name="FTDI Driver INF"},
    @{Path="dmafiles\Winusb_D3XX_Release_1.4.0.1\WU_FTD3XX_Driver\FTD3XXWU.cat"; Name="FTDI Driver CAT"}
)

$allExist = $true
foreach ($file in $filesToCheck) {
    if (Test-Path $file.Path) {
        $size = (Get-Item $file.Path).Length / 1KB
        Write-Host "  [OK] $($file.Name) ($([Math]::Round($size, 0)) KB)" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] $($file.Name) - $($file.Path)" -ForegroundColor Red
        $allExist = $false
    }
}

if ($allExist) {
    Write-Host "`n[SUCCESS] All resource files verified!" -ForegroundColor Green
    Write-Host "DMATool.rc now points to correct paths.`n" -ForegroundColor Green
} else {
    Write-Host "`n[WARNING] Some files are missing!" -ForegroundColor Yellow
    Write-Host "Build may fail if these files are not found.`n" -ForegroundColor Yellow
}

Write-Host "????????????????????????????????????????????????????`n" -ForegroundColor Cyan
