# Benchmark Test Issues - Diagnosis and Fixes

## Date: 2025-12-03

## Issues Found

### 1. ? FIXED: Memory Ranges Duplicated
**Problem**: Memory ranges were listed twice in output:
```
[+] Adding memory range: 1000 - 5E000
[+] Adding memory range: 5F000 - A0000
...
[+] Adding memory range: 1000 - 5E000  ? DUPLICATE
[+] Adding memory range: 5F000 - A0000  ? DUPLICATE
```

**Root Cause**: When LeechCore test fails and falls back to PCILeech test, both functions add to the same `m_CurrentResults.memoryRanges` vector without clearing it first.

**Fix**: Clear the vector in `RunQuickTest()` before adding ranges:
```cpp
// Clear previous memory ranges if this is a fallback (avoid duplication)
if (!m_CurrentResults.memoryRanges.empty())
{
    m_CurrentResults.memoryRanges.clear();
}
```

---

### 2. ?? INVESTIGATING: LeechCore Device Creation Fails

**Problem**: LeechCore initialization fails even though:
- ? FTDI driver is correctly installed (1.4.0.1)
- ? Device shows in Device Manager as "FTDI SuperSpeed-FIFO Bridge"
- ? FTD3XX.dll loads successfully
- ? leechcore.dll loads successfully
- ? `LcCreate()` returns NULL

**Error Message**:
```
[ERROR] Failed to create LeechCore device.
Device config used: fpga
```

**Possible Root Causes**:

1. **Device Not in Correct Mode**
   - Many DMA devices have multiple modes (JTAG, FIFO, etc.)
   - Device may need to be in specific mode for FPGA operation
   - Check if device has mode switches or jumpers

2. **Bitstream Not Loaded**
   - FPGA devices require a bitstream to be loaded
   - Bitstream must match the hardware
   - May need to load bitstream through JTAG first

3. **USB Power Issue**
   - USB 3.0 ports provide 900mA max
   - Some DMA devices need more power
   - Try powered USB hub or different port

4. **Device Firmware**
   - FT601 firmware may need to be in "245 FIFO mode"
   - Device EEPROM may need specific configuration
   - Check with FT_PROG utility

5. **Administrator Rights**
   - Some USB devices require admin access
   - Try running DMATool as Administrator

6. **Other Software Conflict**
   - Another program may have the device open
   - Check Task Manager for PCILeech, Arbiter, etc.

**Improved Error Messages**: Added comprehensive troubleshooting steps to the error message.

---

### 3. ?? INVESTIGATING: PCILeech Benchmark Fails

**Problem**: PCILeech fallback also fails:
```
[ERROR] Failed to execute PCILeech benchmark
```

**Possible Causes**:

1. **Wrong Command-Line Arguments**
   - Current command: `pcileech.exe benchmark`
   - May need device specification: `pcileech.exe benchmark -device fpga`
   - Or full config: `pcileech.exe benchmark -device fpga://algo=0`

2. **Same Device Issue as LeechCore**
   - If LeechCore can't see the device, PCILeech probably can't either
   - They use the same underlying FTD3XX.dll

3. **PCILeech Requires Config File**
   - Some versions of PCILeech need a config file
   - Config might need to specify device type

**Fix Applied**: Added debug output to show PCILeech's actual output/error messages.

---

## Diagnostic Steps

### Step 1: Check Device in Device Manager

1. Open Device Manager (`devmgmt.msc`)
2. Look for "FTDI SuperSpeed-FIFO Bridge"
3. Right-click ? Properties ? Driver tab
4. Verify:
   - Driver Provider: **FTDI**
   - Driver Version: **1.4.0.1**
   - Driver Date: **6/10/2025**

**If you see "USB Composite Device"**:
- This is the wrong device (parent device)
- Look for the actual FIFO bridge as a child device

**If device shows with yellow exclamation**:
- Driver issue - reinstall driver
- Or device not functioning properly

### Step 2: Check Device Mode/Firmware

**Use FT_PROG Utility** (FTDI's official tool):
1. Download from FTDI website
2. Open FT_PROG
3. Click "Scan and Parse"
4. Check device configuration:
   - Should be in "245 FIFO Mode"
   - USB 3.0 enabled
   - Interface A configured correctly

**If device has physical switches/jumpers**:
- Check manual for correct settings
- JTAG mode vs FIFO mode
- Config mode vs Run mode

### Step 3: Test with Official PCILeech

**Download standalone PCILeech**:
```powershell
# Test if standalone PCILeech can see the device
pcileech.exe probe -device fpga
```

Expected output if working:
```
Device: FPGA
Connected: Yes
Version: [firmware version]
```

If this also fails, the issue is with the device/driver, not DMATool.

### Step 4: Check USB Connection

1. **Use USB 3.0 port** (blue port, not black USB 2.0)
2. **Try different USB 3.0 ports**
3. **Try powered USB 3.0 hub** if device needs more power
4. **Check Device Manager ? USB controllers**:
   - Should show "USB 3.0 eXtensible Host Controller"
   - If only USB 2.0, that's the problem

### Step 5: Run as Administrator

```powershell
# Right-click DMATool.exe ? Run as administrator
```

Some USB device operations require admin rights.

### Step 6: Check for Conflicting Software

1. Open Task Manager
2. Look for these processes:
   - `pcileech.exe`
   - `arbiter.exe`
   - Any other DMA tools
3. End them if running
4. Try DMATool again

---

## What Works

? **Driver Detection**: Now correctly identifies FTDI driver 1.4.0.1  
? **Resource Extraction**: All DLLs extracted correctly  
? **DLL Loading**: FTD3XX.dll and leechcore.dll load successfully  
? **Memory Range Duplication**: Fixed  

---

## What Doesn't Work (On Test PC)

? **LeechCore Device Creation**: `LcCreate()` returns NULL  
? **PCILeech Benchmark**: Command fails (no output)  

**These two issues are related** - both suggest the device is not accessible/not in correct state.

---

## Next Steps

1. **Run diagnostic checks above** on test PC
2. **Try standalone PCILeech** to isolate if it's a DMATool issue or device issue
3. **Check device firmware/mode** with FT_PROG
4. **Verify USB 3.0 connection** and power
5. **Share results** so we can determine if it's:
   - Device configuration issue
   - Firmware/bitstream issue
   - DMATool code issue

---

## Files Modified

1. `src/Backend/BenchmarkInterface.cpp`
   - Fixed memory range duplication by clearing vector in fallback
   - Added debug output for PCILeech failures

2. `src/Backend/LeechCoreWrapper.cpp`
   - Enhanced error messages with comprehensive troubleshooting
   - Added specific diagnostic steps

3. `src/Backend/FT601DriverInterface.cpp`
   - Fixed driver detection to query correct device (FIFO bridge, not composite)

---

## Build Status

? **Build successful**

---

## Testing

**On Development PC**: ? Driver detection works  
**On Test PC**: ? Driver detection works, ? Device creation fails  

**Hypothesis**: Device is not in correct mode/state for FPGA operation, even though driver is installed correctly.

---

## Questions to Answer

1. **What type of DMA device is this?**
   - Squirrel?
   - Screamer?
   - Custom FPGA board?

2. **Does it have a bitstream loaded?**
   - Needs to be loaded via JTAG?
   - Or comes pre-programmed?

3. **What mode should it be in?**
   - Any switches or jumpers?
   - Configuration needed?

4. **Does standalone PCILeech work?**
   - If not, it's a device/firmware issue
   - If yes, it's a DMATool integration issue

---

**Status**: Driver fixes complete, investigating hardware/firmware configuration issues
