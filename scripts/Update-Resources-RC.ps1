# Update-Resources-RC.ps1
# Add PCILeech and verify FTDI driver entries in DMATool.rc

$ErrorActionPreference = 'Stop'

Write-Host "`n??????????????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?  Updating DMATool.rc with Embedded Resources       ?" -ForegroundColor White
Write-Host "??????????????????????????????????????????????????????`n" -ForegroundColor Cyan

$rcFile = "DMATool.rc"

# Backup
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$backup = "$rcFile.backup_$timestamp"
Copy-Item $rcFile $backup -Force
Write-Host "[BACKUP] Created: $backup`n" -ForegroundColor Gray

$content = Get-Content $rcFile -Raw

# Check if PCILeech entries already exist
if ($content -match "IDR_PCILEECH_EXE") {
    Write-Host "[INFO] PCILeech entries already exist in DMATool.rc" -ForegroundColor Yellow
} else {
    Write-Host "[ADDING] PCILeech resource entries..." -ForegroundColor Cyan
    
    # Find the position after FT601 entries
    $insertPosition = $content.IndexOf("IDR_FT601_CAT")
    if ($insertPosition -ne -1) {
        # Find end of line
        $lineEnd = $content.IndexOf("`n", $insertPosition)
        
        $pcileechEntries = @"

// PCILeech Files for Benchmark Tests
IDR_PCILEECH_EXE RCDATA "dmafiles\\pcileech\\pcileech.exe"
IDR_LEECHCORE_DLL RCDATA "dmafiles\\pcileech\\leechcore.dll"
IDR_FTD3XX_DLL RCDATA "dmafiles\\pcileech\\FTD3XX.dll"
IDR_VMM_DLL RCDATA "dmafiles\\pcileech\\vmm.dll"
IDR_DBGHELP_DLL RCDATA "dmafiles\\pcileech\\dbghelp.dll"
"@
        
        $content = $content.Insert($lineEnd + 1, $pcileechEntries)
        Write-Host "  [OK] Added PCILeech resource entries" -ForegroundColor Green
    } else {
        Write-Host "  [ERROR] Could not find insertion point in DMATool.rc" -ForegroundColor Red
        exit 1
    }
}

# Verify FTDI entries exist
if ($content -match "IDR_FT601_INF") {
    Write-Host "[OK] FTDI driver entries found" -ForegroundColor Green
} else {
    Write-Host "[WARNING] FTDI driver entries missing!" -ForegroundColor Yellow
    Write-Host "  Please manually add:" -ForegroundColor Gray
    Write-Host "  IDR_FT601_INF RCDATA `"dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf`"" -ForegroundColor Gray
    Write-Host "  IDR_FT601_CAT RCDATA `"dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat`"" -ForegroundColor Gray
}

# Save
$content | Set-Content $rcFile -NoNewline

Write-Host "`n[SUCCESS] DMATool.rc updated!" -ForegroundColor Green
Write-Host "Backup saved to: $backup`n" -ForegroundColor Gray

# Verify files exist
Write-Host "Verifying embedded resource files exist..." -ForegroundColor Cyan

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
} else {
    Write-Host "`n[WARNING] Some files are missing!" -ForegroundColor Yellow
}

Write-Host ""
