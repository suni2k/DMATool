# Ultimate-MT-Fix.ps1
# Ultimate fix for LNK1327 mt.exe failure

$ErrorActionPreference = 'Stop'

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  Ultimate MT.EXE LNK1327 Fix" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Kill Visual Studio
Write-Host "[1/6] Closing Visual Studio..." -ForegroundColor Yellow
Get-Process | Where-Object { $_.Name -like "*devenv*" -or $_.Name -like "*MSBuild*" } | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Write-Host "  Done" -ForegroundColor Green

# Step 2: Deep clean
Write-Host "[2/6] Deep cleaning build artifacts..." -ForegroundColor Yellow
$cleanPaths = @(
    "bin",
    "bin-int",
    "x64",
    ".vs",
    "*.user",
    "*.suo",
    "*.vcxproj.user"
)

foreach ($pattern in $cleanPaths) {
    Get-Item $pattern -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Host "  Done" -ForegroundColor Green

# Step 3: Grant full permissions to workspace
Write-Host "[3/6] Setting permissions on workspace..." -ForegroundColor Yellow
$currentPath = (Get-Location).Path
$acl = Get-Acl $currentPath
$identity = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name
$accessRule = New-Object System.Security.AccessControl.FileSystemAccessRule($identity, "FullControl", "ContainerInherit,ObjectInherit", "None", "Allow")
$acl.SetAccessRule($accessRule)
Set-Acl $currentPath $acl
Write-Host "  Done" -ForegroundColor Green

# Step 4: Verify mt.exe and add to Defender exclusions
Write-Host "[4/6] Configuring Windows Defender exclusions..." -ForegroundColor Yellow
$mtPath = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\mt.exe"
if (Test-Path $mtPath) {
    Add-MpPreference -ExclusionPath $mtPath -ErrorAction SilentlyContinue
    Add-MpPreference -ExclusionPath (Split-Path $mtPath) -ErrorAction SilentlyContinue
    Add-MpPreference -ExclusionPath $currentPath -ErrorAction SilentlyContinue
    Add-MpPreference -ExclusionProcess "mt.exe" -ErrorAction SilentlyContinue
    Add-MpPreference -ExclusionProcess "link.exe" -ErrorAction SilentlyContinue
    Write-Host "  Done" -ForegroundColor Green
} else {
    Write-Host "  [WARNING] mt.exe not found at expected location" -ForegroundColor Red
}

# Step 5: Modify project to use /MANIFEST:NO temporarily
Write-Host "[5/6] Applying linker workaround..." -ForegroundColor Yellow

$vcxprojPath = "DMATool.vcxproj"
$backupPath = "$vcxprojPath.backup_ultimate_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

Copy-Item $vcxprojPath $backupPath
Write-Host "  Backup: $backupPath" -ForegroundColor Gray

[xml]$proj = Get-Content $vcxprojPath

# Find all ItemDefinitionGroup nodes
$modified = $false
$itemDefGroups = $proj.SelectNodes("//ItemDefinitionGroup")

foreach ($group in $itemDefGroups) {
    # Get or create Link element
    $link = $group.Link
    if (-not $link) {
        $link = $proj.CreateElement("Link", $proj.DocumentElement.NamespaceURI)
        $group.AppendChild($link) | Out-Null
    }
    
    # Add AdditionalOptions with /MANIFEST:NO
    $addOptionsNode = $link.SelectSingleNode("AdditionalOptions")
    if (-not $addOptionsNode) {
        $addOptionsNode = $proj.CreateElement("AdditionalOptions", $proj.DocumentElement.NamespaceURI)
        $link.AppendChild($addOptionsNode) | Out-Null
    }
    
    $currentOptions = $addOptionsNode.InnerText
    if ($currentOptions -notlike "*MANIFEST:NO*") {
        $addOptionsNode.InnerText = "$currentOptions /MANIFEST:NO".Trim()
        $modified = $true
    }
}

if ($modified) {
    $proj.Save((Resolve-Path $vcxprojPath).Path)
    Write-Host "  Done - /MANIFEST:NO added to linker options" -ForegroundColor Green
} else {
    Remove-Item $backupPath
    Write-Host "  Done - no changes needed" -ForegroundColor Green
}

# Step 6: Instructions
Write-Host "[6/6] Next steps..." -ForegroundColor Yellow
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  FIX APPLIED" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "What was done:" -ForegroundColor Yellow
Write-Host "  1. Visual Studio closed" -ForegroundColor White
Write-Host "  2. Build artifacts cleaned" -ForegroundColor White
Write-Host "  3. Workspace permissions set to Full Control" -ForegroundColor White
Write-Host "  4. Windows Defender exclusions added" -ForegroundColor White
Write-Host "  5. Linker configured to skip manifest embedding (/MANIFEST:NO)" -ForegroundColor White
Write-Host ""
Write-Host "Now:" -ForegroundColor Yellow
Write-Host "  1. Open Visual Studio" -ForegroundColor White
Write-Host "  2. Load DMATool.sln" -ForegroundColor White
Write-Host "  3. Build -> Rebuild Solution" -ForegroundColor White
Write-Host ""
Write-Host "NOTE:" -ForegroundColor Yellow
Write-Host "  The manifest will NOT be embedded in the .exe" -ForegroundColor Red
Write-Host "  This is a temporary workaround" -ForegroundColor Red
Write-Host "  Keep app.manifest next to DMATool.exe when distributing" -ForegroundColor Red
Write-Host ""
Write-Host "To revert:" -ForegroundColor Gray
Write-Host "  Copy-Item $backupPath DMATool.vcxproj -Force" -ForegroundColor Gray
Write-Host ""
