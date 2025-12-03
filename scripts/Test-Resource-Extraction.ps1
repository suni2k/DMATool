# Test-Resource-Extraction.ps1
# Test that resources are extracted correctly from the exe

$ErrorActionPreference = 'Stop'

Write-Host "`n??????????????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?  Testing Resource Extraction                       ?" -ForegroundColor White
Write-Host "??????????????????????????????????????????????????????`n" -ForegroundColor Cyan

# Clean up any existing temp files
Write-Host "[CLEANUP] Removing old temp files..." -ForegroundColor Yellow
Get-ChildItem "$env:TEMP\DMATool*" -Directory -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force
Write-Host "  [OK] Temp directories cleared`n" -ForegroundColor Green

# Test Debug build
Write-Host "[TEST 1] Debug Build - Resource Extraction" -ForegroundColor Cyan
Write-Host "???????????????????????????????????????????????????????" -ForegroundColor Gray

$debugExe = "bin\Debug-x64\DMATool.exe"

if (Test-Path $debugExe) {
    Write-Host "  [OK] Debug exe found: $debugExe" -ForegroundColor Green
    
    Write-Host "`n  Starting Debug exe (will close automatically in 5 seconds)..." -ForegroundColor Yellow
    Write-Host "  This will trigger resource extraction...`n" -ForegroundColor Gray
    
    # Start the exe
    $process = Start-Process $debugExe -PassThru
    
    # Wait a few seconds for resources to extract
    Start-Sleep -Seconds 5
    
    # Close the process
    if (!$process.HasExited) {
        $process.Kill()
        $process.WaitForExit()
    }
    
    Write-Host "`n  Checking extracted resources..." -ForegroundColor Yellow
    
    # Check PCILeech extraction
    $pcileechDir = "$env:TEMP\DMATool_PCILeech"
    if (Test-Path $pcileechDir) {
        Write-Host "  [SUCCESS] PCILeech directory created" -ForegroundColor Green
        
        $expectedFiles = @("pcileech.exe", "leechcore.dll", "FTD3XX.dll", "vmm.dll", "dbghelp.dll")
        $allFound = $true
        
        foreach ($file in $expectedFiles) {
            $filePath = Join-Path $pcileechDir $file
            if (Test-Path $filePath) {
                $size = (Get-Item $filePath).Length / 1KB
                Write-Host "    [OK] $file ($([Math]::Round($size, 0)) KB)" -ForegroundColor Green
            } else {
                Write-Host "    [MISSING] $file" -ForegroundColor Red
                $allFound = $false
            }
        }
        
        if ($allFound) {
            Write-Host "`n  [SUCCESS] All PCILeech files extracted!" -ForegroundColor Green
        } else {
            Write-Host "`n  [WARNING] Some PCILeech files missing!" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  [ERROR] PCILeech directory not created!" -ForegroundColor Red
        Write-Host "  Expected: $pcileechDir" -ForegroundColor Gray
    }
    
} else {
    Write-Host "  [ERROR] Debug exe not found: $debugExe" -ForegroundColor Red
    Write-Host "  Please build Debug configuration first.`n" -ForegroundColor Yellow
}

Write-Host "`n???????????????????????????????????????????????????????" -ForegroundColor Gray

# Test Release build
Write-Host "`n[TEST 2] Release Build - Resource Extraction" -ForegroundColor Cyan
Write-Host "???????????????????????????????????????????????????????" -ForegroundColor Gray

$releaseExe = "bin\Release-x64\DMATool.exe"

if (Test-Path $releaseExe) {
    Write-Host "  [OK] Release exe found: $releaseExe" -ForegroundColor Green
    
    # Clear temp again
    Get-ChildItem "$env:TEMP\DMATool*" -Directory -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force
    
    Write-Host "`n  Starting Release exe (will close automatically in 5 seconds)..." -ForegroundColor Yellow
    Write-Host "  This will trigger resource extraction...`n" -ForegroundColor Gray
    
    # Start the exe
    $process = Start-Process $releaseExe -PassThru -WindowStyle Hidden
    
    # Wait a few seconds for resources to extract
    Start-Sleep -Seconds 5
    
    # Close the process
    if (!$process.HasExited) {
        $process.Kill()
        $process.WaitForExit()
    }
    
    Write-Host "`n  Checking extracted resources..." -ForegroundColor Yellow
    
    # Check PCILeech extraction
    $pcileechDir = "$env:TEMP\DMATool_PCILeech"
    if (Test-Path $pcileechDir) {
        Write-Host "  [SUCCESS] PCILeech directory created" -ForegroundColor Green
        
        $expectedFiles = @("pcileech.exe", "leechcore.dll", "FTD3XX.dll", "vmm.dll", "dbghelp.dll")
        $allFound = $true
        
        foreach ($file in $expectedFiles) {
            $filePath = Join-Path $pcileechDir $file
            if (Test-Path $filePath) {
                $size = (Get-Item $filePath).Length / 1KB
                Write-Host "    [OK] $file ($([Math]::Round($size, 0)) KB)" -ForegroundColor Green
            } else {
                Write-Host "    [MISSING] $file" -ForegroundColor Red
                $allFound = $false
            }
        }
        
        if ($allFound) {
            Write-Host "`n  [SUCCESS] All PCILeech files extracted!" -ForegroundColor Green
        } else {
            Write-Host "`n  [WARNING] Some PCILeech files missing!" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  [ERROR] PCILeech directory not created!" -ForegroundColor Red
        Write-Host "  Expected: $pcileechDir" -ForegroundColor Gray
    }
    
} else {
    Write-Host "  [ERROR] Release exe not found: $releaseExe" -ForegroundColor Red
    Write-Host "  Please build Release configuration first.`n" -ForegroundColor Yellow
}

Write-Host "`n???????????????????????????????????????????????????????" -ForegroundColor Gray

Write-Host "`n[SUMMARY]" -ForegroundColor Cyan
Write-Host "????????????????????????????????????????????????????????" -ForegroundColor Gray
Write-Host "Resources are extracted on first use when:" -ForegroundColor Yellow
Write-Host "  • Benchmark tab is accessed (PCILeech extraction)" -ForegroundColor White
Write-Host "  • FTDI driver install is clicked (driver extraction)" -ForegroundColor White
Write-Host "`nTo fully test:" -ForegroundColor Yellow
Write-Host "  1. Run DMATool manually" -ForegroundColor White
Write-Host "  2. Go to Benchmark tab" -ForegroundColor White
Write-Host "  3. Check if buttons are enabled" -ForegroundColor White
Write-Host "  4. Try running a test`n" -ForegroundColor White
Write-Host "????????????????????????????????????????????????????????`n" -ForegroundColor Gray
