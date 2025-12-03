# Configure-Manifest.ps1
# This script configures the DMATool project to embed app.manifest for administrator privileges
# SAFE VERSION - Uses text replacement instead of XML manipulation to avoid VS compatibility issues

param(
    [switch]$WhatIf
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DMATool Manifest Configuration Script" -ForegroundColor Cyan
Write-Host "Safe Version - Text-based editing" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$vcxprojPath = Join-Path $PSScriptRoot "..\DMATool.vcxproj"

if (-not (Test-Path $vcxprojPath)) {
    Write-Host "[ERROR] Cannot find DMATool.vcxproj at: $vcxprojPath" -ForegroundColor Red
    exit 1
}

Write-Host "[INFO] Found project file: $vcxprojPath" -ForegroundColor Green

# Backup the original file
$backupPath = "$vcxprojPath.backup"
if (-not $WhatIf) {
    Copy-Item $vcxprojPath $backupPath -Force
    Write-Host "[INFO] Created backup: $backupPath" -ForegroundColor Yellow
}

# Read the file as text
$content = Get-Content $vcxprojPath -Raw

# Check if manifest settings already exist
if ($content -match 'AdditionalManifestFiles') {
    Write-Host ""
    Write-Host "[INFO] Manifest settings already exist in the project file" -ForegroundColor Green
    Write-Host "[INFO] No changes needed!" -ForegroundColor Green
    exit 0
}

# Define the additions for Debug configuration
$debugAddition = @"
      <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
"@

# Define the additions for Release configuration  
$releaseAddition = @"
      <AdditionalDependencies>d3d11.lib;dxgi.lib;d3dcompiler.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>
"@

$modified = $false

# Process Debug configuration
if ($content -match '(?s)(<ItemDefinitionGroup Condition.*Debug\|x64.*?<Link>.*?)<AdditionalDependencies>d3d11\.lib;dxgi\.lib;d3dcompiler\.lib;%\(AdditionalDependencies\)</AdditionalDependencies>') {
    Write-Host "[INFO] Found Debug configuration Link section" -ForegroundColor Cyan
    
    $replacement = $debugAddition
    $content = $content -replace '(<ItemDefinitionGroup Condition.*Debug\|x64.*?<Link>.*?)<AdditionalDependencies>d3d11\.lib;dxgi\.lib;d3dcompiler\.lib;%\(AdditionalDependencies\)</AdditionalDependencies>', "`$1$replacement"
    $modified = $true
    Write-Host "  [ADD] AdditionalManifestFiles to Debug configuration" -ForegroundColor Green
}

# Process Release configuration
if ($content -match '(?s)(<ItemDefinitionGroup Condition.*Release\|x64.*?<Link>.*?)<AdditionalDependencies>d3d11\.lib;dxgi\.lib;d3dcompiler\.lib;%\(AdditionalDependencies\)</AdditionalDependencies>') {
    Write-Host "[INFO] Found Release configuration Link section" -ForegroundColor Cyan
    
    # For Release, we need to be more careful since there are more properties
    # Find the Release Link section and add after AdditionalDependencies
    $pattern = '(<ItemDefinitionGroup Condition.*Release\|x64.*?<Link>(?:.*?<SubSystem>Console</SubSystem>)?(?:.*?<EnableCOMDATFolding>true</EnableCOMDATFolding>)?(?:.*?<OptimizeReferences>true</OptimizeReferences>)?(?:.*?<GenerateDebugInformation>true</GenerateDebugInformation>)?.*?<AdditionalDependencies>d3d11\.lib;dxgi\.lib;d3dcompiler\.lib;%\(AdditionalDependencies\)</AdditionalDependencies>)'
    $replacement = "`$1`r`n      <AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>"
    $content = $content -replace $pattern, $replacement
    $modified = $true
    Write-Host "  [ADD] AdditionalManifestFiles to Release configuration" -ForegroundColor Green
}

# Save the modified content
if ($modified) {
    if ($WhatIf) {
        Write-Host ""
        Write-Host "[WHATIF] Would save changes to: $vcxprojPath" -ForegroundColor Yellow
    }
    else {
        Write-Host ""
        Write-Host "[INFO] Saving changes to: $vcxprojPath" -ForegroundColor Green
        
        Set-Content -Path $vcxprojPath -Value $content -NoNewline
        
        Write-Host "[SUCCESS] Project file updated successfully!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Next steps:" -ForegroundColor Cyan
        Write-Host "  1. Open Visual Studio" -ForegroundColor White
        Write-Host "  2. Load the DMATool project (it should load correctly now)" -ForegroundColor White
        Write-Host "  3. Build -> Clean Solution" -ForegroundColor White
        Write-Host "  4. Build -> Rebuild Solution" -ForegroundColor White
        Write-Host "  5. Check that DMATool.exe has a shield icon" -ForegroundColor White
        Write-Host ""
        Write-Host "If the project still fails to load:" -ForegroundColor Yellow
        Write-Host "  Run: Copy-Item '$backupPath' '$vcxprojPath' -Force" -ForegroundColor Yellow
        Write-Host "  Then configure manually in Visual Studio Properties" -ForegroundColor Yellow
    }
}
else {
    Write-Host ""
    Write-Host "[WARNING] Could not find the expected Link sections" -ForegroundColor Yellow
    Write-Host "[INFO] You may need to configure manually in Visual Studio" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Configuration Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
