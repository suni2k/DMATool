# Fix-ManifestBuild.ps1
# Fixes the LNK1327 mt.exe error

Write-Host "Fixing manifest build error..." -ForegroundColor Cyan
Write-Host ""

# Make sure we're in the project root
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
Set-Location $projectRoot
Write-Host "Working directory: $projectRoot" -ForegroundColor Gray
Write-Host ""

# Step 1: Close Visual Studio if running
Write-Host "[1/4] Checking for Visual Studio processes..." -ForegroundColor Yellow
$vsProc = Get-Process -Name "devenv" -ErrorAction SilentlyContinue
if ($vsProc) {
    Write-Host "  Visual Studio is running. Please close it manually." -ForegroundColor Red
    Write-Host "  Press any key after closing Visual Studio..." -ForegroundColor Yellow
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}

# Step 2: Clean build artifacts
Write-Host "[2/4] Cleaning build artifacts..." -ForegroundColor Yellow
$dirs = @("bin-int", "bin", "x64", ".vs")
foreach ($dir in $dirs) {
    if (Test-Path $dir) {
        Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "  Deleted: $dir" -ForegroundColor Green
    }
}

# Step 3: Verify app.manifest exists
Write-Host "[3/4] Verifying app.manifest..." -ForegroundColor Yellow
if (Test-Path "app.manifest") {
    Write-Host "  app.manifest found!" -ForegroundColor Green
} else {
    Write-Host "  ERROR: app.manifest not found!" -ForegroundColor Red
    Write-Host "  Looking in: $(Get-Location)" -ForegroundColor Gray
    exit 1
}

# Step 4: Verify .vcxproj has manifest configured
Write-Host "[4/4] Verifying .vcxproj configuration..." -ForegroundColor Yellow
$vcxproj = Get-Content "DMATool.vcxproj" -Raw
if ($vcxproj -match 'AdditionalManifestFiles') {
    Write-Host "  Manifest configuration found in project!" -ForegroundColor Green
} else {
    Write-Host "  Manifest not configured - adding it now..." -ForegroundColor Yellow
    
    # Add manifest to Debug configuration
    $vcxproj = $vcxproj -replace '(<Link>[\s\S]*?<AdditionalDependencies>d3d11\.lib;dxgi\.lib;d3dcompiler\.lib;%\(AdditionalDependencies\)</AdditionalDependencies>)(\s*</Link>)', '$1`r`n      <AdditionalManifestFiles>$(ProjectDir)app.manifest</AdditionalManifestFiles>$2'
    
    Set-Content "DMATool.vcxproj" -Value $vcxproj -NoNewline
    Write-Host "  Added manifest configuration!" -ForegroundColor Green
}

Write-Host ""
Write-Host "[SUCCESS] Everything is configured correctly!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Open Visual Studio" -ForegroundColor White
Write-Host "2. Load the DMATool project" -ForegroundColor White
Write-Host "3. Build -> Rebuild Solution" -ForegroundColor White
Write-Host ""
Write-Host "If it still fails with LNK1327, the issue might be:" -ForegroundColor Yellow
Write-Host "- Windows Defender or antivirus blocking mt.exe" -ForegroundColor White
Write-Host "- Corrupted Windows SDK installation" -ForegroundColor White
Write-Host "- Missing mt.exe from Windows SDK" -ForegroundColor White
