# Copy-PCILeech-To-DMATool.ps1
# Copy PCILeech files from C:\Tools to dmafiles directory for embedding

$ErrorActionPreference = 'Stop'

Write-Host "`nCopying PCILeech files to dmafiles directory..." -ForegroundColor Cyan

# Create destination directory
$destDir = "dmafiles\pcileech"
if (-not (Test-Path $destDir)) {
    New-Item -ItemType Directory -Path $destDir -Force | Out-Null
    Write-Host "[CREATED] $destDir" -ForegroundColor Green
}

# Essential files for benchmark testing
$essentialFiles = @(
    "pcileech.exe",                      # Main executable
    "leechcore.dll",                     # Core library
    "FTD3XX.dll",                        # FT601 support
    "vmm.dll",                           # Memory management
    "dbghelp.dll",                       # Debug symbols
    "vcruntime140.dll",                  # CRITICAL: Visual C++ Runtime
    "leechcore_driver.dll",              # CRITICAL: FPGA device driver
    "leechcore_device_hvsavedstate.dll", # Optional: HV saved state support
    "leechcore_device_rawtcp.dll"        # Optional: Raw TCP support
)

$sourceDir = "C:\Tools\PCILeech"

if (-not (Test-Path $sourceDir)) {
    Write-Host "[ERROR] Source directory not found: $sourceDir" -ForegroundColor Red
    Write-Host "Please ensure PCILeech is installed to C:\Tools\PCILeech\" -ForegroundColor Yellow
    exit 1
}

Write-Host "`nCopying essential files..." -ForegroundColor Yellow
foreach ($file in $essentialFiles) {
    $sourcePath = Join-Path $sourceDir $file
    $destPath = Join-Path $destDir $file
    
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath $destPath -Force
        $size = (Get-Item $destPath).Length / 1KB
        Write-Host "  [OK] $file ($([Math]::Round($size, 0)) KB)" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] $file (optional)" -ForegroundColor Yellow
    }
}

Write-Host "`n[SUCCESS] PCILeech files copied to $destDir" -ForegroundColor Green
Write-Host "Files are ready to be embedded as resources`n" -ForegroundColor Cyan
