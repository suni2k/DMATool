# FINAL TESTING GUIDE - Benchmark & FTDI Driver

## ?? IMPORTANT: Run the CORRECT Exe!

**DO NOT run** the old exe from `C:\Users\suni\Downloads\`  
**DO run** the newly compiled exe from the project:

### Correct Paths:
- ? **Debug**: `C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe`
- ? **Release**: `C:\Users\suni\source\repos\DMATool\bin\Release-x64\DMATool.exe`

### Wrong Path (OLD VERSION):
- ? **Downloads**: `C:\Users\suni\Downloads\DMATool.exe` ? This is OLD!

---

## What Was Fixed (All 3 Issues)

### 1. ? FT601DriverInterface Resource Loading
**Problem**: Used `"RCDATA"` string  
**Fix**: Changed to `MAKEINTRESOURCEA(RT_RCDATA)`  
**Result**: FTDI driver installs from embedded resources

### 2. ? BenchmarkInterface Resource Loading
**Problem**: Used `NULL` module handle + wrong API  
**Fix**: Added `GetModuleHandleA(nullptr)` + switched to `FindResourceA`  
**Result**: PCILeech extracts from embedded resources

### 3. ? LeechCoreWrapper DLL Search Path
**Problem**: Didn't check PCILeech temp directory first  
**Fix**: Added `%TEMP%\DMATool_PCILeech\` as first search path  
**Result**: LeechCore finds the DLLs extracted by PCILeech

---

## Test Procedure

### Step 1: Close Any Running DMATool
```powershell
Get-Process -Name "DMATool" -ErrorAction SilentlyContinue | Stop-Process -Force
```

### Step 2: Clean Temp Directories
```powershell
Remove-Item "$env:TEMP\DMATool*" -Recurse -Force -ErrorAction SilentlyContinue
```

### Step 3: Run the NEW Exe

**Option A - Debug (recommended for testing)**:
```powershell
Start-Process "C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe"
```

**Option B - Release**:
```powershell
Start-Process "C:\Users\suni\source\repos\DMATool\bin\Release-x64\DMATool.exe"
```

### Step 4: Test Benchmark Tab

1. **Navigate to Benchmark tab**
2. **Check console output** - should see:
```
[INFO] Extracting PCILeech from embedded resources...
[SUCCESS] PCILeech extracted to: C:\Users\suni\AppData\Local\Temp\DMATool_PCILeech\
```

3. **Verify buttons are enabled** (not greyed out)

4. **Click "Run Quick Speed Test"**

5. **Expected output**:
```
[INFO] Running Quick Speed Test (LeechCore Real-Time)!
[INFO] Enumerating memory ranges...
[INFO] Initializing LeechCore...
[SUCCESS] LeechCore initialized: Loaded from: C:\Users\suni\AppData\Local\Temp\DMATool_PCILeech\leechcore.dll
```

### Step 5: Verify Extraction

After test runs, check temp directory:
```powershell
Get-ChildItem "$env:TEMP\DMATool_PCILeech" | Format-Table Name, Length
```

**Expected files**:
```
pcileech.exe       321,536 bytes
leechcore.dll      150,528 bytes
FTD3XX.dll         514,048 bytes
vmm.dll          2,377,728 bytes
dbghelp.dll      1,558,528 bytes
```

---

## Expected Console Output (Full Test)

```
[INFO] Extracting PCILeech from embedded resources...
[SUCCESS] PCILeech extracted to: C:\Users\suni\AppData\Local\Temp\DMATool_PCILeech\

[INFO] Running Quick Speed Test (LeechCore Real-Time)!

[INFO] Enumerating memory ranges...
[+] Adding memory range: 1000 - 5E000
[+] Adding memory range: 5F000 - A0000
[+] Adding memory range: 100000 - 30B93000
[+] Adding memory range: 30B94000 - 31276000
[+] Adding memory range: 35FFF000 - 36000000
[+] Adding memory range: 100000000 - 10BFC00000

[INFO] Successfully enumerated memory ranges!

[INFO] Initializing LeechCore...
[SUCCESS] LeechCore initialized: Loaded from: C:\Users\suni\AppData\Local\Temp\DMATool_PCILeech\leechcore.dll

[INFO] Running speed test for 10 seconds...

[01/10s]: 6842
[02/10s]: 6845
[03/10s]: 6840
[04/10s]: 6838
...
[10/10s]: 6845

Results:
- Total Reads: 68420
- AVG. Latency: 146 us
- Reads Per Second (RPS): 6842 (AMAZING)

[SUCCESS] Test completed: AMAZING
```

---

## Troubleshooting

### If You See: "Failed to load leechcore.dll"

1. **Check which exe you're running**:
```powershell
Get-Process -Name "DMATool" | Select-Object Path
```
Should show: `C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe`  
NOT: `C:\Users\suni\Downloads\DMATool.exe`

2. **Check if PCILeech was extracted**:
```powershell
Test-Path "$env:TEMP\DMATool_PCILeech\pcileech.exe"
```
Should return: `True`

3. **Check if LeechCore DLLs are there**:
```powershell
Test-Path "$env:TEMP\DMATool_PCILeech\leechcore.dll"
```
Should return: `True`

### If You See: "Resource not found"

This means you're running an OLD build. **Solution**:
1. Close DMATool
2. Run the rebuild command:
```powershell
msbuild C:\Users\suni\source\repos\DMATool\DMATool.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64
```
3. Run the newly built exe

---

## Quick Test Script

Create and run this PowerShell script:

```powershell
# Quick-Test-DMATool.ps1

Write-Host "`n=== DMATool Quick Test ===" -ForegroundColor Cyan

# 1. Kill old process
Write-Host "1. Closing any running DMATool..." -ForegroundColor Yellow
Get-Process -Name "DMATool" -ErrorAction SilentlyContinue | Stop-Process -Force

# 2. Clean temp
Write-Host "2. Cleaning temp directories..." -ForegroundColor Yellow
Remove-Item "$env:TEMP\DMATool*" -Recurse -Force -ErrorAction SilentlyContinue

# 3. Run new exe
Write-Host "3. Starting NEW Debug build..." -ForegroundColor Yellow
$exePath = "C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe"

if (Test-Path $exePath) {
    Write-Host "   Found: $exePath" -ForegroundColor Green
    Start-Process $exePath
    
    Write-Host "`n4. Waiting 3 seconds for extraction..." -ForegroundColor Yellow
    Start-Sleep -Seconds 3
    
    Write-Host "`n5. Checking extracted files..." -ForegroundColor Yellow
    if (Test-Path "$env:TEMP\DMATool_PCILeech") {
        Write-Host "   ? PCILeech directory created!" -ForegroundColor Green
        Get-ChildItem "$env:TEMP\DMATool_PCILeech" | Format-Table Name, @{Name='Size';Expression={"{0:N0} KB" -f ($_.Length/1KB)}}
    } else {
        Write-Host "   ?? PCILeech not extracted yet (go to Benchmark tab)" -ForegroundColor Yellow
    }
    
    Write-Host "`nNow:" -ForegroundColor Cyan
    Write-Host "  1. Go to Benchmark tab in DMATool" -ForegroundColor White
    Write-Host "  2. Click 'Run Quick Speed Test'" -ForegroundColor White
    Write-Host "  3. Watch console for extraction messages`n" -ForegroundColor White
    
} else {
    Write-Host "   ? Exe not found: $exePath" -ForegroundColor Red
    Write-Host "   Run: msbuild DMATool.sln /t:Rebuild /p:Configuration=Debug" -ForegroundColor Yellow
}
```

Save and run:
```powershell
.\Quick-Test-DMATool.ps1
```

---

## Success Criteria

? **PCILeech extracts** to `%TEMP%\DMATool_PCILeech\`  
? **LeechCore loads** from PCILeech temp directory  
? **Benchmark test runs** successfully  
? **Console shows** extraction and initialization messages  
? **No errors** about missing DLLs or resources  

---

**Remember**: Always run from `bin\Debug-x64\` or `bin\Release-x64\`, NOT from Downloads!

**Date**: December 3, 2025  
**Status**: Ready for final testing with correct exe path
