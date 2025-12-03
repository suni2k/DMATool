# Benchmark Tab Issues - Debug & Release Builds

## Summary

Two separate issues identified with the Benchmark tab:

1. **Debug Build**: Test buttons greyed out
2. **Release Build**: Driver check/install/uninstall fail

---

## Issue 1: Debug Build - Buttons Greyed Out ?

### Problem
- "Run Test" buttons are disabled (greyed out)
- All test types unavailable

### Root Cause
```cpp
// From DataPortTab.cpp line 595:
ImGui::BeginDisabled(!Backend::BenchmarkInterface::IsPCILeechAvailable() || s_IsTestRunning);
```

The buttons are disabled because `IsPCILeechAvailable()` returns `false`.

### Why IsPCILeechAvailable() Returns False
```cpp
// From BenchmarkInterface.cpp:
std::string BenchmarkInterface::GetPCILeechPath()
{
    std::vector<std::string> searchPaths = {
        "C:\\Tools\\PCILeech\\pcileech.exe",     // ? FOUND
        "pcileech.exe",                          // ? NOT FOUND
        "tools\\pcileech\\pcileech.exe",         // ? NOT FOUND
    };
    
    for (const auto& path : searchPaths)
    {
        if (std::filesystem::exists(path))
            return path;
    }
    return "";  // Returns empty if not found
}
```

**Current Status**:
- ? `C:\Tools\PCILeech\pcileech.exe` EXISTS
- ? Buttons SHOULD work in Debug build
- ?? But they're greyed out anyway!

### Diagnosis Needed
1. Check if Debug build is actually finding pcileech.exe
2. Verify `IsPCILeechAvailable()` is being called correctly
3. Check if there's a different reason buttons are disabled

### Quick Test
```powershell
# Verify pcileech exists
Test-Path "C:\Tools\PCILeech\pcileech.exe"
# Should return: True

# Check if benchmark is actually finding it
# (Add debug logging to BenchmarkInterface::IsPCILeechAvailable())
```

---

## Issue 2: Release Build - Driver Operations Fail ?

### Problem
- Check Driver, Install Driver, Uninstall Driver all fail
- Error: Looking for driver files in local directory instead of embedded resources

### Root Cause
FT601DriverInterface is looking for driver files in the file system:
```cpp
// Likely in FT601DriverInterface.cpp:
std::string driverPath = "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver";
```

**But in Release build**, these files might not exist because:
- Release exe is standalone
- Driver files should be embedded as resources
- Should extract to temp directory (like OpenOCD files)

### Current Status
? **Good News**: FT601 driver files ARE embedded in resources:
- `resource.h` has `IDR_FT601_INF` and `IDR_FT601_CAT`
- `DMATool.rc` has FT601 driver entries

? **Bad News**: Code isn't extracting them properly in Release build

### Expected Behavior
```cpp
// What SHOULD happen (like OpenOCD):
1. Check if temp directory exists: %TEMP%\DMATool_FT601_Driver
2. If not, extract driver files from resources
3. Use extracted files for installation
```

### Actual Behavior
```cpp
// What's ACTUALLY happening:
1. Looks for dmafiles\Winusb_D3XX_Release_1.4.0.1\WU_FTD3XX_Driver
2. In Release build, this path doesn't exist (exe is standalone)
3. Operations fail
```

### Solution
Modify `FT601DriverInterface.cpp` to:
1. **Always extract from resources** (not check file system)
2. Extract to `%TEMP%\DMATool_FT601_Driver`
3. Use temp directory for driver operations

---

## Recommended Fixes

### Fix 1: Debug Build (Buttons Greyed Out)

**Option A**: Verify pcileech is actually being found
```cpp
// Add logging to BenchmarkInterface.cpp:
bool BenchmarkInterface::IsPCILeechAvailable()
{
    std::string path = GetPCILeechPath();
    std::cout << "[DEBUG] PCILeech path: " << (path.empty() ? "NOT FOUND" : path) << std::endl;
    return !path.empty();
}
```

**Option B**: Check for other reasons buttons are disabled
```cpp
// In DataPortTab.cpp, check both conditions:
std::cout << "[DEBUG] PCILeech available: " << Backend::BenchmarkInterface::IsPCILeechAvailable() << std::endl;
std::cout << "[DEBUG] Test running: " << s_IsTestRunning << std::endl;
```

### Fix 2: Release Build (Driver Operations)

**Modify `FT601DriverInterface.cpp`** to extract from resources:

```cpp
bool FT601DriverInterface::InstallDriver(std::function<void(const std::string&)> progressCallback)
{
    // 1. Create temp directory
    std::string tempDir = GetTempDirectory();  // %TEMP%\DMATool_FT601_Driver
    
    // 2. Extract driver files from resources
    ExtractResourceToFile(IDR_FT601_INF, tempDir + "\\FTD3XXWU.Inf");
    ExtractResourceToFile(IDR_FT601_CAT, tempDir + "\\FTD3XXWU.cat");
    
    // 3. Use temp directory for installation
    std::string command = "pnputil /add-driver \"" + tempDir + "\\FTD3XXWU.Inf\" /install";
    // ...
}
```

---

## Testing Checklist

### Debug Build
- [ ] Run DMATool in Debug mode
- [ ] Navigate to Benchmark tab
- [ ] Check console for "[DEBUG] PCILeech path: ..." message
- [ ] Verify buttons are enabled (not greyed out)
- [ ] Try running a Quick Speed Test

### Release Build
- [ ] Build in Release configuration
- [ ] Run DMATool.exe
- [ ] Navigate to Benchmark tab
- [ ] Click "Check Driver Status"
- [ ] Verify it extracts resources to temp (not looks in dmafiles/)
- [ ] Try "Install FTDI Driver"
- [ ] Verify UAC prompt appears
- [ ] Check Device Manager for installed driver

---

## Quick Diagnostic Commands

```powershell
# Check if pcileech exists
Test-Path "C:\Tools\PCILeech\pcileech.exe"

# Check temp directory after running app
Get-ChildItem $env:TEMP\DMATool* -Recurse | Select-Object FullName

# Verify FT601 resources are embedded
Select-String -Path "DMATool.rc" -Pattern "FTD3XXWU"

# Check resource.h
Select-String -Path "src\resource.h" -Pattern "IDR_FT601"
```

---

## Files to Check/Modify

### For Debug Issue (Buttons Greyed Out)
1. `src/Backend/BenchmarkInterface.cpp` - Add debug logging
2. `src/UI/Tabs/DataPortTab.cpp` - Check disable conditions

### For Release Issue (Driver Operations Fail)
1. `src/Backend/FT601DriverInterface.cpp` - Modify to extract from resources
2. `src/Backend/FT601DriverInterface.h` - Add ExtractResourceToFile helper
3. Verify `DMATool.rc` has correct FT601 entries
4. Verify `src/resource.h` has IDR_FT601_INF and IDR_FT601_CAT

---

## Next Steps

1. **Run diagnostic script**: `.\scripts\Fix-Benchmark-Resource-Issues.ps1`
2. **Add debug logging** to identify why buttons are greyed out
3. **Modify FT601DriverInterface** to use embedded resources
4. **Test both Debug and Release** builds
5. **Report results**

---

**Status**: Issues identified, fixes proposed  
**Priority**: High (blocks benchmark and driver functionality)  
**Complexity**: Medium (requires code changes to resource extraction)
