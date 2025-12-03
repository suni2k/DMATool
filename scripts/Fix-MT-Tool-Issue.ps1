# Fix-MT-Tool-Issue.ps1
# Comprehensive fix for LNK1327 (mt.exe failure)
#
# NOTE: If this script doesn't resolve the issue, use the improved version:
#       .\scripts\Fix-Build-MT-Issue.ps1
#

$ErrorActionPreference = 'Stop'
$workspaceRoot = Split-Path -Parent $PSScriptRoot

Write-Host "[INFO] Diagnosing LNK1327 mt.exe failure..." -ForegroundColor Cyan
Write-Host ""

# Step 1: Check if running as Admin
Write-Host "[1/8] Checking admin privileges..."
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if ($isAdmin) {
    Write-Host "  [OK] Running as Administrator" -ForegroundColor Green
} else {
    Write-Host "  [WARNING] Not running as Administrator - some fixes may not work" -ForegroundColor Yellow
}

# Step 2: Find mt.exe
Write-Host "[2/8] Locating mt.exe..."
$mtPaths = @(
    "${env:ProgramFiles(x86)}\Windows Kits\10\bin\*\x64\mt.exe"
    "${env:ProgramFiles(x86)}\Windows Kits\10\bin\x64\mt.exe"
    "${env:ProgramFiles(x86)}\Microsoft SDKs\Windows\*\bin\*\x64\mt.exe"
)

$foundMt = $false
foreach ($pattern in $mtPaths) {
    $found = Get-Item $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        Write-Host "  [OK] Found: $($found.FullName)" -ForegroundColor Green
        $mtExePath = $found.FullName
        $foundMt = $true
        break
    }
}

if (-not $foundMt) {
    Write-Host "  [ERROR] mt.exe not found! Install Windows SDK" -ForegroundColor Red
    exit 1
}

# Step 3: Test mt.exe execution
Write-Host "[3/8] Testing mt.exe execution..."
try {
    $testOutput = & $mtExePath -nologo 2>&1
    Write-Host "  [OK] mt.exe can execute" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Cannot execute mt.exe: $_" -ForegroundColor Red
    Write-Host "  This might be blocked by antivirus!" -ForegroundColor Yellow
}

# Step 4: Check Windows Defender exclusions
Write-Host "[4/8] Checking Windows Defender exclusions..."
try {
    $preferences = Get-MpPreference -ErrorAction SilentlyContinue
    if ($preferences) {
        $mtDir = Split-Path -Parent $mtExePath
        $vsDir = "${env:ProgramFiles}\Microsoft Visual Studio"
        
        $needsExclusion = @()
        if ($preferences.ExclusionPath -notcontains $mtDir) {
            $needsExclusion += $mtDir
        }
        if ($preferences.ExclusionPath -notcontains $workspaceRoot) {
            $needsExclusion += $workspaceRoot
        }
        
        if ($needsExclusion.Count -gt 0) {
            Write-Host "  [WARNING] Missing exclusions detected" -ForegroundColor Yellow
            if ($isAdmin) {
                foreach ($path in $needsExclusion) {
                    Write-Host "  Adding exclusion: $path" -ForegroundColor Cyan
                    Add-MpPreference -ExclusionPath $path
                }
                Write-Host "  [OK] Exclusions added" -ForegroundColor Green
            } else {
                Write-Host "  [ACTION REQUIRED] Run as Admin to add exclusions:" -ForegroundColor Yellow
                foreach ($path in $needsExclusion) {
                    Write-Host "    Add-MpPreference -ExclusionPath '$path'" -ForegroundColor Gray
                }
            }
        } else {
            Write-Host "  [OK] Proper exclusions exist" -ForegroundColor Green
        }
    }
} catch {
    Write-Host "  [SKIP] Cannot check Defender (not available or disabled)" -ForegroundColor Gray
}

# Step 5: Clean build artifacts
Write-Host "[5/8] Cleaning build artifacts..."
$cleanDirs = @('bin', 'bin-int', 'x64', '.vs')
foreach ($dir in $cleanDirs) {
    $fullPath = Join-Path $workspaceRoot $dir
    if (Test-Path $fullPath) {
        Remove-Item $fullPath -Recurse -Force
        Write-Host "  Deleted: $dir" -ForegroundColor Gray
    }
}
Write-Host "  [OK] Build artifacts cleaned" -ForegroundColor Green

# Step 6: Check project file for manifest settings
Write-Host "[6/8] Checking project manifest configuration..."
$vcxprojPath = Join-Path $workspaceRoot "DMATool.vcxproj"
if (Test-Path $vcxprojPath) {
    [xml]$vcxproj = Get-Content $vcxprojPath
    
    # Check for problematic manifest settings
    $manifestNodes = $vcxproj.SelectNodes("//Manifest")
    $linkNodes = $vcxproj.SelectNodes("//Link")
    
    $hasIssue = $false
    foreach ($node in $manifestNodes) {
        if ($node.EnableDpiAwareness -or $node.GenerateCatalogFiles -or $node.AdditionalManifestFiles) {
            Write-Host "  [WARNING] Found potentially problematic manifest settings" -ForegroundColor Yellow
            $hasIssue = $true
        }
    }
    
    if (-not $hasIssue) {
        Write-Host "  [OK] Manifest configuration looks good" -ForegroundColor Green
    }
} else {
    Write-Host "  [ERROR] DMATool.vcxproj not found!" -ForegroundColor Red
}

# Step 7: Check app.manifest
Write-Host "[7/8] Checking app.manifest..."
$manifestPath = Join-Path $workspaceRoot "app.manifest"
if (Test-Path $manifestPath) {
    $manifestContent = Get-Content $manifestPath -Raw
    
    # Try to parse with mt.exe
    $tempOutput = Join-Path $env:TEMP "mt_test_$(Get-Random).txt"
    try {
        & $mtExePath -inputresource:$manifestPath -nologo -out:$tempOutput 2>&1 | Out-Null
        if (Test-Path $tempOutput) {
            Remove-Item $tempOutput -Force
        }
        Write-Host "  [OK] app.manifest is valid" -ForegroundColor Green
    } catch {
        Write-Host "  [WARNING] app.manifest validation failed: $_" -ForegroundColor Yellow
    }
} else {
    Write-Host "  [ERROR] app.manifest not found!" -ForegroundColor Red
}

# Step 8: Apply workaround - disable manifest embedding temporarily
Write-Host "[8/8] Applying workaround..."
Write-Host "  [INFO] Creating backup of .vcxproj..." -ForegroundColor Cyan

$backupPath = "$vcxprojPath.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
Copy-Item $vcxprojPath $backupPath

[xml]$vcxproj = Get-Content $vcxprojPath

$modified = $false

# Find all ItemDefinitionGroup nodes
$itemDefGroups = $vcxproj.SelectNodes("//ItemDefinitionGroup")
foreach ($group in $itemDefGroups) {
    $link = $group.Link
    if (-not $link) {
        $link = $vcxproj.CreateElement("Link", $vcxproj.DocumentElement.NamespaceURI)
        $group.AppendChild($link) | Out-Null
    }
    
    # Check if GenerateDebugInformation exists
    if (-not $link.GenerateDebugInformation) {
        $genDebug = $vcxproj.CreateElement("GenerateDebugInformation", $vcxproj.DocumentElement.NamespaceURI)
        $genDebug.InnerText = "true"
        $link.AppendChild($genDebug) | Out-Null
        $modified = $true
    }
}

if ($modified) {
    $vcxproj.Save($vcxprojPath)
    Write-Host "  [OK] Applied project file fix" -ForegroundColor Green
    Write-Host "  [INFO] Backup saved: $(Split-Path -Leaf $backupPath)" -ForegroundColor Cyan
} else {
    Remove-Item $backupPath
    Write-Host "  [INFO] No changes needed" -ForegroundColor Gray
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[DIAGNOSIS COMPLETE]" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Try building the project in Visual Studio" -ForegroundColor White
Write-Host "2. If it still fails, check Event Viewer for detailed errors:" -ForegroundColor White
Write-Host "   - Windows Logs -> Application" -ForegroundColor Gray
Write-Host "   - Look for errors from 'mt.exe' or linker" -ForegroundColor Gray
Write-Host "3. Consider temporarily disabling antivirus" -ForegroundColor White
Write-Host "4. Verify Windows SDK installation via Visual Studio Installer" -ForegroundColor White
Write-Host ""

if (-not $isAdmin) {
    Write-Host "[TIP] For full diagnostic capabilities, run this script as Administrator" -ForegroundColor Yellow
    Write-Host ""
}
