# Quick-Test-DMATool.ps1
# Quick test script for DMATool resource extraction

Write-Host "`n?????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?   DMATool Quick Test                  ?" -ForegroundColor White
Write-Host "?????????????????????????????????????????`n" -ForegroundColor Cyan

# 1. Kill old process
Write-Host "[1] Closing any running DMATool..." -ForegroundColor Yellow
Get-Process -Name "DMATool" -ErrorAction SilentlyContinue | Stop-Process -Force
Write-Host "    ? Done`n" -ForegroundColor Green

# 2. Clean temp
Write-Host "[2] Cleaning temp directories..." -ForegroundColor Yellow
Remove-Item "$env:TEMP\DMATool*" -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "    ? Done`n" -ForegroundColor Green

# 3. Run new exe
Write-Host "[3] Starting NEW Debug build..." -ForegroundColor Yellow
$exePath = "bin\Debug-x64\DMATool.exe"

if (Test-Path $exePath) {
    $fullPath = (Get-Item $exePath).FullName
    Write-Host "    Found: $fullPath" -ForegroundColor Green
    Write-Host "    Size: $([Math]::Round((Get-Item $exePath).Length/1MB, 2)) MB" -ForegroundColor Gray
    Write-Host "    Modified: $((Get-Item $exePath).LastWriteTime)`n" -ForegroundColor Gray
    
    Start-Process $exePath
    
    Write-Host "[4] Waiting 5 seconds for initialization..." -ForegroundColor Yellow
    Start-Sleep -Seconds 5
    
    Write-Host "`n[5] Checking extracted files..." -ForegroundColor Yellow
    
    $pcileechDir = "$env:TEMP\DMATool_PCILeech"
    if (Test-Path $pcileechDir) {
        Write-Host "    ? PCILeech directory created!" -ForegroundColor Green
        Write-Host "`n    Extracted files:" -ForegroundColor Cyan
        Get-ChildItem $pcileechDir | Format-Table Name, @{Name='Size';Expression={"{0:N0} KB" -f ($_.Length/1KB)}} -AutoSize
        
        Write-Host "`n    ? ALL FILES EXTRACTED SUCCESSFULLY!" -ForegroundColor Green
        Write-Host "`n    Next steps:" -ForegroundColor Cyan
        Write-Host "      1. DMATool should be running with console visible" -ForegroundColor White
        Write-Host "      2. Go to Benchmark tab" -ForegroundColor White
        Write-Host "      3. Buttons should be enabled" -ForegroundColor White
        Write-Host "      4. Click 'Run Quick Speed Test'" -ForegroundColor White
        Write-Host "      5. Watch console for test results`n" -ForegroundColor White
        
    } else {
        Write-Host "    ??  PCILeech not extracted yet" -ForegroundColor Yellow
        Write-Host "    This is normal - extraction happens when you:" -ForegroundColor Gray
        Write-Host "      • Open the Benchmark tab (triggers IsPCILeechAvailable)" -ForegroundColor White
        Write-Host "      • Click a test button`n" -ForegroundColor White
        
        Write-Host "    Next:" -ForegroundColor Cyan
        Write-Host "      1. DMATool should be running" -ForegroundColor White
        Write-Host "      2. Go to Benchmark tab (this triggers extraction)" -ForegroundColor White
        Write-Host "      3. Watch console for:" -ForegroundColor White
        Write-Host "         [INFO] Extracting PCILeech from embedded resources..." -ForegroundColor Gray
        Write-Host "         [SUCCESS] PCILeech extracted to: %TEMP%\DMATool_PCILeech\`n" -ForegroundColor Gray
    }
    
} else {
    Write-Host "    ? Exe not found: $exePath" -ForegroundColor Red
    Write-Host "`n    Solution:" -ForegroundColor Yellow
    Write-Host "      Run: msbuild DMATool.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64`n" -ForegroundColor White
}

Write-Host "????????????????????????????????????????`n" -ForegroundColor Cyan
