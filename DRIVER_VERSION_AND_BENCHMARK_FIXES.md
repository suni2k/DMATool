# Driver Version Detection and Benchmark Resource Extraction Fixes

## Date: 2025-12-03

## Latest Update: Fixed spam logging and driver version detection

### Issues Fixed (Updated)

### 1. **FT601 Driver Version Detection** (UPDATED)
**Problem**: Driver version check was reading the **OS driver version** (winusb.sys version 10.0.x) instead of the **FTDI provider version**.

**Root Cause**: 
- Initial fix tried to parse INF file manually with regex, but pattern might not match all INF formats
- Need to use proper Windows device property for driver version

**Fix (Updated)**:
- Modified `FT601DriverInterface::CheckDriver()` to:
  - Query `DEVPKEY_Device_DriverProvider` to determine if using FTDI or Microsoft default driver
  - Use `DEVPKEY_Device_DriverVersion` property directly (this should give provider version, not OS version)
  - Remove manual INF file parsing (unreliable)
  - Remove verbose debug output that was printing PowerShell command

**PowerShell Changes**:
```powershell
# Before: Manual INF parsing
$driverInf = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverInfPath' -ErrorAction SilentlyContinue).Data;
if ($driverInf) {
    $infFile = Join-Path $env:SystemRoot ('INF\\' + $driverInf);
    if (Test-Path $infFile) {
        $infContent = Get-Content $infFile -Raw;
        if ($infContent -match 'DriverVer\\s*=\\s*[^,]+,\\s*([\\d.]+)') {
            $props['DriverVersion'] = $Matches[1];
        }
    }
}

# After: Direct property query
$driverVer = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverVersion' -ErrorAction SilentlyContinue).Data;
if ($driverVer) { $props['DriverVersion'] = $driverVer; } else { $props['DriverVersion'] = ''; }
```

**Testing Script**: Created `scripts/Test-FT601-DriverVersion.ps1` to verify what PowerShell returns for each property.

**Files Modified**:
- `src/Backend/FT601DriverInterface.cpp` - `CheckDriver()` - Simplified to use DEVPKEY_Device_DriverVersion
- `scripts/Test-FT601-DriverVersion.ps1` - NEW - Test script to verify driver version detection

---

### 2. **Benchmark Resource Spam Logging** (FIXED)
**Problem**: Console was spammed with:
```
[INFO] PCILeech resources already extracted to temp
[INFO] PCILeech resources already extracted to temp
[INFO] PCILeech resources already extracted to temp
...
```

**Root Cause**: `GetPCILeechPath()` was being called multiple times (likely from UI updates) and logging every time it checked if resources existed.

**Fix**:
- Removed log message when resources are already extracted
- Only log when actually extracting resources
- Silent check when resources already exist

**Before**:
```cpp
if (!needsExtraction)
{
    std::cout << "[INFO] PCILeech resources already extracted to temp" << std::endl;
    return pcileechExe;
}
```

**After**:
```cpp
if (!needsExtraction)
{
    // Don't log every time - resources are already extracted
    return pcileechExe;
}
```

**Files Modified**:
- `src/Backend/BenchmarkInterface.cpp` - `GetPCILeechPath()` - Removed spam logging
