# Properly fix the vcxproj file for VS 2022+ manifest embedding

$vcxprojPath = "DMATool.vcxproj"

Write-Host "Loading project file..." -ForegroundColor Cyan
[xml]$proj = Get-Content $vcxprojPath

# Find all ItemDefinitionGroup elements with Link sections
$linkElements = $proj.Project.ItemDefinitionGroup.Link

foreach ($link in $linkElements) {
    # Remove AdditionalManifestFiles if it exists
    if ($link.AdditionalManifestFiles) {
        Write-Host "Removing AdditionalManifestFiles..." -ForegroundColor Yellow
        $link.RemoveChild($link.AdditionalManifestFiles) | Out-Null
    }
    
    # Set GenerateManifest to false (we're embedding via RC)
    if ($link.GenerateManifest) {
        $link.GenerateManifest = "false"
    } else {
        $elem = $proj.CreateElement("GenerateManifest", $proj.Project.NamespaceURI)
        $elem.InnerText = "false"
        $link.AppendChild($elem) | Out-Null
    }
}

# Remove Manifest ItemGroup
$manifestItemGroup = $proj.Project.ItemGroup | Where-Object { $_.Manifest }
if ($manifestItemGroup) {
    Write-Host "Removing Manifest ItemGroup..." -ForegroundColor Yellow
    $proj.Project.RemoveChild($manifestItemGroup) | Out-Null
}

# Save the file
$proj.Save($vcxprojPath)

Write-Host "Project file fixed successfully!" -ForegroundColor Green
Write-Host "  - Removed AdditionalManifestFiles" -ForegroundColor Gray
Write-Host "  - Set GenerateManifest=false" -ForegroundColor Gray
Write-Host "  - Removed Manifest ItemGroup" -ForegroundColor Gray
Write-Host "  - Manifest will be embedded via DMATool.rc" -ForegroundColor Cyan
