# Fix-Build-MT-Issue.ps1
# Aggressive fix for LNK1327 mt.exe failure

$ErrorActionPreference = 'Stop'
$workspaceRoot = Split-Path -Parent $PSScriptRoot

Write-Host "`n[INFO] Applying aggressive mt.exe fix..." -ForegroundColor Cyan

# Step 1: Kill any stuck mt.exe processes
Write-Host "`n[1/5] Checking for stuck mt.exe processes..."
$mtProcesses = Get-Process mt -ErrorAction SilentlyContinue
if ($mtProcesses) {
    Write-Host "  [ACTION] Killing $($mtProcesses.Count) mt.exe process(es)..." -ForegroundColor Yellow
    $mtProcesses | Stop-Process -Force
    Start-Sleep -Seconds 1
    Write-Host "  [OK] Processes terminated" -ForegroundColor Green
} else {
    Write-Host "  [OK] No stuck processes" -ForegroundColor Green
}

# Step 2: Clean all build artifacts aggressively
Write-Host "`n[2/5] Deep cleaning build artifacts..."
$cleanPaths = @(
    "bin-int\*"
    "bin\*"
    "x64\*"
    ".vs\*"
    "*.tlog"
)

foreach ($pattern in $cleanPaths) {
    $items = Get-ChildItem -Path $workspaceRoot -Include $pattern -Recurse -ErrorAction SilentlyContinue
    if ($items) {
        $items | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
    }
}
Write-Host "  [OK] Deep clean completed" -ForegroundColor Green

# Step 3: Backup and modify project file
Write-Host "`n[3/5] Modifying project file..."
$vcxprojPath = Join-Path $workspaceRoot "DMATool.vcxproj"

# Create timestamped backup
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$backupPath = "$vcxprojPath.backup_$timestamp"
Copy-Item $vcxprojPath $backupPath -Force
Write-Host "  [INFO] Backup: DMATool.vcxproj.backup_$timestamp" -ForegroundColor Gray

# Load XML
[xml]$vcxproj = Get-Content $vcxprojPath

# Get or create namespace manager
$ns = New-Object System.Xml.XmlNamespaceManager($vcxproj.NameTable)
$ns.AddNamespace("ms", $vcxproj.DocumentElement.NamespaceURI)

# Find all ItemDefinitionGroup nodes
$itemDefGroups = $vcxproj.SelectNodes("//ms:ItemDefinitionGroup", $ns)

$modified = $false
foreach ($group in $itemDefGroups) {
    # Get or create Link element
    $link = $group.SelectSingleNode("ms:Link", $ns)
    if (-not $link) {
        $link = $vcxproj.CreateElement("Link", $vcxproj.DocumentElement.NamespaceURI)
        $group.AppendChild($link) | Out-Null
    }
    
    # Set UACExecutionLevel to disable manifest embedding
    $uacLevel = $link.SelectSingleNode("ms:UACExecutionLevel", $ns)
    if (-not $uacLevel) {
        $uacLevel = $vcxproj.CreateElement("UACExecutionLevel", $vcxproj.DocumentElement.NamespaceURI)
        $link.AppendChild($uacLevel) | Out-Null
    }
    $uacLevel.InnerText = "AsInvoker"
    
    # Ensure GenerateDebugInformation exists
    $genDebug = $link.SelectSingleNode("ms:GenerateDebugInformation", $ns)
    if (-not $genDebug) {
        $genDebug = $vcxproj.CreateElement("GenerateDebugInformation", $vcxproj.DocumentElement.NamespaceURI)
        $link.AppendChild($genDebug) | Out-Null
    }
    if ([string]::IsNullOrWhiteSpace($genDebug.InnerText)) {
        $genDebug.InnerText = "true"
    }
    
    # Add EmbedManifest and set to false as workaround
    $embedManifest = $link.SelectSingleNode("ms:EmbedManifest", $ns)
    if (-not $embedManifest) {
        $embedManifest = $vcxproj.CreateElement("EmbedManifest", $vcxproj.DocumentElement.NamespaceURI)
        $link.AppendChild($embedManifest) | Out-Null
        $embedManifest.InnerText = "false"
        $modified = $true
        Write-Host "  [CHANGE] Added EmbedManifest=false to workaround mt.exe" -ForegroundColor Yellow
    }
}

# Save if modified
if ($modified) {
    $vcxproj.Save($vcxprojPath)
    Write-Host "  [OK] Project file updated" -ForegroundColor Green
} else {
    Write-Host "  [INFO] No changes needed" -ForegroundColor Gray
}

# Step 4: Add Windows Defender exclusions (if admin)
Write-Host "`n[4/5] Checking Windows Defender exclusions..."
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin) {
    try {
        $mtPath = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin\*\x64\mt.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($mtPath) {
            $mtDir = Split-Path -Parent $mtPath.FullName
            Add-MpPreference -ExclusionPath $workspaceRoot -ErrorAction SilentlyContinue
            Add-MpPreference -ExclusionPath $mtDir -ErrorAction SilentlyContinue
            Write-Host "  [OK] Added Defender exclusions" -ForegroundColor Green
        }
    } catch {
        Write-Host "  [SKIP] Could not add exclusions: $_" -ForegroundColor Gray
    }
} else {
    Write-Host "  [SKIP] Not admin - run as Administrator to add exclusions" -ForegroundColor Gray
}

# Step 5: Reload solution hint
Write-Host "`n[5/5] Post-fix instructions..."
Write-Host "  [ACTION REQUIRED] In Visual Studio:" -ForegroundColor Yellow
Write-Host "    1. Close the solution (File -> Close Solution)" -ForegroundColor White
Write-Host "    2. Reopen DMATool.vcxproj" -ForegroundColor White
Write-Host "    3. Clean Solution (Build -> Clean Solution)" -ForegroundColor White
Write-Host "    4. Rebuild Solution (Build -> Rebuild Solution)" -ForegroundColor White

Write-Host "`n[SUCCESS] Fix script completed!" -ForegroundColor Green
Write-Host "`nIf the issue persists:" -ForegroundColor Yellow
Write-Host "  - Check Event Viewer (Windows Logs -> Application) for mt.exe errors" -ForegroundColor Gray
Write-Host "  - Temporarily disable antivirus" -ForegroundColor Gray
Write-Host "  - Restore backup: Copy-Item DMATool.vcxproj.backup_$timestamp DMATool.vcxproj -Force" -ForegroundColor Gray
Write-Host ""
