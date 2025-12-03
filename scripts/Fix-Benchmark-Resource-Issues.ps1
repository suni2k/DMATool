# Fix-Benchmark-Resource-Issues.ps1
# Fix benchmark tab and driver issues in both Debug and Release builds

$ErrorActionPreference = 'Stop'

Write-Host "`n??????????????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?  Fixing Benchmark & Driver Resource Issues        ?" -ForegroundColor White
Write-Host "??????????????????????????????????????????????????????`n" -ForegroundColor Cyan

Write-Host "Issues Found:" -ForegroundColor Yellow
Write-Host "  1. Debug build: Benchmark buttons greyed out" -ForegroundColor White
Write-Host "     Cause: pcileech.exe not found (looking in file system)" -ForegroundColor Gray
Write-Host "     Solution: pcileech should be embedded resource`n" -ForegroundColor Gray

Write-Host "  2. Release build: Driver operations fail" -ForegroundColor White
Write-Host "     Cause: Looking for driver files in local directory" -ForegroundColor Gray
Write-Host "     Solution: Extract from embedded resources`n" -ForegroundColor Gray

Write-Host "Diagnosis:" -ForegroundColor Yellow

# Check if pcileech.exe exists in project
$pcileechPaths = @(
    "C:\Tools\PCILeech\pcileech.exe",
    "pcileech.exe",
    "tools\pcileech\pcileech.exe",
    "dmafiles\pcileech\pcileech.exe"
)

Write-Host "`nSearching for pcileech.exe..." -ForegroundColor Cyan
$foundPCILeech = $false
foreach ($path in $pcileechPaths) {
    if (Test-Path $path) {
        Write-Host "  [FOUND] $path" -ForegroundColor Green
        $foundPCILeech = $true
    } else {
        Write-Host "  [NOT FOUND] $path" -ForegroundColor Gray
    }
}

if (-not $foundPCILeech) {
    Write-Host "`n[WARNING] pcileech.exe not found in any location!" -ForegroundColor Yellow
    Write-Host "This is why benchmark buttons are greyed out.`n" -ForegroundColor Yellow
}

# Check FT601 driver files
Write-Host "`nSearching for FT601 driver files..." -ForegroundColor Cyan
$ft601DriverPath = "dmafiles\Winusb_D3XX_Release_1.4.0.1\WU_FTD3XX_Driver"
if (Test-Path $ft601DriverPath) {
    Write-Host "  [FOUND] $ft601DriverPath" -ForegroundColor Green
    $driverFiles = Get-ChildItem $ft601DriverPath -Filter "*.inf"
    foreach ($file in $driverFiles) {
        Write-Host "    - $($file.Name)" -ForegroundColor Gray
    }
} else {
    Write-Host "  [NOT FOUND] $ft601DriverPath" -ForegroundColor Red
}

# Check resource.h for resource IDs
Write-Host "`nChecking resource.h for embedded resources..." -ForegroundColor Cyan
$resourceH = Get-Content "src\resource.h" -Raw

if ($resourceH -match "IDR_PCILEECH") {
    Write-Host "  [OK] IDR_PCILEECH found in resource.h" -ForegroundColor Green
} else {
    Write-Host "  [MISSING] IDR_PCILEECH not in resource.h" -ForegroundColor Yellow
}

if ($resourceH -match "IDR_FT601") {
    Write-Host "  [OK] IDR_FT601 resources found in resource.h" -ForegroundColor Green
} else {
    Write-Host "  [MISSING] IDR_FT601 resources not in resource.h" -ForegroundColor Yellow
}

# Check DMATool.rc for resource entries
Write-Host "`nChecking DMATool.rc for resource entries..." -ForegroundColor Cyan
if (Test-Path "DMATool.rc") {
    $rc = Get-Content "DMATool.rc" -Raw
    
    if ($rc -match "pcileech\.exe") {
        Write-Host "  [OK] pcileech.exe embedded in DMATool.rc" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] pcileech.exe not embedded in DMATool.rc" -ForegroundColor Yellow
    }
    
    if ($rc -match "FTD3XXWU\.Inf") {
        Write-Host "  [OK] FT601 driver files embedded in DMATool.rc" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] FT601 driver files not embedded in DMATool.rc" -ForegroundColor Yellow
    }
}

Write-Host "`n??????????????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?  Recommended Actions                               ?" -ForegroundColor White
Write-Host "??????????????????????????????????????????????????????`n" -ForegroundColor Cyan

Write-Host "For Debug Build (Benchmark buttons greyed out):" -ForegroundColor Yellow
Write-Host "  1. Verify pcileech.exe exists in one of these locations:" -ForegroundColor White
foreach ($path in $pcileechPaths) {
    Write-Host "     - $path" -ForegroundColor Gray
}
Write-Host "  2. OR: Embed pcileech.exe as resource (recommended)" -ForegroundColor White
Write-Host "     - Add to resource.h: #define IDR_PCILEECH_EXE" -ForegroundColor Gray
Write-Host "     - Add to DMATool.rc: IDR_PCILEECH_EXE RCDATA ""path\to\pcileech.exe""" -ForegroundColor Gray
Write-Host "     - Modify BenchmarkInterface.cpp to extract from resources`n" -ForegroundColor Gray

Write-Host "For Release Build (Driver operations fail):" -ForegroundColor Yellow
Write-Host "  1. Verify FT601 driver files are embedded:" -ForegroundColor White
Write-Host "     - Check DMATool.rc has IDR_FT601_INF entries" -ForegroundColor Gray
Write-Host "     - Rebuild solution to re-embed resources" -ForegroundColor Gray
Write-Host "  2. OR: Ensure driver files exist in:" -ForegroundColor White
Write-Host "     - $ft601DriverPath`n" -ForegroundColor Gray

Write-Host "Quick Test Commands:" -ForegroundColor Yellow
Write-Host "  # Test if pcileech is available" -ForegroundColor White
Write-Host "  Test-Path 'C:\Tools\PCILeech\pcileech.exe'`n" -ForegroundColor Gray

Write-Host "  # Check resource extraction in temp" -ForegroundColor White
Write-Host "  Get-ChildItem `$env:TEMP\DMATool* -Recurse`n" -ForegroundColor Gray

Write-Host "  # Verify FT601 driver extraction" -ForegroundColor White
Write-Host "  Get-ChildItem `$env:TEMP\DMATool_FT601_Driver`n" -ForegroundColor Gray

Write-Host "????????????????????????????????????????????????????`n" -ForegroundColor Cyan
