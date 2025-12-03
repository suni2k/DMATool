# Disable-Manifest-Embedding.ps1
# Temporary workaround for LNK1327 by disabling manifest embedding

$vcxprojPath = "DMATool.vcxproj"
$backupPath = "$vcxprojPath.backup_mt_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

Write-Host "[INFO] Disabling manifest embedding as workaround for LNK1327..." -ForegroundColor Cyan

# Backup
Copy-Item $vcxprojPath $backupPath
Write-Host "[INFO] Backup created: $backupPath" -ForegroundColor Gray

# Load and modify
[xml]$proj = Get-Content $vcxprojPath

# Find all ItemDefinitionGroup nodes
$itemDefGroups = $proj.SelectNodes("//ItemDefinitionGroup")

foreach ($group in $itemDefGroups) {
    # Get or create Link element
    $link = $group.Link
    if (-not $link) {
        $link = $proj.CreateElement("Link", $proj.DocumentElement.NamespaceURI)
        $group.AppendChild($link) | Out-Null
    }
    
    # Add or update GenerateDebugInformation
    $genDebugNode = $link.SelectSingleNode("GenerateDebugInformation")
    if (-not $genDebugNode) {
        $genDebugNode = $proj.CreateElement("GenerateDebugInformation", $proj.DocumentElement.NamespaceURI)
        $link.AppendChild($genDebugNode) | Out-Null
    }
    $genDebugNode.InnerText = "DebugFull"
    
    # Disable manifest embedding
    $embedManifestNode = $link.SelectSingleNode("EmbedManifest")
    if (-not $embedManifestNode) {
        $embedManifestNode = $proj.CreateElement("EmbedManifest", $proj.DocumentElement.NamespaceURI)
        $link.AppendChild($embedManifestNode) | Out-Null
    }
    $embedManifestNode.InnerText = "false"
}

$proj.Save((Resolve-Path $vcxprojPath).Path)

Write-Host "[SUCCESS] Manifest embedding disabled" -ForegroundColor Green
Write-Host ""
Write-Host "This is a temporary workaround. The manifest will NOT be embedded in the .exe" -ForegroundColor Yellow
Write-Host "You'll need to keep app.manifest in the same folder as the .exe" -ForegroundColor Yellow
Write-Host ""
Write-Host "To revert: Copy $(Split-Path -Leaf $backupPath) back to DMATool.vcxproj" -ForegroundColor Gray
