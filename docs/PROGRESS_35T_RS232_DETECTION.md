# 35T RS-232 Detection & Driver Management - Progress Report

**Date:** December 12, 2024  
**Status:** In Progress - Driver Detection Working, Installation Needs Completion

---

## Overview
Adding support for the **35T DMA card** which uses the **FTDI FT4232H Quad RS-232-HS** chipset for JTAG communication (Interface 0). This is separate from the existing 75T/100T cards which use CH347.

### Hardware Details
- **Card:** 35T DMA Card
- **JTAG Chipset:** FTDI FT4232H (Quad RS-232-HS)
- **Interface:** Interface 0 (MI_00)
- **VID:PID:** 0403:6011
- **Default Driver:** FTDIBUS (FTDI Serial Port Driver)
- **Required Driver:** WinUSB (for OpenOCD JTAG operations)

---

## What's Working ✅

### 1. Hardware Detection
- ✅ Auto-detects 35T vs 75T/100T based on VID/PID
- ✅ Searches for `VID_0403&PID_6011&MI_00` (35T)
- ✅ Searches for `VID_1A86&PID_55DD` or `PID_55DE` (75T/100T)
- ✅ Displays detected card type in logs

### 2. Driver Status Checking
- ✅ Detects FTDIBUS vs WinUSB driver
- ✅ Shows device name: "USB Serial Port (COM10)" (instead of parent device name)
- ✅ Displays driver status with clear messaging:
  - When FTDIBUS installed: Shows "(FTDIBUS - Needs WinUSB)" 
  - When WinUSB installed: Shows driver as installed
- ✅ Shows adapter type as "RS-232" (not CH347)
- ✅ Displays driver version and provider info

### 3. UI State Management
- ✅ Detect FPGA & Read DNA buttons are **disabled** until driver check is performed
- ✅ Install/Uninstall buttons are **grayed out** until driver check is performed
- ✅ Buttons dynamically switch between "Install RS232 Driver" and "Install CH347 Driver" based on detected card
- ✅ Tooltips guide users to check driver status first

### 4. Code Architecture
- ✅ Unified card detection in `DetectHardware()` function
- ✅ Separate `CheckRS232Driver()` function (parallel to `CheckCH347Driver()`)
- ✅ Card type stored in memory (`detectedCardType_`)
- ✅ Smart button state management based on card type

---

## What Needs to be Done ⚠️

### 1. **Automated RS232 Driver Installation** (HIGH PRIORITY)
**Status:** NOT WORKING - pnputil fails even with correct files

#### Problem:
- Driver files extract successfully from embedded resources
- Files confirmed to exist on disk at `C:\Users\suni\AppData\Local\Temp\DMATool\drivers\rs232\`
- `pnputil.exe /add-driver` command fails with "The system cannot find the file specified"
- Same failure occurs with manual pnputil command from PowerShell

#### Investigation Results:
1. **WinUSB_Driver folder** has incorrect/incomplete files:
   - `ftdi_winusb.inf` - UTF-16 BOM encoded (causes issues)
   - `ftdi_winusb.cat` - Mismatched catalog name
   - Missing `WdfCoInstaller01011.dll` (required by INF)

2. **Zadig_Signed folder** has WORKING files:
   - `quad_rs232-hs_(interface_0).inf` - Properly formatted
   - `Quad_RS232-HS_(Interface_0).cat` - Matching catalog file
   - Manual test with pnputil **SUCCEEDED**: `pnputil /add-driver "C:\Users\suni\source\repos\DMATool\tools\FTDI_RS232_Driver\Zadig_Signed\quad_rs232-hs_(interface_0).inf" /install`

#### Solution Path:
1. **Update embedded resources** to use Zadig_Signed files instead of WinUSB_Driver files
2. **Embed the correct files:**
   - `quad_rs232-hs_(interface_0).inf`
   - `Quad_RS232-HS_(Interface_0).cat`
3. **Update InstallRS232Driver()** to use these files
4. **Note:** After `pnputil /add-driver /install`, the device still uses FTDIBUS because Windows prefers it
5. **Need to force driver switch** using one of:
   - PowerShell `Update-PnpDevice` (not available on this system)
   - Device Manager manual update
   - Zadig tool (known to work)
   - DevCon utility
   - `pnputil /disable-device` + `/enable-device` (tested - doesn't force driver change)

### 2. **RS232 Driver Uninstallation**
**Status:** NOT IMPLEMENTED

#### Need to Implement:
- `UninstallRS232Driver()` function to revert WinUSB back to FTDIBUS
- Should install FTDIBUS driver from `tools\FTDI_RS232_Driver\FTDIBUS_Driver\`
- Files available:
  - `ftdibus.inf`
  - `ftdibus.cat`
  - `ftdibus.sys`
  - `ftser2k.sys`
  - `ftbusui.dll`
  - `ftd2xx.dll`

### 3. **Device Name Display Format**
**Status:** WORKING but could be improved

Current display:
```
Device: USB Serial Port (COM10)
(FTDIBUS - Needs WinUSB)
```

Suggested improvement:
```
Device: USB Serial Port (COM10)
Driver: FTDIBUS (Needs WinUSB)
```

### 4. **Testing with 75T/100T Cards**
**Status:** PENDING HARDWARE SWAP

- Current code has logic for 75T/100T (CH347) which was working before
- Need to test with actual 75T/100T hardware to ensure:
  - Auto-detection still works
  - Driver checking works for both card types
  - Button labels switch correctly
  - Install/uninstall for CH347 still works

---

## File Locations

### Source Code
- Main detection logic: `DMATool\OpenOCDIntegration.cpp`
  - `DetectHardware()` - Auto-detects card type
  - `CheckRS232Driver()` - Checks RS232 driver status
  - `InstallRS232Driver()` - (Needs completion)
  - `UninstallRS232Driver()` - (Needs implementation)

### Driver Files
- **Working files (use these):**
  - `C:\Users\suni\source\repos\DMATool\tools\FTDI_RS232_Driver\Zadig_Signed\`
- **Incomplete files (do not use):**
  - `C:\Users\suni\source\repos\DMATool\tools\FTDI_RS232_Driver\WinUSB_Driver\`
- **Revert driver:**
  - `C:\Users\suni\source\repos\DMATool\tools\FTDI_RS232_Driver\FTDIBUS_Driver\`

### Build Output
- Executable: `C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe`

---

## Next Steps (Priority Order)

1. **HIGH:** Fix automated driver installation
   - Embed Zadig_Signed files as resources
   - Update InstallRS232Driver() to extract and use correct files
   - Implement forced driver switching (likely need Zadig or DevCon)

2. **MEDIUM:** Implement driver uninstallation
   - Create UninstallRS232Driver() function
   - Embed FTDIBUS_Driver files
   - Test reversion process

3. **MEDIUM:** Test with 75T/100T hardware
   - Verify CH347 detection still works
   - Test button switching between RS232 and CH347
   - Verify CH347 install/uninstall still works

4. **LOW:** Improve UI text formatting
   - Adjust device name display format
   - Clean up status messages

---

## Known Issues

1. **pnputil file not found error**
   - Even when files exist and are verified
   - Occurs with both embedded extraction and manual file paths
   - Zadig_Signed folder files DO work with manual pnputil command

2. **Driver doesn't auto-apply after pnputil**
   - Windows prefers FTDIBUS over generic WinUSB
   - Disable/enable device doesn't force driver switch
   - Requires manual intervention (Device Manager or Zadig)

3. **COM port naming**
   - COM port number varies per system (COM9, COM10, etc.)
   - Current detection correctly finds COM port by matching parent device
   - Displays actual COM port name instead of parent device

---

## Testing Notes

### Manual Driver Installation Test (SUCCESSFUL)
```powershell
# This command WORKS:
pnputil.exe /add-driver "C:\Users\suni\source\repos\DMATool\tools\FTDI_RS232_Driver\Zadig_Signed\quad_rs232-hs_(interface_0).inf" /install

# Output:
# Adding driver package:  quad_rs232-hs_(interface_0).inf
# Driver package added successfully.
# Published Name:         oem75.inf
# Driver package is up-to-date on device: USB\VID_0403&PID_6011&MI_00\7&1230f973&0&0000
```

### Device Information
```
Instance ID: USB\VID_0403&PID_6011&MI_00\7&1230F973&0&0000
Friendly Name: USB Serial Converter A (parent device)
Child Device: USB Serial Port (COM10) ← Interface 0
Hardware ID: USB\VID_0403&PID_6011&REV_0800&MI_00
Default Service: FTDIBUS
Required Service: WinUSB
```

---

## References

### Resources Checked
- libwdi source: `C:\Users\suni\Downloads\libwdi-1.5.1\`
- Zadig GitHub: https://github.com/pbatard/libwdi
- Current driver folder: `C:\Users\suni\source\repos\DMATool\tools\FTDI_RS232_Driver\`

### Documentation
- RS232 Detection Implementation: `docs\RS232_DETECTION_IMPLEMENTATION.md`
- This Progress Report: `docs\PROGRESS_35T_RS232_DETECTION.md`

---

## Build Information

**Last Build:** December 12, 2024 10:42 AM  
**Configuration:** Debug x64  
**Executable Location:** `C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe`

---

## Summary

✅ **Detection and status checking are fully functional**  
⚠️ **Driver installation needs to be completed using Zadig_Signed files**  
❌ **Driver uninstallation not yet implemented**  
📝 **Need to test with 75T/100T hardware after 35T work is complete**

The foundation is solid - all the detection and UI logic works correctly. The remaining work is primarily focused on fixing the automated driver installation process by using the correct (Zadig_Signed) files and implementing a method to force Windows to actually switch from FTDIBUS to WinUSB.
