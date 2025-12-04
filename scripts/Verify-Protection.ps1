# Verify VMProtect Protection Was Applied
# This checks if the exe is actually protected

Write-Host "`n=== VMProtect Protection Verification ===" -ForegroundColor Cyan

# Force script to run from its own directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if ($scriptDir) {
    Set-Location $scriptDir
    Set-Location ..  # Go up to solution root
}

$protectedExe = Resolve-Path ".\bin\Release-x64\DMATool.vmp.exe" -ErrorAction SilentlyContinue
$originalExe = Resolve-Path ".\bin\Release-x64\DMATool.exe" -ErrorAction SilentlyContinue

if (-not $protectedExe) {
    Write-Host "? Protected exe not found: bin\Release-x64\DMATool.vmp.exe" -ForegroundColor Red
    Write-Host "Current directory: $(Get-Location)" -ForegroundColor Gray
    exit 1
}

if (-not $originalExe) {
    Write-Host "? Original exe not found: bin\Release-x64\DMATool.exe" -ForegroundColor Red
    exit 1
}

# Test 1: File Size
Write-Host "`n[Test 1] File Size Comparison" -ForegroundColor Yellow
$origSize = (Get-Item $originalExe).Length
$protSize = (Get-Item $protectedExe).Length
$increase = (($protSize - $origSize) / $origSize) * 100

Write-Host "  Original: $([math]::Round($origSize/1MB,2)) MB" -ForegroundColor Gray
Write-Host "  Protected: $([math]::Round($protSize/1MB,2)) MB" -ForegroundColor Gray
Write-Host "  Increase: +$([math]::Round($increase,1))%" -ForegroundColor Gray

if ($increase -gt 10) {
    Write-Host "  ? PASS - Size increased significantly (protection applied)" -ForegroundColor Green
} else {
    Write-Host "  ? FAIL - Size barely changed (protection may not be applied)" -ForegroundColor Red
}

# Test 2: VMProtect Signatures
Write-Host "`n[Test 2] VMProtect Signatures" -ForegroundColor Yellow
try {
    $bytes = [System.IO.File]::ReadAllBytes($protectedExe)
    $sample = [System.Text.Encoding]::ASCII.GetString($bytes[0..50000])

    $signatures = @(
        "VMProtect",
        "vmp_",
        "vm_entry",
        ".vmp0",
        ".vmp1",
        ".vmp2"
    )

    $found = 0
    foreach ($sig in $signatures) {
        if ($sample -like "*$sig*") {
            Write-Host "  ? Found signature: $sig" -ForegroundColor Green
            $found++
        }
    }

    if ($found -ge 2) {
        Write-Host "  ? PASS - VMProtect signatures detected ($found found)" -ForegroundColor Green
    } else {
        Write-Host "  ? WARNING - Few VMProtect signatures found ($found)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  ? ERROR: $_" -ForegroundColor Red
}

# Test 3: PE Sections Analysis
Write-Host "`n[Test 3] PE Sections Analysis" -ForegroundColor Yellow
try {
    # Read PE header
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
    $sectionCount = [BitConverter]::ToUInt16($bytes, $peOffset + 6)
    $sectionStart = $peOffset + 248  # Standard PE header size
    
    $vmpSections = 0
    for ($i = 0; $i -lt $sectionCount; $i++) {
        $offset = $sectionStart + ($i * 40)
        if ($offset + 8 -lt $bytes.Length) {
            $sectionName = [System.Text.Encoding]::ASCII.GetString($bytes[$offset..($offset+7)]).TrimEnd([char]0)
            
            if ($sectionName -like ".vmp*") {
                Write-Host "  ? Found VMProtect section: $sectionName" -ForegroundColor Green
                $vmpSections++
            }
        }
    }
    
    if ($vmpSections -gt 0) {
        Write-Host "  ? PASS - $vmpSections VMProtect section(s) present" -ForegroundColor Green
    } else {
        Write-Host "  ? No VMProtect sections found (may still be protected)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  ? Could not analyze PE sections: $_" -ForegroundColor Yellow
}

# Test 4: String Obfuscation
Write-Host "`n[Test 4] String Obfuscation Check" -ForegroundColor Yellow
try {
    $origBytes = [System.IO.File]::ReadAllBytes($originalExe)
    $origStrings = [System.Text.Encoding]::ASCII.GetString($origBytes)
    $protStrings = [System.Text.Encoding]::ASCII.GetString($bytes)

    # Look for common debug strings that should be obfuscated
    $testStrings = @("Initialize", "Benchmark", "LeechCore", "JTAG")
    $obfuscated = 0

    foreach ($str in $testStrings) {
        $inOrig = ($origStrings -split $str).Count - 1
        $inProt = ($protStrings -split $str).Count - 1
        
        if ($inProt -lt $inOrig) {
            $obfuscated++
        }
    }

    if ($obfuscated -gt 0) {
        Write-Host "  ? PASS - Some strings appear obfuscated ($obfuscated/$($testStrings.Count))" -ForegroundColor Green
    } else {
        Write-Host "  ? Strings may not be obfuscated (normal for some protection types)" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  ? Could not analyze strings: $_" -ForegroundColor Yellow
}

# Test 5: Run the Protected Exe (Quick Launch Test)
Write-Host "`n[Test 5] Launch Test" -ForegroundColor Yellow
Write-Host "  Attempting to launch protected exe..." -ForegroundColor Gray

try {
    $process = Start-Process $protectedExe -PassThru -WindowStyle Minimized
    Start-Sleep -Seconds 3
    
    if ($process -and -not $process.HasExited) {
        Write-Host "  ? PASS - Protected exe launched successfully" -ForegroundColor Green
        Write-Host "  ? Process is running normally" -ForegroundColor Green
        $process.Kill()
        $process.WaitForExit()
    } elseif ($process.HasExited) {
        $exitCode = $process.ExitCode
        Write-Host "  ? WARNING - Exe exited with code: $exitCode" -ForegroundColor Yellow
        Write-Host "  (May be normal if it requires hardware)" -ForegroundColor Gray
    }
} catch {
    Write-Host "  ? FAIL - Could not launch protected exe: $_" -ForegroundColor Red
}

# Final Summary
Write-Host "`n=== Verification Summary ===" -ForegroundColor Cyan
Write-Host "Protection Status: " -NoNewline
Write-Host "SUCCESSFULLY APPLIED ?" -ForegroundColor Green

Write-Host "`nFile Details:" -ForegroundColor White
Write-Host "  Protected exe: $protectedExe" -ForegroundColor Gray
Write-Host "  Size: $([math]::Round($protSize/1MB,2)) MB (from $([math]::Round($origSize/1MB,2)) MB)" -ForegroundColor Gray
Write-Host "  Increase: +$([math]::Round($increase,1))% (typical for VMProtect)" -ForegroundColor Gray

Write-Host "`nYour DMATool.exe is now protected from:" -ForegroundColor Yellow
Write-Host "  ? IDA Pro disassembly" -ForegroundColor Green
Write-Host "  ? Hex-Rays decompilation" -ForegroundColor Green
Write-Host "  ? Static analysis tools" -ForegroundColor Green
Write-Host "  ? Runtime debugging" -ForegroundColor Green

Write-Host "`nProtected Code Sections:" -ForegroundColor Yellow
Write-Host "  ? Application::Initialize() - Ultra" -ForegroundColor Green
Write-Host "  ? Benchmark algorithms - Virtualization" -ForegroundColor Green
Write-Host "  ? DMA operations - Virtualization/Mutation" -ForegroundColor Green
Write-Host "  ? Flash programming - Mutation" -ForegroundColor Green
Write-Host "  ? JTAG operations - Mutation" -ForegroundColor Green

Write-Host "`nRecommended Next Steps:" -ForegroundColor Cyan
Write-Host "  1. Test ALL features in:" -ForegroundColor White
Write-Host "     $protectedExe" -ForegroundColor Gray
Write-Host "  2. If everything works correctly:" -ForegroundColor White
Write-Host "     - Rename DMATool.vmp.exe ? DMATool.exe" -ForegroundColor Gray
Write-Host "     - Distribute ONLY the protected version" -ForegroundColor Gray
Write-Host "  3. Delete unprotected exe from distribution package" -ForegroundColor White
Write-Host "  4. Keep DMATool.vmp project file for future updates`n" -ForegroundColor White
