# PCILeech Setup Script
# Downloads and sets up PCILeech for DMA testing

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "   PCILeech Setup for DMA Testing" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Configuration
$installPath = "C:\Tools\PCILeech"
$tempPath = "$env:TEMP\pcileech_download"

# Create directories
Write-Host "[1/5] Creating directories..." -ForegroundColor Yellow
New-Item -Path $installPath -ItemType Directory -Force | Out-Null
New-Item -Path $tempPath -ItemType Directory -Force | Out-Null
Write-Host "      Created: $installPath" -ForegroundColor Green

# Get latest release info from GitHub API
Write-Host "[2/5] Fetching latest PCILeech release..." -ForegroundColor Yellow
try {
    $apiUrl = "https://api.github.com/repos/ufrisk/pcileech/releases/latest"
    $release = Invoke-RestMethod -Uri $apiUrl
    $version = $release.tag_name
    Write-Host "      Latest version: $version" -ForegroundColor Green
    
    # Find the binaries asset
    $asset = $release.assets | Where-Object { $_.name -like "*files_and_binaries*.zip" } | Select-Object -First 1
    
    if ($asset) {
        $downloadUrl = $asset.browser_download_url
        $fileName = $asset.name
        Write-Host "      Found: $fileName" -ForegroundColor Green
    } else {
        Write-Host "      ERROR: Could not find binaries in latest release!" -ForegroundColor Red
        Write-Host "      Please download manually from:" -ForegroundColor Yellow
        Write-Host "      https://github.com/ufrisk/pcileech/releases/latest" -ForegroundColor Cyan
        exit 1
    }
} catch {
    Write-Host "      ERROR: Failed to fetch release info" -ForegroundColor Red
    Write-Host "      $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
    Write-Host "      Please download manually from:" -ForegroundColor Yellow
    Write-Host "      https://github.com/ufrisk/pcileech/releases/latest" -ForegroundColor Cyan
    exit 1
}

# Download
Write-Host "[3/5] Downloading PCILeech..." -ForegroundColor Yellow
$zipPath = "$tempPath\$fileName"
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath -UseBasicParsing
    $fileSize = [math]::Round((Get-Item $zipPath).Length / 1MB, 2)
    Write-Host "      Downloaded: $fileSize MB" -ForegroundColor Green
} catch {
    Write-Host "      ERROR: Download failed!" -ForegroundColor Red
    Write-Host "      $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Extract
Write-Host "[4/5] Extracting files..." -ForegroundColor Yellow
try {
    Expand-Archive -Path $zipPath -DestinationPath $installPath -Force
    Write-Host "      Extracted to: $installPath" -ForegroundColor Green
} catch {
    Write-Host "      ERROR: Extraction failed!" -ForegroundColor Red
    Write-Host "      $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Cleanup
Write-Host "[5/5] Cleaning up..." -ForegroundColor Yellow
Remove-Item -Path $tempPath -Recurse -Force
Write-Host "      Cleanup complete" -ForegroundColor Green

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "   Installation Complete!" -ForegroundColor Green
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "PCILeech installed to: $installPath" -ForegroundColor White
Write-Host ""
Write-Host "Quick Test Commands:" -ForegroundColor Yellow
Write-Host "  cd $installPath" -ForegroundColor Cyan
Write-Host "  .\pcileech.exe probe" -ForegroundColor Cyan
Write-Host "  .\pcileech.exe benchmark" -ForegroundColor Cyan
Write-Host "  .\pcileech.exe display -min 0x1000" -ForegroundColor Cyan
Write-Host ""
Write-Host "Documentation:" -ForegroundColor Yellow
Write-Host "  See: docs\DMA_TESTING_GUIDE.md" -ForegroundColor Cyan
Write-Host ""
Write-Host "Need help? Join Discord: https://discord.gg/MfH9UHxkdP" -ForegroundColor Magenta
Write-Host ""
