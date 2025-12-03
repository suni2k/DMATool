# Fix Release build to hide console window

$vcxprojPath = "DMATool.vcxproj"

Write-Host "Configuring Release build to hide console window..." -ForegroundColor Cyan

[xml]$proj = Get-Content $vcxprojPath

# Find the Release configuration Link section
$releaseLink = $proj.Project.ItemDefinitionGroup | Where-Object { 
    $_.'Condition' -like "*'Release|x64'*" 
} | Select-Object -First 1 | Select-Object -ExpandProperty Link

if ($releaseLink) {
    # Change SubSystem to Windows
    $releaseLink.SubSystem = "Windows"
    
    # Add EntryPointSymbol to use main() instead of WinMain()
    if ($releaseLink.EntryPointSymbol) {
        $releaseLink.EntryPointSymbol = "mainCRTStartup"
    } else {
        $elem = $proj.CreateElement("EntryPointSymbol", $proj.Project.NamespaceURI)
        $elem.InnerText = "mainCRTStartup"
        $releaseLink.AppendChild($elem) | Out-Null
    }
    
    Write-Host "  ? Set SubSystem=Windows for Release" -ForegroundColor Green
    Write-Host "  ? Set EntryPointSymbol=mainCRTStartup for Release" -ForegroundColor Green
}

# Save the file
$proj.Save($vcxprojPath)

Write-Host ""
Write-Host "Done! Release builds will now:" -ForegroundColor Green
Write-Host "  - Hide the console window" -ForegroundColor Gray
Write-Host "  - Use main() entry point (no code changes needed)" -ForegroundColor Gray
Write-Host "  - Require administrator privileges" -ForegroundColor Gray
Write-Host ""
Write-Host "Debug builds will still show the console for logging." -ForegroundColor Cyan
