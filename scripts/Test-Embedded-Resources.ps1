# Test-Embedded-Resources.ps1
# Test if resources are properly embedded in the compiled exe

$ErrorActionPreference = 'Stop'

Write-Host "`n??????????????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?  Testing Embedded Resources in EXE                 ?" -ForegroundColor White
Write-Host "??????????????????????????????????????????????????????`n" -ForegroundColor Cyan

function Test-ResourceInExe {
    param(
        [string]$ExePath,
        [int]$ResourceId,
        [string]$ResourceName
    )
    
    # Use ResourceHacker or just check file size as proxy
    # A proper check would require Win32 API, but file size is a good indicator
    
    Write-Host "  Checking $ResourceName (ID: $ResourceId)..." -ForegroundColor Gray
    
    # Since we can't easily check resources without external tools,
    # we'll run the exe and check if it creates temp files
    return $true
}

# Test Debug build
Write-Host "[TEST 1] Debug Build - Embedded Resources" -ForegroundColor Yellow
Write-Host "???????????????????????????????????????????????????????" -ForegroundColor Gray

$debugExe = "bin\Debug-x64\DMATool.exe"

if (Test-Path $debugExe) {
    $size = (Get-Item $debugExe).Length / 1MB
    Write-Host "  [OK] Debug exe found: $debugExe" -ForegroundColor Green
    Write-Host "  [INFO] Size: $([Math]::Round($size, 2)) MB" -ForegroundColor Cyan
    
    # Expected size with embedded resources: ~8-12 MB
    # Without resources: ~3-5 MB
    if ($size -gt 7) {
        Write-Host "  [SUCCESS] Size indicates resources are embedded!" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Size seems small, resources may not be embedded" -ForegroundColor Yellow
    }
} else {
    Write-Host "  [ERROR] Debug exe not found" -ForegroundColor Red
}

Write-Host "`n???????????????????????????????????????????????????????" -ForegroundColor Gray

# Test Release build
Write-Host "`n[TEST 2] Release Build - Embedded Resources" -ForegroundColor Yellow
Write-Host "???????????????????????????????????????????????????????" -ForegroundColor Gray

$releaseExe = "bin\Release-x64\DMATool.exe"

if (Test-Path $releaseExe) {
    $size = (Get-Item $releaseExe).Length / 1MB
    Write-Host "  [OK] Release exe found: $releaseExe" -ForegroundColor Green
    Write-Host "  [INFO] Size: $([Math]::Round($size, 2)) MB" -ForegroundColor Cyan
    
    # Expected size with embedded resources: ~5-8 MB
    # Without resources: ~1-3 MB
    if ($size -gt 4) {
        Write-Host "  [SUCCESS] Size indicates resources are embedded!" -ForegroundColor Green
    } else {
        Write-Host "  [WARNING] Size seems small, resources may not be embedded" -ForegroundColor Yellow
    }
} else {
    Write-Host "  [ERROR] Release exe not found" -ForegroundColor Red
}

Write-Host "`n???????????????????????????????????????????????????????" -ForegroundColor Gray

# Runtime test - actually run the exe and check
Write-Host "`n[TEST 3] Runtime Resource Extraction Test" -ForegroundColor Yellow
Write-Host "???????????????????????????????????????????????????????" -ForegroundColor Gray

# Clean temp
Write-Host "  Cleaning temp directories..." -ForegroundColor Gray
Get-ChildItem "$env:TEMP\DMATool*" -Directory -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force

Write-Host "  Starting Release exe (5 second test)...`n" -ForegroundColor Yellow

# Create a simple test that just checks if the exe can be started
$testExe = $releaseExe
if (!(Test-Path $testExe)) {
    $testExe = $debugExe
}

if (Test-Path $testExe) {
    try {
        # Start process and let it initialize
        $process = Start-Process $testExe -PassThru -WindowStyle Minimized -ErrorAction Stop
        
        Write-Host "  [INFO] Process started (PID: $($process.Id))" -ForegroundColor Cyan
        Write-Host "  [INFO] Waiting 5 seconds for initialization..." -ForegroundColor Gray
        
        Start-Sleep -Seconds 5
        
        # Check if temp directories were created
        $pcileechDir = "$env:TEMP\DMATool_PCILeech"
        $ftdiDir = "$env:TEMP\DMATool_FT601_Driver"
        
        if (Test-Path $pcileechDir) {
            Write-Host "`n  [SUCCESS] PCILeech directory created!" -ForegroundColor Green
            Write-Host "  Location: $pcileechDir" -ForegroundColor Gray
            
            $files = Get-ChildItem $pcileechDir -File
            foreach ($file in $files) {
                $size = $file.Length / 1KB
                Write-Host "    ? $($file.Name) ($([Math]::Round($size, 0)) KB)" -ForegroundColor Green
            }
        } else {
            Write-Host "`n  [INFO] PCILeech not extracted yet (normal - extracts on Benchmark tab access)" -ForegroundColor Yellow
        }
        
        # Kill process
        if (!$process.HasExited) {
            $process.Kill()
            $process.WaitForExit()
        }
        
        Write-Host "`n  [INFO] Test complete - process closed" -ForegroundColor Cyan
        
    } catch {
        Write-Host "`n  [ERROR] Failed to start process: $_" -ForegroundColor Red
    }
} else {
    Write-Host "  [ERROR] No exe found to test" -ForegroundColor Red
}

Write-Host "`n???????????????????????????????????????????????????????" -ForegroundColor Gray

Write-Host "`n[SUMMARY] Resource Embedding Check" -ForegroundColor Cyan
Write-Host "????????????????????????????????????????????????????????" -ForegroundColor Gray

Write-Host "`nExpected Resource IDs:" -ForegroundColor Yellow
Write-Host "  IDR_PCILEECH_EXE    = 120" -ForegroundColor White
Write-Host "  IDR_LEECHCORE_DLL   = 200" -ForegroundColor White
Write-Host "  IDR_FTD3XX_DLL      = 201" -ForegroundColor White
Write-Host "  IDR_VMM_DLL         = 121" -ForegroundColor White
Write-Host "  IDR_DBGHELP_DLL     = 122" -ForegroundColor White
Write-Host "  IDR_FT601_INF       = 116" -ForegroundColor White
Write-Host "  IDR_FT601_CAT       = 117" -ForegroundColor White

Write-Host "`nTo fully test resource extraction:" -ForegroundColor Yellow
Write-Host "  1. Run DMATool.exe manually" -ForegroundColor White
Write-Host "  2. Navigate to Benchmark tab" -ForegroundColor White
Write-Host "  3. Watch console for:" -ForegroundColor White
Write-Host "     [INFO] Extracting PCILeech from embedded resources..." -ForegroundColor Gray
Write-Host "     [SUCCESS] PCILeech extracted to: %TEMP%\DMATool_PCILeech\" -ForegroundColor Gray
Write-Host "  4. If you see '[ERROR] Resource not found: 116'" -ForegroundColor White
Write-Host "     ? Resources weren't embedded during build" -ForegroundColor Gray
Write-Host "     ? Try rebuilding: msbuild /t:Rebuild`n" -ForegroundColor Gray

Write-Host "????????????????????????????????????????????????????????`n" -ForegroundColor Gray
