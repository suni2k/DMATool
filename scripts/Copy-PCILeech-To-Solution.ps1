# Copy-PCILeech-To-Solution.ps1
# Copy working PCILeech from C:\Tools to solution directory

$ErrorActionPreference = 'Stop'

Write-Host "`n=== Copying Working PCILeech to Solution Directory ===" -ForegroundColor Cyan

$source = "C:\Tools\PCILeech"
$dest = "bin\PCILeech"

if (-not (Test-Path $source)) {
    Write-Host "[ERROR] Source directory not found: $source" -ForegroundColor Red
    exit 1
}

# Create destination directory
if (Test-Path $dest) {
    Write-Host "[INFO] Removing old PCILeech directory..." -ForegroundColor Yellow
    Remove-Item $dest -Recurse -Force
}

Write-Host "[INFO] Creating directory: $dest" -ForegroundColor Yellow
New-Item -ItemType Directory -Path $dest -Force | Out-Null

# Copy all DLLs and EXE
Write-Host "`n[INFO] Copying files..." -ForegroundColor Cyan

$filesToCopy = @(
    "pcileech.exe",
    "leechcore.dll",
    "FTD3XX.dll",
    "vmm.dll",
    "dbghelp.dll",
    "vcruntime140.dll",
    "leechcore_driver.dll",
    "leechcore_device_hvsavedstate.dll",
    "leechcore_device_rawtcp.dll",
    "symsrv.dll",
    "vmmyara.dll",
    "FTD3XXWU.dll"
)

$totalSize = 0
foreach ($file in $filesToCopy) {
    $srcFile = Join-Path $source $file
    $dstFile = Join-Path $dest $file
    
    if (Test-Path $srcFile) {
        Copy-Item $srcFile $dstFile -Force
        $size = (Get-Item $dstFile).Length / 1KB
        $totalSize += $size
        Write-Host "  [OK] $file ($([Math]::Round($size, 0)) KB)" -ForegroundColor Green
    } else {
        Write-Host "  [SKIP] $file (not found)" -ForegroundColor Gray
    }
}

Write-Host "`n[SUCCESS] Copied $([Math]::Round($totalSize / 1024, 1)) MB to $dest" -ForegroundColor Green
Write-Host "[INFO] PCILeech is now in the solution directory" -ForegroundColor Cyan
Write-Host ""

# Verify
Write-Host "Verifying files..." -ForegroundColor Yellow
$fileCount = (Get-ChildItem $dest).Count
Write-Host "[OK] $fileCount files in $dest" -ForegroundColor Green
Write-Host ""
