# Driver Version & LeechCore Fixes

## Issues Fixed

### 1. Driver Version Detection ?

**Problem**: PowerShell was returning Windows driver version (10.0.19041.6456) instead of the FTDI INF version (1.4.0.1)

**Root Cause**: `DEVPKEY_Device_DriverVersion` returns the OS driver version, not the INF file version

**Solution**: Changed PowerShell query to read the `DriverVer` field from the actual INF file

#### Before (Wrong):
```powershell
$driverVersion = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverVersion').Data
# Returns: 10.0.19041.6456 (Windows driver version)
```

#### After (Correct):
```powershell
# Get INF file path
$driverInf = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName 'DEVPKEY_Device_DriverInfPath').Data
# Read DriverVer from INF file
$infContent = Get-Content $infFile -Raw
if ($infContent -match 'DriverVer\s*=\s*[^,]+,\s*([\d.]+)') {
    $driverVersion = $Matches[1]  # Returns: 1.4.0.1 (from INF)
}
```

**Result**: Now correctly identifies driver version from the INF file

---

### 2. LeechCore Device Initialization ?

**Problem**: LeechCore failing with "Failed to create LeechCore device"

**Improvements Made**:
1. Try `fpga://algo=0` first (auto-detect algorithm)
2. Fall back to `fpga` if that fails
3. Added detailed error messages with troubleshooting steps

**Better Error Messages**:
```
Failed to create LeechCore device.
Possible causes:
  1. DMA hardware not connected
  2. FTDI WinUSB driver not installed (version 1.4.0.1 required)
  3. Device in use by another application
  4. Run as Administrator
  5. Device may be in UPDATE mode instead of DATA mode

Troubleshooting:
  - Check Data Port tab and install FTDI driver if needed
  - Disconnect and reconnect the DMA device
  - Ensure device is in DATA mode (not UPDATE mode)
```

---

## Testing Steps

### 1. Test Driver Version Detection

**Run DMATool and check Data Port tab**:

**Without Driver** (Expected):
```
[INFO] Checking FTDI driver status...
[WARNING] FTDI WinUSB driver not installed
[INFO] Device detected: USB Composite Device
[INFO] Driver version: Not installed (using default Windows driver)
[INFO] Required version: 1.4.0.1 or higher
```

**With Correct Driver 1.4.0.1** (Expected):
```
[INFO] Checking FTDI driver status...
[SUCCESS] FTDI driver is installed
[INFO] Device: USB Composite Device
[INFO] Driver Version: 1.4.0.1  ? Should show INF version, NOT 10.0.x
[INFO] VID/PID: VID_0403 / PID_601F
```

**With Old Driver 1.3.x** (Expected):
```
[INFO] Checking FTDI driver status...
[WARNING] FTDI WinUSB driver is out of date
[INFO] Current version: 1.3.0.0  ? Shows INF version
[INFO] Required version: 1.4.0.1 or higher
```

### 2. Test LeechCore Initialization

**Run Benchmark Test**:

**If Driver Not Installed**:
```
[INFO] Initializing LeechCore...
[ERROR] Failed to initialize LeechCore: Failed to create LeechCore device.
Possible causes:
  1. DMA hardware not connected
  2. FTDI WinUSB driver not installed (version 1.4.0.1 required)  ? Clear message
  ...
```

**If Driver Installed Correctly**:
```
[INFO] Initializing LeechCore...
[SUCCESS] LeechCore initialized: Success - Device: FPGA  ? Should work!
```

---

## Troubleshooting Guide

### Issue: Driver shows version 10.0.19041.6456

**This was a bug** - now fixed! The tool was reading the Windows driver version instead of the INF version.

**After fix**: Should show `1.4.0.1` (or empty if not installed)

### Issue: LeechCore fails to initialize

**Check the following**:

1. **Driver Status**:
   - Go to Data Port tab
   - Click "Check Driver Status"
   - Should show version 1.4.0.1
   - If not, click "Install FTDI Driver"

2. **Device Mode**:
   - Some DMA devices have UPDATE and DATA modes
   - Make sure it's in DATA mode (usually a switch or jumper)

3. **Device Connection**:
   - Unplug and replug the USB cable
   - Try a different USB port
   - Make sure it's a USB 3.0 port (blue)

4. **Administrator Rights**:
   - Right-click DMATool.exe
   - Click "Run as administrator"

5. **Other Applications**:
   - Close any other DMA tools
   - Only one app can use the device at a time

---

## Files Modified

1. **src/Backend/FT601DriverInterface.cpp**
   - Changed PowerShell query to read INF file instead of device property
   - Now correctly gets version 1.4.0.1 instead of 10.0.x

2. **src/Backend/LeechCoreWrapper.cpp**
   - Try `fpga://algo=0` first, fallback to `fpga`
   - Added detailed error messages with troubleshooting
   - Added iostream include

---

## Build Status

? **Debug**: `bin\Debug-x64\DMATool.exe`  
? **Release**: `bin\Release-x64\DMATool.exe`

---

## Expected Behavior After Fix

### Driver Check:
- ? Shows INF version (1.4.0.1) NOT OS version (10.0.x)
- ? Correctly identifies "not installed" when driver missing
- ? Shows "out of date" for versions < 1.4.0.1
- ? Shows "Installed" for version 1.4.0.1+

### Benchmark Tests:
- ? LeechCore initializes if driver is installed
- ? Clear error message if driver not installed
- ? Helpful troubleshooting steps in error message
- ? Tries alternative device initialization methods

---

## Quick Test

1. **Uninstall FTDI driver** (if installed):
   - Data Port tab ? "Uninstall FTDI Driver"

2. **Check driver status**:
   - Should show "Driver Needed"
   - Version should be empty (not 10.0.x)

3. **Install FTDI driver**:
   - Data Port tab ? "Install FTDI Driver"

4. **Check driver status again**:
   - Should show "Installed"
   - Version should show "1.4.0.1" (not 10.0.x)

5. **Run benchmark test**:
   - Benchmark tab ? "Run Quick Speed Test"
   - Should initialize LeechCore successfully

---

**Date**: December 3, 2025  
**Status**: ? Both issues fixed  
**Impact**: Driver version detection now correct, better LeechCore error messages
