# DNA ID Tab - Complete Implementation Guide

## Overview
The DNA ID tab provides automated FPGA detection and unique DNA ID extraction for DMA hardware via JTAG interface.

## ? Features Implemented

### ?? **Core Functionality**
- ? **Automatic FPGA Detection** - Detects Xilinx Artix-7 FPGAs (XC7A35T, XC7A75T, XC7A100T)
- ? **DNA ID Extraction** - Reads 57-bit unique device identifier
- ? **Smart Driver Checking** - Validates driver before attempting detection
- ? **Auto-Detection on Startup** - Runs once when tab is first opened
- ? **Manual Detection** - User-triggered detection via button
- ? **Copy to Clipboard** - One-click DNA ID copying

### ?? **Adapter Support**

#### **CH347 USB-JTAG Adapter** ? **FULLY TESTED**
- **VID/PID:** `1A86:55DD` or `1A86:55DE`
- **Driver:** USB HighSpeed-JTAG/I2C... CH347T
- **Status:** ? Working perfectly
- **Features:**
  - Automatic driver installation
  - Automatic driver uninstallation
  - Smart driver validation
  - Embedded driver files (no external dependencies)

#### **RS232/FTDI Adapter** ?? **PARTIALLY IMPLEMENTED**
- **VID/PID:** `0403:6011`
- **Driver:** FTDI D2XX drivers
- **Status:** ?? Code implemented, not tested (no hardware available)
- **TODO:**
  - Test with actual RS232/FTDI hardware
  - Implement FTDI driver installation (if needed)
  - Validate OpenOCD configuration

### ?? **Smart Auto-Detection Flow**

```
Tool Startup
    ?
Wait 2 frames (UI stabilization)
    ?
Start Auto-Detection
    ?
[1] Check Driver Status (PowerShell - 500ms)
    ?
Is driver correct?
    ?? YES: "USB HighSpeed-JTAG/I2C... CH347T"
    ?   ?
    ?   [2] Search for OpenOCD
    ?   ?
    ?   [3] Detect JTAG Adapter (CH347 or FTDI)
    ?   ?
    ?   [4] Execute OpenOCD Commands
    ?   ?
    ?   [5] Parse FPGA IDCODE
    ?   ?
    ?   [6] Extract DNA ID
    ?   ?
    ?   ? SUCCESS: Display results
    ?
    ?? NO: "USB to UART+JTAG" (Wrong Driver)
    ?   ?
    ?   ? SKIP FPGA Detection
    ?   ?
    ?   Show Installation Instructions:
    ?   1. Uninstall wrong driver
    ?   2. Install correct driver
    ?   3. Retry detection
    ?
    ?? NO DRIVER: Not installed
        ?
        ? SKIP FPGA Detection
        ?
        Show Installation Instructions:
        1. Click "Install CH347 Driver"
        2. Approve UAC prompt
        3. Retry detection
```

### ?? **UI Components**

#### **Left Panel: FPGA Device Information**
```
???????????????????????????????????????
? FPGA Device Information             ?
???????????????????????????????????????
? FPGA Details                        ?
?   Chip Model:      XC7A75T          ?
?   Adapter:         CH347            ?
?   Manufacturer:    Xilinx           ?
?   Family:          Artix-7          ?
?                                     ?
? DNA ID                              ?
?   Unique DNA:      00542417dc636678 ?
?                                     ?
? ??????????????????????????????????? ?
? ?  Detect FPGA & Read DNA         ? ?
? ??????????????????????????????????? ?
? ??????????????????????????????????? ?
? ?  Copy DNA to Clipboard          ? ?
? ??????????????????????????????????? ?
???????????????????????????????????????
```

#### **Right Panel: JTAG Driver Information**
```
???????????????????????????????????????
? JTAG Driver Information             ?
???????????????????????????????????????
? Driver Status                       ?
?   Status:   Installed ?             ?
?   Version:  2.5.2024.03             ?
?   Adapter:  CH347                   ?
?   Device:   USB HighSpeed-JTAG...  ?
?   VID/PID:  1A86:55DD               ?
?                                     ?
? Management                          ?
? ??????????????????????????????????? ?
? ?  Check Driver Status            ? ?
? ??????????????????????????????????? ?
? ??????????????????????????????????? ?
? ?  Install CH347 Driver           ? ?
? ??????????????????????????????????? ?
? ??????????????????????????????????? ?
? ?  Uninstall CH347 Driver         ? ?
? ??????????????????????????????????? ?
???????????????????????????????????????
```

#### **Bottom Panel: Status & Log**
```
???????????????????????????????????????????????????????????
? Status & Log                                            ?
???????????????????????????????????????????????????????????
? Connection: Connected     Detection: Detected           ?
? Last Operation: Auto-Detection                          ?
???????????????????????????????????????????????????????????
? [SUCCESS] FPGA detected: XC7A75T                       ?
? [INFO] DNA ID: 00542417dc636678                        ?
? [INFO] Version: 2.5.2024.03                            ?
???????????????????????????????????????????????????????????
```

### ?? **Visual Feedback**

#### **Floating Progress Notification**
```
??????????????????????????????????????
?                                    ?
?        Scanning Device             ?
?      ?????????????????             ?
?   Checking driver status...        ?
?                                    ?
??????????????????????????????????????
```
- **Overlay:** Semi-transparent black (dims background)
- **Position:** Centered on screen
- **Style:** Rounded corners, blue border
- **Content:** Dynamic progress messages

### ?? **Auto-Refresh After Operations**

After **Install Driver** or **Uninstall Driver**:
1. Wait 1 second (Windows driver update delay)
2. Automatically re-check driver status
3. Update UI panels with new information
4. Log results to console

**Console Output Example:**
```
[INFO] Installing CH347 driver...
[SUCCESS] Driver added to Windows driver store
[SUCCESS] Device driver updated successfully
[INFO] Re-checking driver status...
[SUCCESS] Driver status updated - Driver is installed
[INFO] Device: USB HighSpeed-JTAG/I2C... CH347T
[INFO] Version: 2.5.2024.03
```

## ?? **File Structure**

```
DMATool/
?? src/
?  ?? Backend/
?  ?  ?? OpenOCDInterface.h        # FPGA detection & driver management
?  ?  ?? OpenOCDInterface.cpp
?  ?? UI/
?  ?  ?? Tabs/
?  ?     ?? JTAGPortTab.h          # DNA ID tab UI
?  ?     ?? JTAGPortTab.cpp
?  ?? resource.h                   # Resource IDs for embedded files
?? tools/
?  ?? ch347/
?     ?? drivers/                  # CH347 driver files
?        ?? CH341WDM.INF
?        ?? CH341WDM.SYS
?        ?? CH341M64.SYS
?        ?? CH341W64.SYS
?        ?? CH341WDM.CAT
?        ?? CH341DLL.DLL           # ? Was missing, now included
?        ?? CH341DLLA64.DLL        # ? Was missing, now included
?        ?? CH347DLL.DLL
?        ?? CH347DLLA64.DLL
?? DMATool.rc                      # Embedded resources (OpenOCD, configs, drivers)
```

## ?? **Issues Resolved**

### **1. Driver Installation Failed** ? ? ?
**Problem:** Missing `CH341DLL.DLL` and `CH341DLLA64.DLL` files
- pnputil reported: "The system cannot find the file specified"
- INF file referenced DLL files that weren't in our driver folder

**Solution:**
- Copied complete driver package from `C:\Users\suni\Desktop\dma\75T-driver`
- Now includes ALL required files
- Driver installation works with one UAC prompt

### **2. Wrong Driver Detection** ? ? ?
**Problem:** Tool couldn't detect if wrong driver was installed
- Users had "USB to UART+JTAG" (serial driver)
- Tool tried to detect FPGA anyway (wasted time, showed confusing errors)

**Solution:**
- Smart driver check BEFORE FPGA detection
- Validates driver name contains "HighSpeed-JTAG"
- Skips detection and shows clear instructions if wrong driver

### **3. No Auto-Refresh After Driver Changes** ? ? ?
**Problem:** After install/uninstall, user had to manually click "Check Driver Status"

**Solution:**
- Automatic driver status refresh after install/uninstall
- 1-second delay for Windows to update
- Auto-updates UI panels and logs results

### **4. Auto-Detection Runs Every Tab Switch** ? ? ?
**Problem:** Switching tabs triggered unnecessary detections

**Solution:**
- `s_HasAutoDetected` static flag - tracks if EVER auto-detected
- Only runs ONCE when tab is first opened
- Manual detection still available via button

### **5. Manual Download Required** ? ? ?
**Problem:** Users had to manually download drivers from WCH website

**Solution:**
- Complete driver package copied to `tools/ch347/drivers/`
- Tool copies to temp and runs pnputil automatically
- No external downloads needed

## ?? **Testing Status**

### **CH347 Adapter** ? **FULLY TESTED**
- ? Driver installation (automatic via pnputil)
- ? Driver uninstallation (automatic via pnputil)
- ? Driver status checking
- ? FPGA detection (XC7A75T confirmed)
- ? DNA ID extraction (57-bit unique ID)
- ? Wrong driver detection ("USB to UART+JTAG")
- ? Auto-detection on startup
- ? Manual detection button
- ? Copy DNA to clipboard
- ? Auto-refresh after driver changes

### **RS232/FTDI Adapter** ?? **NOT TESTED**
- ?? Code implemented but not tested (no hardware)
- ?? OpenOCD configuration exists (from working tool)
- ?? Driver management not implemented yet
- ? Unknown if FTDI drivers need special installation

**TODO for RS232/FTDI:**
1. Test with actual RS232/FTDI hardware
2. Verify FTDI driver detection
3. Implement FTDI driver installation (if needed)
4. Test FPGA detection and DNA extraction
5. Validate OpenOCD pin configuration

## ?? **Technical Details**

### **OpenOCD Configuration**

#### **CH347 Adapter**
```tcl
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
adapter speed 10000
source xilinx-dna-347.cfg
source xilinx-xc7.cfg
source jtagspi.cfg
init
set dna [xc7_get_dna xc7.tap]
xilinx_print_dna $dna
shutdown
```

#### **RS232/FTDI Adapter**
```tcl
interface ftdi
ftdi_vid_pid 0x0403 0x6011
ftdi_channel 0
ftdi_layout_init 0x0098 0x008b
reset_config none
adapter_khz 10000
source xilinx-dna.cfg
source xilinx-xc7.cfg
source jtagspi.cfg
init
set dna [xc7_get_dna xc7.tap]
xilinx_print_dna $dna
shutdown
```

### **Embedded Resources**
All required files embedded in `DMATool.exe`:
- `openocd.exe` - JTAG interface tool
- `libusb-1.0.dll` - USB library
- `libhidapi-0.dll` - HID library
- `ch347.cfg` - CH347 configuration
- `xilinx-dna-347.cfg` - DNA extraction (CH347)
- `xilinx-dna.cfg` - DNA extraction (FTDI)
- `xilinx-xc7.cfg` - Xilinx 7-series support
- `jtagspi.cfg` - JTAG SPI flash support

**Driver files NOT embedded** (copied from `tools/ch347/drivers/` to temp):
- All `.INF`, `.SYS`, `.CAT`, `.DLL` files
- Reason: Better for development and updates
- Future: Could embed if full standalone distribution needed

### **FPGA IDCODE Mapping**
```cpp
0x13622093 ? XC7A35T   (33,280 logic cells)
0x1362D093 ? XC7A75T   (75,520 logic cells)
0x13631093 ? XC7A100T  (101,440 logic cells)
```

## ?? **User Guide**

### **First Time Setup (CH347)**
1. Plug in CH347 adapter to USB port
2. Connect JTAG cable to DMA card (TDI, TDO, TCK, TMS, GND)
3. Power on DMA card
4. Run DMATool.exe **as Administrator** (recommended)
5. Tool auto-detects and shows driver status

**If wrong driver installed:**
1. Click "Uninstall CH347 Driver"
2. Approve UAC prompt
3. Wait for uninstall confirmation
4. Click "Install CH347 Driver"
5. Approve UAC prompt
6. Wait for installation confirmation
7. Click "Detect FPGA & Read DNA"

**If no driver installed:**
1. Click "Install CH347 Driver"
2. Approve UAC prompt
3. Wait for installation confirmation
4. Click "Detect FPGA & Read DNA"

### **Normal Usage**
1. Tool auto-detects on startup (if driver correct)
2. DNA ID displayed automatically
3. Click "Copy DNA to Clipboard" to copy ID
4. Use DNA ID for DMA firmware or licensing

### **Manual Detection**
- Click "Detect FPGA & Read DNA" anytime to re-scan
- Useful after:
  - Changing JTAG cables
  - Power cycling DMA card
  - Switching between different DMA cards

## ?? **Next Steps**

### **DNA ID Tab** ? **COMPLETE (CH347)**
- ? All features implemented and tested
- ?? RS232/FTDI support needs testing (no hardware)

### **Flash DMA Tab** ?? **TODO**
- Programming FPGA firmware via JTAG
- Bitstream upload
- Flash memory operations

### **Benchmark DMA Tab** ?? **TODO**
- DMA read/write speed tests
- Memory throughput benchmarks
- Performance analysis

## ?? **Support & Community**
- **Website:** https://www.dmakings.com
- **Discord:** https://discord.gg/MfH9UHxkdP
- **Setup Guide:** https://injectkings.gitbook.io/dma-kings

---

**Status:** ? DNA ID Tab - Feature Complete (CH347 Tested)
**Last Updated:** January 2025
**Version:** 1.0
