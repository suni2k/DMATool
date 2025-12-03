# Resource Not Found 116 - Troubleshooting Guide

## Error Message
```
[ERROR] Resource not found: 116
```

## What This Means
- Resource ID 116 = `IDR_FT601_INF` (FTDI driver INF file)
- The `FindResourceA()` call is returning NULL
- This happens in `FT601DriverInterface::ExtractResourceFile()`

## Verification Steps

### 1. Verify Resources Are Embedded
```powershell
# Check exe file size
Get-Item bin\Release-x64\DMATool.exe | Select-Object Length, @{Name='SizeMB';Expression={$_.Length/1MB}}

# Expected: ~19-20 MB (with resources)
# If only 3-5 MB: Resources NOT embedded
```

**Current Status**: ? **19.33 MB** - Resources ARE embedded

### 2. Check Resource IDs Match
```powershell
# In resource.h
#define IDR_FT601_INF                   116
#define IDR_FT601_CAT                   117

# In DMATool.rc
IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"
IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"
```

**Current Status**: ? IDs match, paths correct

### 3. The Actual Problem

The error occurs in `FT601DriverInterface::ExtractResourceFile()`:

```cpp
HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), "RCDATA");
if (!hRes)
{
    std::cerr << "[ERROR] Resource not found: " << resourceId << std::endl;
    return false;  // ? Error happens here
}
```

**Possible Causes**:

1. **Wrong Module Handle** ?
   - Code uses: `HMODULE hModule = GetModuleHandleA(nullptr);`
   - This gets the current exe module
   - **Should be correct** ?

2. **Resource Type Mismatch** ??
   - Code looks for: `"RCDATA"`
   - DMATool.rc declares: `RCDATA`
   - **Should match** ?

3. **Resource Not Compiled** ??
   - Even though file size suggests it's there
   - The .rc file might not have been fully processed
   - **Needs verification**

## Solution: Add Debug Logging

Modify `FT601DriverInterface::ExtractResourceFile()` to diagnose:

```cpp
bool FT601DriverInterface::ExtractResourceFile(int resourceId, const std::string& outputPath)
{
    // Get handle to current module (exe file)
    HMODULE hModule = GetModuleHandleA(nullptr);
    if (!hModule)
    {
        std::cerr << "[ERROR] Failed to get module handle" << std::endl;
        return false;
    }
    
    // ADD THIS DEBUG OUTPUT:
    std::cout << "[DEBUG] Attempting to find resource ID: " << resourceId << std::endl;
    std::cout << "[DEBUG] Module handle: " << hModule << std::endl;
    
    HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), "RCDATA");
    if (!hRes)
    {
        DWORD error = GetLastError();
        std::cerr << "[ERROR] Resource not found: " << resourceId << std::endl;
        std::cerr << "[ERROR] GetLastError: " << error << std::endl;  // ADD THIS
        
        // Try alternative resource type
        std::cout << "[DEBUG] Trying RT_RCDATA constant instead..." << std::endl;
        hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), RT_RCDATA);
        if (!hRes)
        {
            std::cerr << "[ERROR] Still not found with RT_RCDATA" << std::endl;
            return false;
        }
        else
        {
            std::cout << "[SUCCESS] Found with RT_RCDATA!" << std::endl;
        }
    }
    
    // ... rest of function
}
```

## Quick Fix to Try

The issue might be that `"RCDATA"` string doesn't match what the resource compiler used. Try using the `RT_RCDATA` constant instead:

### File: `src/Backend/FT601DriverInterface.cpp`

**Line ~445** (in `ExtractResourceFile`):

**Change FROM**:
```cpp
HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), "RCDATA");
```

**Change TO**:
```cpp
HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(resourceId), RT_RCDATA);
```

## Same Fix Needed in BenchmarkInterface

### File: `src/Backend/BenchmarkInterface.cpp`

**Line ~700** (in `ExtractResourceToFile`):

**Change FROM**:
```cpp
HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
```

**Already correct!** ? It's using `RT_RCDATA` constant

## The Difference

- `FT601DriverInterface` uses: `"RCDATA"` (string literal) ?
- `BenchmarkInterface` uses: `RT_RCDATA` (Windows constant) ?

**This is likely why FT601 fails but PCILeech works!**

## Files to Modify

1. `src/Backend/FT601DriverInterface.cpp` - Line ~445
   - Change `"RCDATA"` to `RT_RCDATA`

## After Fix

1. Rebuild: `msbuild /t:Rebuild /p:Configuration=Release`
2. Test FTDI driver installation
3. Should see: `[INFO] FTDI driver files extracted to: ...`
4. No more `[ERROR] Resource not found: 116`

---

**Status**: Root cause identified - string vs constant mismatch  
**Fix**: Use `RT_RCDATA` constant instead of `"RCDATA"` string  
**Impact**: FT601 driver installation will work
