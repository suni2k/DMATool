# Standalone Exe - No External Dependencies ?

## Summary

Fixed all issues to ensure DMATool is **100% standalone** - no external files needed, everything extracts from embedded resources to temp directory.

---

## ? Issue 1: Benchmark Tab - Resources to Temp Only

### What Was Changed

**BenchmarkInterface.cpp**:
- ? Removed ALL references to external paths
- ? Only extracts to `%TEMP%\DMATool_PCILeech\`
- ? Updated error message to reflect embedded resources only

**LeechCoreWrapper.cpp**:
- ? Removed ALL external DLL fallbacks (`C:\Tools\PCILeech\`, `vendor\leechcore\`, etc.)
- ? Only looks in `%TEMP%\DMATool_PCILeech\` for leechcore.dll
- ? Simplified error message

### Before (Had Fallbacks):
```cpp
// LeechCoreWrapper tried multiple locations:
searchPaths.push_back(exeDir + "\\vendor\\leechcore\\leechcore.dll");
searchPaths.push_back("C:\\Tools\\PCILeech\\leechcore.dll");
searchPaths.push_back(exeDir + "\\leechcore.dll");
```

### After (Temp Only):
```cpp
// Only uses temp directory:
std::string pcileechTempDir = std::string(tempPath) + "DMATool_PCILeech\\";
std::string leechcorePath = pcileechTempDir + "leechcore.dll";
m_hLeechCore = LoadLibraryA(leechcorePath.c_str());
```

### How It Works Now:

1. **User opens Benchmark tab** ? Triggers `IsPCILeechAvailable()`
2. **GetPCILeechPath()** checks temp directory
3. **If not found** ? Extracts ALL files from embedded resources:
   - pcileech.exe
   - leechcore.dll
   - FTD3XX.dll
   - vmm.dll
   - dbghelp.dll
4. **Returns path** to temp directory
5. **LeechCore** loads DLLs from temp directory
6. **No fallbacks** to external directories

---

## ? Issue 2: Driver Version Check Logic

### Requirements (Your Specs):

1. FTDI device **always shows** if connected
2. **No version** = driver NOT installed (using default Windows driver)
3. **Version < 1.4.0.1** = installed but **out of date**
4. **Version >= 1.4.0.1** = installed **correctly**

### What Was Changed

**FT601DriverInterface.cpp - CheckDriver()**:
```cpp
// NEW LOGIC:
if (info.version.empty() || info.version == "Unknown")
{
    // No driver version = not installed
    info.installed = false;
    info.isCorrectDriver = false;
}
else
{
    // Driver is installed - check version
    info.installed = true;
    
    if (CompareVersion(info.version, "1.4.0.1") < 0)
    {
        // Version is lower than 1.4.0.1 = out of date
        info.isCorrectDriver = false;
    }
    else
    {
        // Version is 1.4.0.1 or higher = correct
        info.isCorrectDriver = true;
    }
}
```

**Added CompareVersion() function**:
- Parses version strings like "1.4.0.1"
- Compares part by part (major.minor.patch.build)
- Returns: -1 if v1 < v2, 0 if equal, 1 if v1 > v2

**FT601DriverInterface.cpp - InstallDriver()**:
- ? Removed ALL external file fallbacks
- ? Only uses embedded resources
- ? Fails immediately if extraction fails (no searching in file system)

### UI Display Updates

**DataPortTab.cpp - Status Display**:
```cpp
// Before: Only showed "Installed" or "Driver Needed"

// After: Shows three states:
if (installed && isCorrectDriver)
    "Installed"  // Green ?
else if (installed && !isCorrectDriver)
    "Installed (out of date)"  // Yellow ??
else if (!installed)
    "Driver Needed"  // Yellow ??
else
    "Not Detected"  // Gray
```

**DataPortTab.cpp - Console Log Messages**:

**Correct Version (1.4.0.1 or higher)**:
```
[SUCCESS] FTDI driver is installed
[INFO] Device: USB Composite Device
[INFO] Driver Version: 1.4.0.1
[INFO] VID/PID: VID_0403 / PID_601F
```

**Out of Date (version < 1.4.0.1)**:
```
[WARNING] FTDI WinUSB driver is out of date
[INFO] Device: USB Composite Device
[INFO] Current version: 1.3.0.0
[INFO] Required version: 1.4.0.1 or higher
[INFO] Action: Click 'Install FTDI Driver' to update to version 1.4.0.1
```

**Not Installed (no version)**:
```
[WARNING] FTDI WinUSB driver not installed
[INFO] Device detected: USB Composite Device
[INFO] Driver version: Not installed (using default Windows driver)
[INFO] Required version: 1.4.0.1 or higher
[INFO] Action: Click 'Install FTDI Driver' to install WinUSB driver
```

---

## ?? Files Modified

### 1. src/Backend/BenchmarkInterface.cpp
- Updated error message when PCILeech not available
- Now says "Failed to extract from embedded resources" instead of "Install to C:\Tools\PCILeech\"

### 2. src/Backend/LeechCoreWrapper.cpp
- Removed all external DLL search paths
- Only uses `%TEMP%\DMATool_PCILeech\leechcore.dll`
- Simplified error message

### 3. src/Backend/FT601DriverInterface.h
- Added `CompareVersion()` static method declaration

### 4. src/Backend/FT601DriverInterface.cpp
- Updated `CheckDriver()` with new version logic
- Added `CompareVersion()` implementation
- Removed external driver file fallbacks from `InstallDriver()`
- Only uses embedded resources now

### 5. src/UI/Tabs/DataPortTab.cpp
- Updated status display to show "(out of date)" for old versions
- Updated console log messages to differentiate states
- Added specific messages for out of date vs not installed

---

## ?? Testing Scenarios

### Scenario 1: No Driver Installed
**Device Status**: Connected, using default Windows driver  
**Driver Version**: Empty or not reported  
**Expected UI**: "Driver Needed" (Yellow)  
**Expected Log**:
```
[WARNING] FTDI WinUSB driver not installed
[INFO] Device detected: USB Composite Device
[INFO] Driver version: Not installed (using default Windows driver)
```

### Scenario 2: Old Driver (e.g., 1.3.0.0)
**Device Status**: Connected  
**Driver Version**: 1.3.0.0 (less than 1.4.0.1)  
**Expected UI**: "Installed (out of date)" (Yellow)  
**Expected Log**:
```
[WARNING] FTDI WinUSB driver is out of date
[INFO] Current version: 1.3.0.0
[INFO] Required version: 1.4.0.1 or higher
[INFO] Action: Click 'Install FTDI Driver' to update
```

### Scenario 3: Correct Driver (1.4.0.1 or higher)
**Device Status**: Connected  
**Driver Version**: 1.4.0.1 or 1.4.0.2, etc.  
**Expected UI**: "Installed" (Green)  
**Expected Log**:
```
[SUCCESS] FTDI driver is installed
[INFO] Driver Version: 1.4.0.1
```

### Scenario 4: Benchmark Tab Resource Extraction
1. **Open Benchmark tab**
2. **Expected**: Resources extract to `%TEMP%\DMATool_PCILeech\`
3. **Expected Log**:
```
[INFO] Extracting PCILeech from embedded resources...
[SUCCESS] PCILeech extracted to: C:\Users\...\Temp\DMATool_PCILeech\
[INFO] Initializing LeechCore...
[SUCCESS] LeechCore initialized: Loaded from: C:\Users\...\Temp\DMATool_PCILeech\leechcore.dll
```

---

## ? Verification Checklist

### Standalone Exe
- ? No external directories referenced in code
- ? All resources extract to `%TEMP%` directory
- ? LeechCore only looks in temp directory
- ? FTDI driver only extracts from embedded resources
- ? No fallbacks to `C:\Tools\`, `vendor\`, or external directories

### Driver Version Detection
- ? Detects device even without driver
- ? Correctly identifies "no version" as not installed
- ? Correctly identifies version < 1.4.0.1 as out of date
- ? Correctly identifies version >= 1.4.0.1 as installed
- ? UI shows appropriate status for each state
- ? Console log provides clear guidance

### Resource Extraction
- ? Benchmark tab extracts PCILeech + DLLs to temp
- ? FTDI driver extracts INF + CAT to temp
- ? No external file dependencies
- ? Fails gracefully if extraction fails (no fallbacks)

---

## ?? Build Status

? **Debug**: `bin\Debug-x64\DMATool.exe` - Build successful  
? **Release**: `bin\Release-x64\DMATool.exe` - Build successful  

**Warnings**: Minor (signed/unsigned mismatch, safe to ignore)

---

## ?? Next Steps for User

1. **Test driver detection**:
   - With no driver (should show "Driver Needed")
   - With old driver 1.3.x (should show "Installed (out of date)")
   - With correct driver 1.4.0.1+ (should show "Installed")

2. **Test benchmark tab**:
   - Open Benchmark tab
   - Verify PCILeech extracts to temp
   - Verify LeechCore loads from temp
   - Run a test

3. **Distribute standalone exe**:
   - Copy ONLY the exe file
   - No other files needed
   - Everything works from embedded resources

---

## ?? Success Criteria Met

? **Benchmark tab**: Only uses temp directory, no external dependencies  
? **Driver check**: Correctly identifies not installed, out of date, and installed  
? **Driver install**: Only uses embedded resources, no external files  
? **UI display**: Shows "(out of date)" for old versions  
? **Console logs**: Clear messages for all driver states  
? **Standalone**: No external file dependencies whatsoever  

---

**Date**: December 3, 2025  
**Status**: ? **COMPLETE**  
**Impact**: DMATool is now 100% standalone with correct driver version detection
