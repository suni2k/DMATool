# Set-Release-NoConsole.ps1
# Configures Release build to hide console window while keeping Debug with console

$ErrorActionPreference = 'Stop'
$vcxprojPath = "DMATool.vcxproj"

Write-Host "`n[INFO] Configuring console window settings..." -ForegroundColor Cyan
Write-Host "  Debug mode: Console window ENABLED (for debug logs)" -ForegroundColor Gray
Write-Host "  Release mode: Console window DISABLED (clean UI)" -ForegroundColor Gray
Write-Host ""

# Backup project file
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$backupPath = "$vcxprojPath.backup_$timestamp"
Copy-Item $vcxprojPath $backupPath -Force
Write-Host "[BACKUP] Created: $backupPath" -ForegroundColor Gray

# Load XML
[xml]$vcxproj = Get-Content $vcxprojPath

# Create namespace manager
$ns = New-Object System.Xml.XmlNamespaceManager($vcxproj.NameTable)
$ns.AddNamespace("ms", $vcxproj.DocumentElement.NamespaceURI)

# Find ItemDefinitionGroup nodes
$itemDefGroups = $vcxproj.SelectNodes("//ms:ItemDefinitionGroup", $ns)

$modified = $false

foreach ($group in $itemDefGroups) {
    $condition = $group.Condition
    
    if (-not $condition) {
        continue
    }
    
    # Get Link element
    $link = $group.SelectSingleNode("ms:Link", $ns)
    if (-not $link) {
        Write-Host "[WARNING] No Link element found in configuration: $condition" -ForegroundColor Yellow
        continue
    }
    
    # Get or create SubSystem element
    $subSystem = $link.SelectSingleNode("ms:SubSystem", $ns)
    if (-not $subSystem) {
        $subSystem = $vcxproj.CreateElement("SubSystem", $vcxproj.DocumentElement.NamespaceURI)
        $link.AppendChild($subSystem) | Out-Null
    }
    
    # Set based on configuration
    if ($condition -match "Debug") {
        # Debug: Keep Console for debug logs
        if ($subSystem.InnerText -ne "Console") {
            $subSystem.InnerText = "Console"
            $modified = $true
            Write-Host "[CHANGE] Debug -> SubSystem = Console (logs enabled)" -ForegroundColor Green
        } else {
            Write-Host "[OK] Debug already set to Console" -ForegroundColor Gray
        }
    }
    elseif ($condition -match "Release") {
        # Release: Use Windows subsystem to hide console
        if ($subSystem.InnerText -ne "Windows") {
            $subSystem.InnerText = "Windows"
            $modified = $true
            Write-Host "[CHANGE] Release -> SubSystem = Windows (no console)" -ForegroundColor Green
        } else {
            Write-Host "[OK] Release already set to Windows" -ForegroundColor Gray
        }
    }
}

# Save if modified
if ($modified) {
    $vcxproj.Save($vcxprojPath)
    Write-Host "`n[SUCCESS] Project file updated!" -ForegroundColor Green
} else {
    # Remove backup if nothing changed
    Remove-Item $backupPath -Force
    Write-Host "`n[INFO] No changes needed - already configured correctly" -ForegroundColor Cyan
}

Write-Host "`n" -NoNewline
Write-Host "=" -NoNewline -ForegroundColor Cyan
Write-Host "=" -NoNewline -ForegroundColor Cyan
Write-Host "=" -NoNewline -ForegroundColor Cyan
Write-Host " Configuration Summary " -NoNewline -ForegroundColor White
Write-Host "=" -NoNewline -ForegroundColor Cyan
Write-Host "=" -NoNewline -ForegroundColor Cyan
Write-Host "=" -ForegroundColor Cyan

Write-Host "`nBuild Configuration:" -ForegroundColor Yellow
Write-Host "  ? Debug Build:" -ForegroundColor White
Write-Host "    - Console window: VISIBLE" -ForegroundColor Gray
Write-Host "    - Debug logs: Shown in console (std::cout)" -ForegroundColor Gray
Write-Host "    - Use for: Development, debugging, testing" -ForegroundColor Gray
Write-Host ""
Write-Host "  ? Release Build:" -ForegroundColor White
Write-Host "    - Console window: HIDDEN" -ForegroundColor Gray
Write-Host "    - Debug logs: Not visible (clean UI)" -ForegroundColor Gray
Write-Host "    - Use for: Distribution to users" -ForegroundColor Gray

Write-Host "`nNext Steps:" -ForegroundColor Yellow
Write-Host "  1. Close solution in Visual Studio" -ForegroundColor White
Write-Host "  2. Reopen DMATool.vcxproj" -ForegroundColor White
Write-Host "  3. Test Debug build (console should show)" -ForegroundColor White
Write-Host "  4. Test Release build (no console)" -ForegroundColor White

Write-Host "`nNote:" -ForegroundColor Yellow
Write-Host "  If you need to see logs in Release mode, you can:" -ForegroundColor Gray
Write-Host "  - Use OutputDebugString() instead of std::cout" -ForegroundColor Gray
Write-Host "  - View logs with DebugView tool" -ForegroundColor Gray
Write-Host "  - Or temporarily build in Debug mode" -ForegroundColor Gray
Write-Host ""
