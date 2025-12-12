# FTDI Quad RS232-HS (FT4232H) Driver Files for 35T DMA Card

This folder contains both driver sets for the FTDI Quad RS232-HS device (VID_0403 & PID_6011).

## Device Information
- **Device:** Quad RS232-HS (FT4232H chip)
- **VID/PID:** VID_0403 & PID_6011
- **Interfaces:** 4 independent UART channels (Interface 0, 1, 2, 3)
- **Primary Use:** Interface 0 for JTAG programming of 35T FPGA

## Directory Structure

```
FTDI_RS232_Driver/
├── README.md
├── FTDIBUS_Driver/          # Original FTDI COM port driver
│   ├── ftdibus.inf
│   ├── ftdibus.cat
│   ├── ftdibus.sys
│   ├── ftser2k.sys
│   ├── ftbusui.dll
│   └── ftd2xx.dll
└── WinUSB_Driver/           # WinUSB driver for JTAG access
    ├── ftdi_winusb.inf
    ├── ftdi_winusb.cat
    └── WinUSBCoInstaller2.dll
```

## Driver Files Overview

### FTDIBUS_Driver/ - Original COM Port Driver
**Purpose:** Creates virtual COM ports for serial communication  
**Provider:** FTDI  
**Version:** 2.12.36.20  

**Files:**
- `ftdibus.inf` (33 KB) - Driver installation file
- `ftdibus.cat` (26 KB) - Security catalog  
- `ftdibus.sys` (152 KB) - Main FTDI bus driver
- `ftser2k.sys` (101 KB) - Serial port driver
- `ftbusui.dll` (170 KB) - User interface library
- `ftd2xx.dll` (808 KB) - D2XX API library

**When Installed:**
- Interface 0, 1, 2, 3 all appear as COM ports (e.g., COM8, COM9, COM10, COM11)
- Used for serial communication and debugging
- Standard FTDI driver behavior

### WinUSB_Driver/ - Direct USB Access
**Purpose:** Provides direct USB access for JTAG programming  
**Provider:** libwdi (via Zadig)  
**Version:** 6.1.7600.16385  

**Files:**
- `ftdi_winusb.inf` (4 KB) - WinUSB installation file (for Interface 0 only)
- `ftdi_winusb.cat` (5 KB) - Security catalog
- `WinUSBCoInstaller2.dll` (1002 KB) - CoInstaller for WinUSB

**System Files (already in Windows):**
- `C:\Windows\System32\drivers\winusb.sys` - WinUSB kernel driver
- `C:\Windows\System32\WinUSBCoInstaller2.dll` - CoInstaller

**When Installed:**
- Interface 0 becomes "Quad RS232-HS (Interface 0)" in Device Manager under "Universal Serial Bus devices"
- Interface 1, 2, 3 remain as COM ports with FTDIBUS driver
- Required for OpenOCD JTAG programming

## Usage in DMATool

### For 35T FPGA Programming:
1. **Install WinUSB** on Interface 0 (required for JTAG/OpenOCD)
2. Keep FTDIBUS on Interfaces 1, 2, 3 (optional, for serial debugging)

### Driver Management Functions Needed:
- **CheckDriver()** - Detect which driver is installed on Interface 0
- **InstallWinUSB()** - Replace FTDIBUS with WinUSB on Interface 0
- **RestoreFTDIBUS()** - Restore original FTDIBUS driver on Interface 0

## Technical Notes

### VID/PID Matching:
- **Device ID for Interface 0:** `USB\VID_0403&PID_6011&MI_00`
- **Device ID for Interface 1:** `USB\VID_0403&PID_6011&MI_01`
- **Device ID for Interface 2:** `USB\VID_0403&PID_6011&MI_02`
- **Device ID for Interface 3:** `USB\VID_0403&PID_6011&MI_03`

### WinUSB INF Customization:
The `ftdi_winusb.inf` file is specifically configured for Interface 0 only:
- DeviceID = "VID_0403&PID_6011&MI_00"
- This ensures only Interface 0 gets WinUSB, leaving others with FTDIBUS

### Windows Compatibility:
- **FTDIBUS:** Works on Windows 7, 8, 8.1, 10, 11, Server 2008 R2+
- **WinUSB:** Built into Windows Vista and later (10, 11 fully supported)

## Comparison with 75T/100T (FT601 Driver)

| Feature | 35T (FT4232H) | 75T/100T (FT601) |
|---------|---------------|------------------|
| VID/PID | 0403:6011 | 0403:601F |
| Interfaces | 4 x UART | 1 x SuperSpeed FIFO |
| Driver for DMA | WinUSB on Interface 0 | WinUSB or D3XX |
| Driver Files | ftdibus.inf + ftdi_winusb.inf | FTD3XXWU.inf |
| Use Case | JTAG programming | High-speed DMA |

## Next Steps for DMATool Integration

1. Create `FT4232DriverInterface.h` and `FT4232DriverInterface.cpp`
2. Embed INF/CAT files as resources (similar to FT601)
3. Implement driver detection for Interface 0 specifically
4. Add UI tab for 35T device management
5. Integrate OpenOCD JTAG flashing functionality

---
**Created:** 2025-12-12  
**Source:** Extracted from Windows driver store after Zadig installation
