# Set UAC execution level to requireAdministrator in the linker

$vcxprojPath = "DMATool.vcxproj"

Write-Host "Setting UAC execution level to requireAdministrator..." -ForegroundColor Cyan
[xml]$proj = Get-Content $vcxprojPath

# Find all ItemDefinitionGroup elements with Link sections
$linkElements = $proj.Project.ItemDefinitionGroup.Link

foreach ($link in $linkElements) {
    # Set UACExecutionLevel to requireAdministrator
    if ($link.UACExecutionLevel) {
        $link.UACExecutionLevel = "RequireAdministrator"
    } else {
        $elem = $proj.CreateElement("UACExecutionLevel", $proj.Project.NamespaceURI)
        $elem.InnerText = "RequireAdministrator"
        $link.AppendChild($elem) | Out-Null
    }
    
    Write-Host "  - Set UACExecutionLevel=RequireAdministrator" -ForegroundColor Gray
}

# Save the file
$proj.Save($vcxprojPath)

Write-Host "Done! The app will now require administrator privileges." -ForegroundColor Green
