# Fix manifest configuration for Visual Studio 2022+
# The AdditionalManifestFiles option is deprecated - use proper manifest embedding instead

$vcxprojPath = "DMATool.vcxproj"

Write-Host "Fixing manifest configuration for VS 2022..." -ForegroundColor Cyan

# Read the project file
$content = Get-Content $vcxprojPath -Raw

# Remove deprecated AdditionalManifestFiles
$content = $content -replace '\s*<AdditionalManifestFiles>app\.manifest</AdditionalManifestFiles>\s*', ''

# Remove Manifest ItemGroup (we'll embed it properly instead)
$content = $content -replace '\s*<ItemGroup>\s*<Manifest Include="app\.manifest"\s*/>\s*</ItemGroup>\s*', ''

# Add /FS flag for parallel compilation (fixes C1041 errors)
$content = $content -replace '(<ClCompile>)', '$1`n      <MinimalRebuild>false</MinimalRebuild>`n      <MultiProcessorCompilation>true</MultiProcessorCompilation>`n      <AdditionalOptions>/FS %(AdditionalOptions)</AdditionalOptions>'

# Save the modified file
$content | Set-Content $vcxprojPath -NoNewline

Write-Host "Done! The project now uses embedded manifest properly." -ForegroundColor Green
Write-Host ""
Write-Host "Changes made:" -ForegroundColor Yellow
Write-Host "  - Removed deprecated AdditionalManifestFiles" -ForegroundColor Gray
Write-Host "  - Removed Manifest ItemGroup entry" -ForegroundColor Gray
Write-Host "  - Added /FS flag for parallel compilation" -ForegroundColor Gray
Write-Host ""
Write-Host "The manifest will be embedded via the .rc file instead." -ForegroundColor Cyan
