# CH347 Communication Error Troubleshooting

## Problem
Both DMATool and the official CH347FPGATool fail with:
```
Error: CH347_Read read data failure.
Error: CH347 clear Buffer Error.
```

## Root Cause
This is **NOT a software bug** - it's a hardware communication issue where the CH347 adapter has gotten into a bad state and cannot communicate with the FPGA via JTAG.

## Evidence
1. ? Official CH347FPGATool shows the **exact same errors**
2. ? DMATool shows the **exact same errors**
3. ? OpenOCD loads configs successfully
4. ? OpenOCD fails during `init` when trying to communicate with hardware

## Solution: Reset the CH347 Adapter

### Option 1: Quick Manual Reset (Recommended)
1. **Unplug** the CH347 USB cable from your computer
2. **Wait 5-10 seconds**
3. **Plug it back in**
4. **Wait 3 seconds** for Windows to detect it
5. Try again

### Option 2: Automated Script Reset
```powershell
# Run as Administrator
.\scripts\Reset-CH347-Adapter.ps1
```

### Option 3: Device Manager Reset
1. Open **Device Manager**
2. Find **"USB HighSpeed-JTAG/I2C... CH347T"** under "Universal Serial Bus devices"
3. Right-click ? **Disable device**
4. Wait 2 seconds
5. Right-click ? **Enable device**
6. Wait 3 seconds
7. Try again

## Hardware Checklist

Before blaming the software, verify:

| Item | Status | Notes |
|------|--------|-------|
| ? CH347 USB cable plugged in | | Must be connected to computer |
| ? CH347 power LED on | | Should see a light on the CH347 board |
| ? JTAG cable connected | | CH347 ? FPGA board |
| ? FPGA board powered | | DMA card must have power |
| ? Correct JTAG pins | | TDI, TDO, TCK, TMS, GND |
| ? JTAG cable orientation | | Pin 1 marker aligned correctly |
| ? No other app using CH347 | | Close all tools before testing |

## Common Causes

### 1. **CH347 Buffer Overflow** (Most Common)
- **Symptom**: `CH347_Read read data failure`
- **Cause**: Previous operation didn't complete cleanly
- **Fix**: Unplug/replug USB cable or run reset script

### 2. **No JTAG Connection**
- **Symptom**: Same error as #1
- **Cause**: JTAG cable disconnected or loose
- **Fix**: Check physical JTAG cable connection

### 3. **FPGA Not Powered**
- **Symptom**: Same error as #1
- **Cause**: DMA card has no power
- **Fix**: Ensure DMA card is powered via PCIe or external power

### 4. **Wrong JTAG Pinout**
- **Symptom**: Same error as #1
- **Cause**: JTAG cable connected to wrong pins
- **Fix**: Verify pinout matches FPGA datasheet

### 5. **USB Port Issue**
- **Symptom**: Intermittent failures
- **Cause**: USB 3.0 ports can cause issues
- **Fix**: Try a USB 2.0 port instead

## Diagnostic Commands

### Check CH347 Status
```powershell
Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'}
```

**Expected Output:**
```
Status     Class           FriendlyName
------     -----           ------------
OK         USB             USB HighSpeed-JTAG/I2C... CH347T
```

### Test OpenOCD Communication
```powershell
# Set environment
$env:OPENOCD_SCRIPTS = "$env:TEMP\DMATool"

# Run minimal test
& "$env:TEMP\DMATool\openocd.exe" `
    -c "adapter driver ch347" `
    -c "ch347 vid_pid 0x1a86 0x55dd" `
    -c "adapter speed 1000" `
    -c "init" `
    -c "shutdown"
```

**If this works:** Software is fine, check JTAG connection
**If this fails:** CH347 needs reset or hardware issue

## Error Pattern Analysis

| Error Message | Meaning | Likely Cause |
|--------------|---------|--------------|
| `CH347_Read read data failure` | Can't read from CH347 USB | Buffer stuck, USB issue, or no JTAG target |
| `CH347 clear Buffer Error` | Can't reset CH347's internal buffer | Device in bad state, needs reset |
| `tap/device found: 0x...` followed by error | JTAG TAP detected but communication failed | Intermittent connection |
| No TAP detected | JTAG not connected at all | Check physical connections |

## What This Is NOT

? **NOT a DMATool bug** - Official tool fails the same way
? **NOT a config file issue** - Configs load successfully
? **NOT an OpenOCD version issue** - Same OpenOCD binary in both tools
? **NOT a driver issue** - Device shows as "OK" in Device Manager

## What This IS

? **Hardware communication problem**
? **CH347 adapter in bad state**
? **Missing or incorrect JTAG connection**
? **Power/connection issue**

## Recovery Steps (In Order)

1. **Kill all OpenOCD processes**
   ```powershell
   taskkill /F /IM openocd.exe /T
   ```

2. **Reset CH347 adapter**
   - Unplug USB cable
   - Wait 5 seconds
   - Plug back in

3. **Verify device status**
   ```powershell
   Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'}
   ```

4. **Check JTAG connections**
   - Cable connected?
   - FPGA powered?
   - Pins correct?

5. **Try different USB port**
   - Prefer USB 2.0 over USB 3.0

6. **Test with official tool first**
   - If official tool works, compare settings
   - If official tool fails, it's definitely hardware

## Success Indicators

After reset, you should see:
```
[INFO] OpenOCD found at: C:\Users\...\openocd.exe
[INFO] Detected adapter: CH347
[SUCCESS] FPGA detected: XC7A75T
[INFO] IDCODE: 0x13631093
[INFO] DNA ID: <64-bit hex value>
```

## Still Not Working?

If after trying all the above, it still fails:

1. **Test with different FPGA board** (if available)
2. **Test with different CH347 adapter** (if available)
3. **Check JTAG cable with multimeter** (continuity test)
4. **Try from a different computer**
5. **Check FPGA JTAG pins aren't damaged**

## Quick Reference

| Issue | Quick Fix |
|-------|-----------|
| `CH347_Read failure` | Unplug/replug USB |
| Official tool also fails | Hardware issue, not software |
| Device shows OK but fails | Reset adapter |
| Works sometimes | Bad USB port or cable |

---

**Remember**: If the official CH347FPGATool shows the same error, it's 100% a hardware/connection issue, not a problem with DMATool!

**Last Updated**: 2025-12-02
**Status**: Hardware troubleshooting complete
