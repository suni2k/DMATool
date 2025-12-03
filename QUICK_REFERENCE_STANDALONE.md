# Quick Reference - Standalone Exe Changes

## ? What Was Fixed

### 1. Benchmark Tab (Temp Directory Only)
**Before**: Had fallbacks to `C:\Tools\PCILeech\`, `vendor\leechcore\`, etc.  
**After**: Only uses `%TEMP%\DMATool_PCILeech\`

**Files Changed**:
- `BenchmarkInterface.cpp` - Updated error message
- `LeechCoreWrapper.cpp` - Removed all external path fallbacks

### 2. Driver Version Check
**Before**: Only showed installed/not installed  
**After**: Shows three states with version checking

**Version Logic**:
- No version ? "Driver Needed" (not installed)
- Version < 1.4.0.1 ? "Installed (out of date)" 
- Version >= 1.4.0.1 ? "Installed" (correct)

**Files Changed**:
- `FT601DriverInterface.h` - Added `CompareVersion()`
- `FT601DriverInterface.cpp` - New version logic, removed external file fallbacks
- `DataPortTab.cpp` - Updated status display and console messages

## ?? Test Scenarios

### Driver Status
1. **No driver**: Should show "Driver Needed" + log message about not installed
2. **Old driver (1.3.x)**: Should show "Installed (out of date)" + suggestion to update
3. **Correct driver (1.4.0.1+)**: Should show "Installed" in green

### Benchmark Tab
1. Open tab ? PCILeech extracts to `%TEMP%\DMATool_PCILeech\`
2. LeechCore loads from temp directory
3. No errors about missing files

## ?? Distribution
**Just ship the .exe file** - no other files needed!
- All DLLs embedded
- All drivers embedded  
- Everything extracts to temp on first use

---
**Builds**: Both Debug and Release successful ?  
**Documentation**: `STANDALONE_EXE_COMPLETE.md`
