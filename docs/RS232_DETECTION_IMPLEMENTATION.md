# RS232 (35T) Detection & Driver Management Implementation

**Date**: December 12, 2024  
**Status**: Phase 1 Complete - Manual Driver Installation Working  
**Next Phase**: Embed WinUSB Driver Resources

---

## Overview

This document tracks the implementation of automatic DMA card detection and driver management for both RS232 (35T) and CH347 (75T/100T) cards. The goal is to provide a seamless user experience where the tool automatically detects the card type and guides users through proper driver installation.

---

## ✅ Completed Features

### 1. **Hardware Detection System**

#### Unified Card Detection (`DetectDMACard()`)
- ✅ Detects 35T cards by VID/PID: `VID_0403&PID_6011&MI_00` (FTDI FT4232H)
- ✅ Detects 75T/100T cards by VID/PID: `VID_1A86&PID_55DD` or `VID_1A86&PID_55DE` (CH347)
- ✅ Works in all driver states: correct driver, wrong driver, no driver, unknown device
- ✅ Driver-agnostic detection using Windows PnP device enumeration

**Location**: `src/Backend/OpenOCDInterface.cpp` - `DetectDMACard()`

#### RS232 Driver Detection (`CheckRS232Driver()`)
- ✅ Queries Interface 0 of FTDI FT4232H: `VID_0403&PID_6011&MI_00`
- ✅ Distinguishes between driver states:
  - **FTDIBUS** (default) → Marks as "not installed" (wrong driver for JTAG)
  - **WinUSB** (custom) → Marks as "installed" (correct driver for JTAG)
  - **None** → Marks as "not installed"
- ✅ Returns detailed driver info: version, provider, VID/PID, service type

**Location**: `src/Backend/OpenOCDInterface.cpp` - `CheckRS232Driver()`

---

### 2. **Smart UI Flow**

#### Failsafe Detection Workflow
- ✅ **On Launch**: All detection buttons grayed out
- ✅ **"Check Driver Status" Required First**: Detects hardware and driver state
- ✅ **"Detect FPGA & Read DNA" Enabled After**: Only when driver status is checked
- ✅ **Driver Install Required Check**: If wrong/missing driver detected, FPGA detection stays grayed out

**User Flow**:
```
1. Launch DMATool
   └─> Status: "---"
   └─> Install/Uninstall: Grayed out (no card detected)
   └─> Detect FPGA: Grayed out ("Check Driver First")

2. Click "Check Driver Status"
   └─> Detects: "35T DMA card" or "75T/100T DMA card"
   └─> Shows: Adapter type (RS-232 or CH347)
   └─> Shows: Driver status (Installed or Driver Install Needed)
   └─> Enables: Install/Uninstall buttons (correct driver type)
   └─> Detect FPGA: Still grayed out if wrong driver ("Install Driver First")

3. Install Correct Driver
   └─> 35T: Opens Zadig for WinUSB installation (temporary)
   └─> 75T/100T: Uses embedded CH347 driver installer

4. After Correct Driver Installed
   └─> Status: "Installed"
   └─> Detect FPGA: Enabled
   └─> User can detect FPGA and read DNA
```

**Location**: `src/UI/Tabs/JTAGPortTab.cpp`

---

### 3. **Dynamic Button Adaptation**

#### Automatic Button Labels
- ✅ Buttons change from "DMA Card Driver" to specific driver type
- ✅ **35T detected** → "Install RS232 Driver" / "Uninstall RS232 Driver"
- ✅ **75T/100T detected** → "Install CH347 Driver" / "Uninstall CH347 Driver"
- ✅ Install/Uninstall call correct backend functions based on `adapterType`

#### Smart Button States
- ✅ **Before card detection**: Grayed out with "(No Card Detected)" suffix
- ✅ **After card detection**: Enabled with correct driver name
- ✅ **Tooltips**: Helpful messages when buttons are disabled

**Location**: `src/UI/Tabs/JTAGPortTab.cpp` - Lines 1100-1350

---

### 4. **Driver Status Display**

#### Status Indicator Logic
- ✅ **Before driver check**: Shows `---`
- ✅ **Correct driver installed**: Shows `Installed` (green)
- ✅ **Wrong/missing driver**: Shows `Driver Install Needed` (yellow warning)

#### Adapter Display
- ✅ Shows `Unknown` before detection
- ✅ Shows `RS-232` for 35T cards (blue)
- ✅ Shows `CH347` for 75T/100T cards (green)
- ✅ Prioritizes `s_FPGAInfo.adapterType` set by card detection

**Location**: `src/UI/Tabs/JTAGPortTab.cpp` - Lines 906-940

---

### 5. **Backend Driver Management**

#### CH347 Driver (75T/100T) - Fully Implemented
- ✅ `InstallCH347Driver()`: Extracts embedded driver files, uses pnputil
- ✅ `UninstallCH347Driver()`: Removes WCH driver packages
- ✅ `CheckCH347Driver()`: Validates HighSpeed-JTAG vs UART+JTAG driver

#### RS232 Driver (35T) - Phase 1 (Manual Install)
- ✅ `CheckRS232Driver()`: Detects FTDIBUS vs WinUSB
- ✅ `InstallRS232Driver()`: Opens Zadig website with instructions (temporary)
- ✅ `UninstallRS232Driver()`: Finds and removes WinUSB driver via pnputil
- ⏳ **TODO**: Embed WinUSB driver files as resources (like CH347)

**Location**: `src/Backend/OpenOCDInterface.cpp`

---

## 🔧 In Progress / Pending

### Phase 2: Automated RS232 WinUSB Driver Installation

**Current State**: Uses Zadig tool for manual installation  
**Target State**: Fully automated WinUSB installation

#### Issue Discovered (Dec 12, 2024):
The Zadig-generated INF file requires additional components not included in our current package:
- **WdfCoInstaller01011.dll** - Part of Windows Driver Kit (WDK), ~1MB file
- **Architecture-specific folder structure** - Needs x86/amd64/arm subdirectories
- **CAT file name mismatch** - INF references `Quad_RS232-HS_(Interface_0).cat` but we have `ftdi_winusb.cat`

#### Solutions for Phase 2:

**Option 1: Integrate libwdi Library (Recommended)**
- Use the same library that Zadig uses: https://github.com/pbatard/libwdi
- Handles driver signing and installation automatically
- No manual INF file management
- Well-tested and maintained
- Implementation: Link libwdi.lib and use `wdi_prepare_driver()` + `wdi_install_driver()`

**Option 2: Create Simplified INF (Modern Windows)**
- Modern Windows 10/11 doesn't require WdfCoInstaller
- Create simplified INF that uses built-in Windows WinUSB support
- Self-sign or use unsigned (test mode)
- Less dependencies, but requires test signing on production machines

**Option 3: Include Full Zadig Driver Package**
- Download and embed WdfCoInstaller01011.dll
- Create proper folder structure: drivers/rs232/x86/, drivers/rs232/amd64/
- Update CAT file references in INF
- Most complex, requires ~1MB additional space

#### Current Working Solution:
- Device detection: ✅ Works perfectly
- Driver checking: ✅ Distinguishes FTDIBUS vs WinUSB
- Manual installation: ✅ User installs WinUSB via Zadig
- Button workflow: ✅ Smart failsafe detection

**Recommendation**: Use Option 1 (libwdi) for Phase 2 - it's the most robust and maintainable solution.

---

### Files That Need to Be Added (if continuing with embedded approach):

**From Windows Driver Kit (WDK):**
- WdfCoInstaller01011.dll (x86 version)
- WdfCoInstaller01011.dll (amd64 version)  
- WdfCoInstaller01011.dll (arm version)

**Repository Reference**: https://github.com/pbatard/libwdi/tree/master/libwdi/.msvc

---

## 📋 Testing Checklist

### 35T (RS232) Testing
- [x] Detect with FTDIBUS driver (default)
- [x] Detect with WinUSB driver
- [x] Button labels show "RS232"
- [x] Adapter shows "RS-232"
- [x] Status shows "Driver Install Needed" with FTDIBUS
- [x] Status shows "Installed" with WinUSB
- [x] "Check Driver Status" identifies 35T correctly
- [ ] Automated WinUSB installation (pending Phase 2)
- [ ] Driver uninstall restores FTDIBUS

### 75T/100T (CH347) Testing
- [x] Detect with correct driver (HighSpeed-JTAG)
- [x] Detect with wrong driver (UART+JTAG)
- [x] Button labels show "CH347"
- [x] Adapter shows "CH347"
- [x] Install/Uninstall CH347 driver works
- [x] FPGA detection distinguishes 75T vs 100T

### Edge Cases
- [x] No card connected → All buttons grayed out appropriately
- [x] Multiple cards → Detection prioritizes 35T, then CH347
- [x] Unknown device (no driver) → Detects by VID/PID
- [x] Driver check before FPGA detection enforced
- [x] Tooltips guide user through workflow

---

## 🗂️ File Structure

### Modified Files
```
src/
├── Backend/
│   ├── OpenOCDInterface.h          # Added CardInfo struct, RS232 functions
│   └── OpenOCDInterface.cpp        # Implemented DetectDMACard(), CheckRS232Driver()
└── UI/
    └── Tabs/
        ├── JTAGPortTab.h           # Added s_DriverCheckCompleted flag
        └── JTAGPortTab.cpp         # Updated detection flow, smart buttons

docs/
├── CARD_DETECTION_RESEARCH.md      # Research findings (created Dec 12)
├── IMPLEMENTATION_SUMMARY.md       # Implementation details (created Dec 12)
└── RS232_DETECTION_IMPLEMENTATION.md  # This file
```

### Key Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `DetectDMACard()` | OpenOCDInterface.cpp:638 | Hardware-first detection by VID/PID |
| `CheckRS232Driver()` | OpenOCDInterface.cpp:554 | Validate RS232 driver state |
| `CheckCH347Driver()` | OpenOCDInterface.cpp:496 | Validate CH347 driver state |
| `InstallRS232Driver()` | OpenOCDInterface.cpp:1126 | Install WinUSB (Zadig for now) |
| `UninstallRS232Driver()` | OpenOCDInterface.cpp:1168 | Remove WinUSB driver |
| `InstallCH347Driver()` | OpenOCDInterface.cpp:639 | Install CH347 from embedded resources |
| `UninstallCH347Driver()` | OpenOCDInterface.cpp:1004 | Remove CH347 driver |

---

## 🎯 Next Session Goals

1. **Embed RS232 WinUSB Driver Files**
   - Add resource IDs
   - Update `InstallRS232Driver()` to extract from resources
   - Test automated installation

2. **Verify OpenOCD Configuration**
   - Ensure OpenOCD uses correct adapter type (RS232 vs CH347)
   - Update config files if needed

3. **Final Testing**
   - Test complete workflow on clean machine
   - Verify switching between drivers works
   - Document any remaining edge cases

---

## 💡 Design Decisions

### Why Hardware Detection Before Driver Check?
- Users don't know what card they have
- Different cards need different drivers
- Automatic detection prevents user confusion

### Why Gray Out FPGA Detection?
- Prevents errors from attempting detection without proper driver
- Forces users through correct workflow
- Reduces support burden

### Why Zadig for RS232 (Temporary)?
- WinUSB driver installation requires signed driver packages
- Zadig handles driver signing and installation reliably
- Embedding WinUSB is complex but planned for Phase 2

### Why Check Driver Status Before Everything?
- Establishes baseline state
- Enables correct button labels
- Prevents mismatched driver operations

---

## 🐛 Known Issues

### Minor Issues
- [ ] Initial status shows "Driver Install Needed" briefly before reset → **FIXED** (shows "---" now)
- [ ] CH347 driver check runs after RS232 install → **FIXED** (uses correct check now)

### Future Enhancements
- [ ] Support for multiple cards simultaneously
- [ ] Driver version comparison and upgrade prompts
- [ ] Persistent card type detection (save to config)
- [ ] Add progress bars for driver installation

---

## 📚 References

### Documentation
- [FTDI FT4232H Datasheet](https://ftdichip.com/products/ft4232h/)
- [WCH CH347 Repository](https://github.com/WCHSoftGroup/ch347)
- [Zadig USB Driver Tool](https://zadig.akeo.ie/)
- [Windows PnP API Reference](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/)

### Related Documents
- `CARD_DETECTION_RESEARCH.md` - Research findings and VID/PID info
- `IMPLEMENTATION_SUMMARY.md` - Phase 1 implementation summary
- `tools/FTDI_RS232_Driver/README.txt` - FTDI driver installation notes

---

## 👥 Contributors

- **Development**: Session work on Dec 12, 2024
- **Testing**: Ongoing with 35T hardware

---

**Last Updated**: December 12, 2024 02:53 UTC  
**Status**: ✅ Phase 1 Complete - Ready for Phase 2 (Embedding RS232 Driver)
