# Resource Extraction Fix - Complete Guide

## Issues Found

### 1. FTDI Driver Path in DMATool.rc ? FIXED
**Problem**: RC file pointed to `tools\ftdi601\drivers\` which doesn't exist  
**Fix**: Updated to `dmafiles\Winusb_D3XX_Release_1.4.0.1\WU_FTD3XX_Driver\`  
**Status**: ? Fixed and rebuilt

### 2. Resource Extraction Timing
**Behavior**: Resources extract **only when needed**, not on startup  
**PCILeech**: Extracts when `IsPCILeechAvailable()` is called  
**FTDI Drivers**: Extract when `InstallDriver()` is called  
**Status**: ? **This is correct behavior** - lazy loading saves startup time

## How It Works

### PCILeech Extraction
```cpp
// Called when checking if benchmark tests can run
bool IsPCILeechAvailable()
{
    return !GetPCILeechPath().empty();  // This triggers extraction
}

std::string GetPCILeechPath()
{
    // Check if already extracted
    if (std::filesystem::exists(pcileechExe))
        return pcileechExe;
    
    // Extract from resources to %TEMP%\DMATool_PCILeech\
    ExtractResourceToFile(IDR_PCILEECH_EXE, pcileechExe);
    ExtractResourceToFile(IDR_LEECHCORE_DLL, ...);
    // ... etc
}
```

**Extraction Location**: `%TEMP%\DMATool_PCILeech\`  
**Files Extracted**:
- pcileech.exe (314 KB)
- leechcore.dll (147 KB)
- FTD3XX.dll (502 KB)
- vmm.dll (2,322 KB)
- dbghelp.dll (1,522 KB)

### FTDI Driver Extraction
```cpp
// Called when user clicks "Install FTDI Driver"
bool InstallDriver(ProgressCallback progressCallback)
{
    std::string driverPath;
    
    // Try embedded resources first
    if (ExtractDriverFiles(driverPath))
    {
        // Use extracted files
    }
    else
    {
        // Fallback to external files (shouldn't happen in standalone mode)
    }
}
```

**Extraction Location**: `%TEMP%\DMATool_FT601_Driver\`  
**Files Extracted**:
- FTD3XXWU.Inf (23 KB)
- FTD3XXWU.cat (11 KB)

## Testing Procedures

### Test 1: Benchmark Tab (PCILeech)
1. Run `bin\Debug-x64\DMATool.exe` or `bin\Release-x64\DMATool.exe`
2. Navigate to **Benchmark** tab
3. **Expected**:
   - Buttons should be **enabled** (not greyed out)
   - Console shows: `[INFO] Extracting PCILeech from embedded resources...`
   - Console shows: `[SUCCESS] PCILeech extracted to: C:\Users\...\Temp\DMATool_PCILeech\`
4. Click any "Run Test" button
5. **Expected**:
   - Test executes successfully
   - PCILeech commands run from temp directory

### Test 2: FTDI Driver Installation
1. Run `bin\Release-x64\DMATool.exe`
2. Navigate to **Data Port** tab
3. Click **"Check Driver Status"**
4. Click **"Install FTDI Driver"**
5. **Expected**:
   - Progress popup shows: "Extracting driver files..."
   - Console shows: `[INFO] FTDI driver files extracted to: C:\Users\...\Temp\DMATool_FT601_Driver\`
   - Driver installs successfully
   - NO errors about missing files

### Test 3: Standalone Operation
1. Copy **only** `bin\Release-x64\DMATool.exe` to a USB drive
2. Plug USB drive into a different computer
3. Run `DMATool.exe` (no other files needed)
4. **Expected**:
   - All features work
   - Resources extract automatically
   - No external file dependencies

## Verification Commands

### Check Extracted Resources
```powershell
# After running DMATool and accessing Benchmark tab
Get-ChildItem $env:TEMP\DMATool* -Recurse | Select-Object FullName, @{Name='Size';Expression={"{0:N0} KB" -f ($_.Length/1KB)}}
```

**Expected Output**:
```
DMATool_PCILeech\pcileech.exe         314 KB
DMATool_PCILeech\leechcore.dll        147 KB
DMATool_PCILeech\FTD3XX.dll           502 KB
DMATool_PCILeech\vmm.dll             2322 KB
DMATool_PCILeech\dbghelp.dll         1522 KB
```

### Check Resource Embedding
```powershell
# Verify resources are in RC file
Select-String -Path "DMATool.rc" -Pattern "IDR_PCILEECH_EXE|IDR_FT601_INF"
```

**Expected Output**:
```
IDR_PCILEECH_EXE RCDATA "dmafiles\pcileech\pcileech.exe"
IDR_FT601_INF RCDATA "dmafiles\Winusb_D3XX_Release_1.4.0.1\WU_FTD3XX_Driver\FTD3XXWU.Inf"
```

## Troubleshooting

### Issue: Benchmark buttons still greyed out

**Possible Causes**:
1. Resource extraction failed
2. PCILeech path returned empty
3. Another condition disabled buttons

**Debug Steps**:
```cpp
// Add to BenchmarkInterface::IsPCILeechAvailable()
std::string path = GetPCILeechPath();
std::cout << "[DEBUG] PCILeech path: " << (path.empty() ? "NOT FOUND" : path) << std::endl;
std::cout << "[DEBUG] PCILeech available: " << !path.empty() << std::endl;
return !path.empty();
```

**Check Console Output**:
- Should see: `[INFO] Extracting PCILeech from embedded resources...`
- Should see: `[SUCCESS] PCILeech extracted to: ...`
- Should see: `[DEBUG] PCILeech path: C:\Users\...\Temp\DMATool_PCILeech\pcileech.exe`

### Issue: FTDI driver installation fails

**Possible Causes**:
1. Resource extraction failed
2. Paths incorrect in DMATool.rc
3. Files not embedded in exe

**Debug Steps**:
1. Check console output for extraction messages
2. Verify files exist: `dir %TEMP%\DMATool_FT601_Driver`
3. Check RC file paths: `.\scripts\Fix-Resource-Paths.ps1`

### Issue: "Resource not found" errors

**Causes**:
1. Resource ID mismatch between resource.h and DMATool.rc
2. Resource not embedded (missing from RC file)
3. Build didn't include resources (need rebuild)

**Fix**:
1. Verify resource IDs match in both files
2. Run `.\scripts\Fix-Resource-Paths.ps1`
3. Clean and rebuild: `msbuild /t:Clean` then `msbuild /t:Rebuild`

## Build Status

? **Debug Build**: SUCCESS  
? **Release Build**: SUCCESS  
? **Resource Paths**: FIXED  
? **All Files Embedded**: VERIFIED  

## What's Changed

### Files Modified
1. ? `DMATool.rc` - Fixed FTDI driver paths
2. ? `src/Backend/BenchmarkInterface.cpp` - Extract PCILeech from resources
3. ? `src/Backend/FT601DriverInterface.cpp` - Already correct

### Scripts Created
1. `scripts/Fix-Resource-Paths.ps1` - Fix RC file paths
2. `scripts/Test-Resource-Extraction.ps1` - Test extraction

## Manual Testing Required

**You need to manually test**:

1. **Launch DMATool** (Debug or Release)
2. **Go to Benchmark tab**
3. **Verify buttons are enabled**
4. **Try running a test**
5. **Check console output** for extraction messages

If you see any issues, check the console output and share what you see!

## Expected Console Output on First Run

### When Opening Benchmark Tab:
```
[INFO] Extracting PCILeech from embedded resources...
[SUCCESS] PCILeech extracted to: C:\Users\suni\AppData\Local\Temp\DMATool_PCILeech\
```

### When Installing FTDI Driver:
```
[INFO] Installing FTDI driver...
[INFO] Extracting driver files...
[INFO] Using embedded driver files from temp
[INFO] FTDI driver files extracted to: C:\Users\suni\AppData\Local\Temp\DMATool_FT601_Driver
[INFO] Installing driver to Windows driver store...
```

---

**Status**: Ready for manual testing  
**Action Required**: Run DMATool and test Benchmark tab  
**Last Updated**: December 3, 2025
