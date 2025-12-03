# DMA Firmware Flashing Guide

## ?? Overview
This guide covers manual FPGA firmware flashing for DMA hardware using the CH347 adapter. This is preparatory work before integrating flashing capabilities into the DMATool Flash DMA tab.

## ?? Goal
1. Understand how the CH347FpgaDownloadTool works
2. Manually flash firmware to verify the process
3. Test command-line OpenOCD flashing
4. Document the procedure for integration into DMATool

---

## ?? Quick Start - Command Line Flashing

We've created a PowerShell script to test OpenOCD command-line flashing:

### Basic Usage
```powershell
# Flash 75T firmware (default)
.\scripts\Test-FPGAFlash.ps1

# Flash 100T firmware
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a100t

# Flash custom firmware
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t -BinFile "C:\path\to\firmware.bin"

# Verify existing flash
.\scripts\Test-FPGAFlash.ps1 -VerifyOnly

# Erase flash
.\scripts\Test-FPGAFlash.ps1 -EraseOnly

# Flash at slower speed (more reliable for long cables)
.\scripts\Test-FPGAFlash.ps1 -ClockSpeed 5000000
```

### Script Features
- ? Auto-detects CH347 adapter
- ? Validates prerequisites (OpenOCD, DLLs)
- ? Progress logging with color output
- ? Automatic verification after flash
- ? Error handling and detailed logs

---

## ?? What You Have

### Hardware
- ? **75T DMA Card** (Xilinx Artix-7 XC7A75T)
- ? **CH347 USB-JTAG Adapter**
- ? **JTAG Cable** (6-pin: TDI, TDO, TCK, TMS, GND, VCC)

### Software & Files
```
C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\
??? CH347FpgaDownloadTool.exe        # GUI flash tool (Chinese)
??? 002ced811686a854_ACE_75T.bin     # Your 75T firmware
??? 003ccd8c77d04854_BEEAC_100T.bin  # 100T firmware (reference)
??? OpenOCD_CH347\
    ??? bin\
        ??? ch347.cfg                # CH347 adapter config
        ??? libusb-1.0.dll          # USB library
        ??? libhidapi-0.dll         # HID library
```

---

## ?? CH347FpgaDownloadTool Settings

### Current Working Configuration
Based on your successful flash:

| Setting | Value | Description |
|---------|-------|-------------|
| **Chip Type** | `xc7a75t` | Target FPGA chip model |
| **File Type** | `BIN` | Binary firmware file |
| **Clock Speed** | `10000000` Hz (10 MHz) | JTAG TCK frequency |
| **File Path** | `002ced811686a854_ACE_75T.bin` | Firmware binary |

### Settings Explanation

#### 1. **Chip Type Selection** (??FPGA??)
- **XC7A35T** ? 35,000 logic cells
- **XC7A75T** ? 75,000 logic cells ? **(Your chip)**
- **XC7A100T** ? 100,000 logic cells

**Important:** Must match your physical FPGA chip! Wrong selection = flash failure or bricked FPGA.

#### 2. **Download File Type** (????????)

##### Option A: **BIT File** (Temporary)
- Downloads to **FPGA RAM**
- **Volatile** - Lost on power cycle
- Fast for testing
- No permanent changes

##### Option B: **BIN File** (Permanent) ? **(What you used)**
- Downloads to **SPI Flash memory**
- **Non-volatile** - Survives power cycles
- Slower flashing process
- Permanent firmware update
- Auto-resets FPGA after flash

#### 3. **Clock Speed** (??????)
- **Default:** `10000000` Hz (10 MHz) ? **(Recommended)**
- **Faster:** `15000000` Hz (15 MHz) - risky
- **Faster:** `30000000` Hz (30 MHz) - very risky

**Lower = More reliable**. Keep at 10 MHz unless you have signal integrity issues (use slower like 5 MHz).

---

## ?? Manual Flashing Procedure

### Step 1: Hardware Connections

#### JTAG Pin Mapping
```
CH347 JTAG Port ? DMA Card JTAG Header
???????????????????????????????????
TDI   (Pin 1)   ?  TDI
TDO   (Pin 2)   ?  TDO
TCK   (Pin 3)   ?  TCK
TMS   (Pin 4)   ?  TMS
GND   (Pin 5)   ?  GND
VCC   (Pin 6)   ?  3.3V (optional, for power indication)
```

**Connection Checklist:**
- [ ] CH347 plugged into USB port
- [ ] JTAG cable connected to CH347
- [ ] JTAG cable connected to DMA card JTAG header
- [ ] DMA card powered (via PCIe or external power)
- [ ] CH347 driver installed (check Device Manager)

### Step 2: Verify CH347 Driver

**Check Device Manager:**
1. Open Device Manager (`devmgmt.msc`)
2. Look for: **"USB HighSpeed-JTAG/I2C... CH347T"**
   - Under "Universal Serial Bus controllers"
3. If shows **"USB to UART+JTAG"** ? **Wrong driver!**
   - Uninstall and reinstall correct driver

**Correct Driver:**
- **Name:** USB HighSpeed-JTAG/I2C... CH347T
- **VID/PID:** 1A86:55DD or 1A86:55DE
- **Version:** 2.5.2024.03 or newer

### Step 3: Launch Flash Tool

```powershell
cd "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
.\CH347FpgaDownloadTool.exe
```

**GUI Interface (Chinese):**
```
???????????????????????????????????????
? CH347 FPGA ????                  ?
???????????????????????????????????????
? ??FPGA??: [xc7a75t      ?]      ?
? ????????:                    ?
?   ? BIT?????? (RAM)            ?
?   ? BIN?????? (Flash) ? ?    ?
? ??????: [10000000    Hz]      ?
? [??]                               ?
? [??????] ? 002ced811686a854... ?
? [??] ? Click to start             ?
?                                      ?
? ??????????????????????????????????? ?
? ? Progress: ?????????????  60%    ? ?
? ??????????????????????????????????? ?
?                                      ?
? ????:                            ?
? [INFO] CH347 device connected       ?
? [INFO] Erasing flash...             ?
? [INFO] Programming...               ?
? [SUCCESS] Flash complete!           ?
? [??] ? Clear log                  ?
???????????????????????????????????????
```

### Step 4: Configure Flash Settings

1. **Select FPGA Model:**
   - Click dropdown (??FPGA??)
   - Type or select: `xc7a75t`

2. **Select File Type:**
   - Select radio button: **BIN??????**
   - (Not BIT - that's temporary RAM only)

3. **Set Clock Speed:**
   - Keep default: `10000000` Hz
   - (Can type custom value if needed)

4. **Choose Firmware File:**
   - Click **[??????]**
   - Navigate to: `C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\`
   - Select: `002ced811686a854_ACE_75T.bin`
   - Click **Open**

### Step 5: Flash Firmware

1. **Double-check settings:**
   - ? Chip: `xc7a75t`
   - ? Mode: `BIN??????`
   - ? Clock: `10000000` Hz
   - ? File: `002ced811686a854_ACE_75T.bin`

2. **Start Flashing:**
   - Click **[??]** (Download)
   - Progress bar appears
   - Wait for completion (2-5 minutes typical)

3. **Monitor Progress:**
   - Watch log output in black box
   - Progress bar shows percentage
   - Tool will auto-reset FPGA when done

### Step 6: Verify Flash Success

**Success Indicators:**
```
[INFO] Erasing flash sector...
[INFO] Programming flash @ 0x000000
[INFO] Programming flash @ 0x010000
[INFO] Programming flash @ 0x020000
...
[INFO] Verifying flash...
[SUCCESS] Flash programming complete!
[INFO] Resetting FPGA...
[SUCCESS] Device is now running new firmware
```

**If you see errors:**
- Check JTAG cable connections
- Verify correct chip model selected
- Try lower clock speed (e.g., 5000000 Hz)
- Check driver is correct version
- Ensure DMA card is powered

### Step 7: Test Flashed Firmware

**Option A: Use PCILeech (if installed)**
```powershell
cd C:\Tools\PCILeech
.\pcileech.exe probe
```

**Expected:**
```
DEVICE: FPGA: Artix-7 XC7A75T
FIRMWARE: 002ced811686a854_ACE_75T
STATUS: Connected
```

**Option B: Use DMATool DNA ID Tab**
1. Open DMATool
2. Go to DNA ID tab
3. Click "Detect FPGA & Read DNA"
4. Should detect XC7A75T with your DNA ID

---

## ?? Understanding the Flashing Process

### What Happens During BIN Flash?

#### 1. **Initialization**
```
OpenOCD connects to CH347
CH347 initializes JTAG chain
Detects FPGA (XC7A75T)
Reads IDCODE: 0x1362D093 ? Confirms chip
```

#### 2. **JTAG SPI Flash Access**
```
OpenOCD uses JTAG to access SPI flash controller
Enters programming mode
Sets up flash parameters (manufacturer, size, etc.)
```

#### 3. **Flash Erase**
```
Erases necessary sectors (slow - 30-60 seconds)
Sector size: typically 64KB
Verifies erase by reading back 0xFF
```

#### 4. **Flash Programming**
```
Programs BIN file in chunks (e.g., 256 bytes)
Each chunk: Write ? Verify
Progress updates every sector
Typical speed: 50-100 KB/s
```

#### 5. **Verification**
```
Reads back entire flash contents
Compares with original BIN file
CRC check (if enabled)
```

#### 6. **FPGA Reset**
```
Issues JTAG reset command
FPGA loads new firmware from flash
FPGA starts running (should enumerate on PCIe)
```

### OpenOCD Commands (Behind the Scenes)

The CH347 tool essentially runs these OpenOCD commands:

```tcl
# 1. Initialize adapter
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd
adapter speed 10000000

# 2. Load configurations
source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]

# 3. Initialize JTAG chain
init

# 4. Detect FPGA
scan_chain

# 5. Program flash
jtagspi_init 0 bscan_spi_xc7a75t.bit
jtagspi_program 002ced811686a854_ACE_75T.bin 0x0

# 6. Reset and exit
xc7_program xc7.tap
shutdown
```

---

## ?? Flash Memory Layout

### Typical SPI Flash Organization (XC7A75T)

```
????????????????????????????????????????
? Address Range    ? Content           ?
????????????????????????????????????????
? 0x000000-0x0FFFFF ? Primary Firmware  ? ? Your BIN goes here
? 0x100000-0x1FFFFF ? Backup/Recovery   ? (Optional)
? 0x200000-0x2FFFFF ? Configuration     ? (Optional)
? 0x300000-0x3FFFFF ? Reserved          ?
????????????????????????????????????????
Total: 4MB typical (W25Q32 flash chip)
```

**Important:**
- Flashing overwrites **0x000000** onwards
- Size: ~1-2MB typical for 75T firmware
- Flash chip: Usually Winbond W25Q32 (32Mbit = 4MB)

---

## ?? Troubleshooting

### Issue 1: "Device Not Found" or "No CH347 Detected"

**Causes:**
- CH347 not plugged in
- Wrong driver installed
- USB port issue

**Solutions:**
```powershell
# Check device in Device Manager
devmgmt.msc

# Verify correct driver
# Should see: "USB HighSpeed-JTAG/I2C... CH347T"

# If wrong, reinstall driver:
cd "C:\Users\suni\source\repos\DMATool\tools\ch347\drivers"
# Run: pnputil /add-driver CH341WDM.INF /install
```

### Issue 2: "JTAG Chain Not Detected" or "No FPGA Found"

**Causes:**
- JTAG cable not connected
- Wrong pinout
- DMA card not powered
- Bad cable/connector

**Solutions:**
1. Check physical connections (see Step 1)
2. Verify DMA card power (LED should be on)
3. Try different JTAG cable
4. Use multimeter to check continuity on TDI/TDO/TCK/TMS

### Issue 3: "Flash Programming Failed" or "Verify Error"

**Causes:**
- Signal integrity issues (cable too long, noisy environment)
- Clock speed too high
- Faulty flash chip
- Wrong BIN file

**Solutions:**
1. Lower clock speed to 5 MHz or 1 MHz
2. Shorten JTAG cable (< 20cm recommended)
3. Try a different BIN file to test
4. Check that BIN file is for correct chip (75T vs 100T)

### Issue 4: "Wrong Chip Selected" Error

**Causes:**
- Selected xc7a100t but actual chip is xc7a75t
- IDCODE mismatch

**Solutions:**
```
XC7A35T  ? IDCODE: 0x13622093
XC7A75T  ? IDCODE: 0x1362D093  ? Your chip
XC7A100T ? IDCODE: 0x13631093

# Tool auto-detects from IDCODE
# But you must manually select correct model in dropdown
```

### Issue 5: Tool is in Chinese - Can't Find Settings

**Quick Translation:**
```
??FPGA??          ? Select FPGA Model
????????      ? Select Download File Type
  BIT??????     ?   BIT File Mode (RAM)
  BIN??????     ?   BIN File Mode (Flash) ?
??????          ? Select Hardware Clock
??                  ? About
??????          ? Select Download File
??                  ? Download (START)
??                  ? Clear Screen
????              ? Device Information
```

---

## ?? Testing Procedure

### Before Integrating into DMATool

1. **Test Manual Flash (You've Done This ?)**
   - Used CH347FpgaDownloadTool.exe
   - Flashed 002ced811686a854_ACE_75T.bin
   - Verified success

2. **Test Command-Line Flash (Next Step)**
   - Find OpenOCD executable (might need to extract from tool)
   - Create test script using OpenOCD commands
   - Verify same result as GUI tool

3. **Document Command-Line Process**
   - Exact OpenOCD commands needed
   - Required config files
   - Expected output/errors

4. **Plan DMATool Integration**
   - Embed OpenOCD (if not already)
   - Create FlashInterface class (similar to OpenOCDInterface)
   - Add UI to Flash DMA tab
   - Implement progress callbacks

---

## ?? Next Steps for DMATool Integration

### 1. Find/Extract OpenOCD from CH347 Tool

**Hypothesis:** OpenOCD might be embedded in `CH347FpgaDownloadTool.exe`

**Check:**
```powershell
# Search for openocd.exe in tool directory
cd "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
Get-ChildItem -Recurse -Filter "openocd.exe"

# If not found, might be embedded in .exe
# Use tool like "Resource Hacker" or "7-Zip" to extract
```

**Alternative:** Use the OpenOCD we already have from DNA ID tab:
- We've embedded `openocd.exe` in DMATool.rc
- Already have `libusb-1.0.dll`, `libhidapi-0.dll`
- Just need flash-specific config files

### 2. Create OpenOCD Flash Config

**File:** `xilinx-flash-75t.cfg`
```tcl
# CH347 adapter configuration
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd

# Set JTAG clock speed (10 MHz)
adapter speed 10000000

# Load Xilinx 7-series support
source [find cpld/xilinx-xc7.cfg]

# Load JTAG SPI flash support
source [find cpld/jtagspi.cfg]

# Initialize
init

# This config can be extended with flash programming commands
```

### 3. Test Command-Line Flashing

**Script:** `test-flash.ps1`
```powershell
# Test OpenOCD flash programming

$openocd = "C:\path\to\openocd.exe"
$binFile = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\002ced811686a854_ACE_75T.bin"
$configFile = "xilinx-flash-75t.cfg"

# Run OpenOCD with flash programming commands
& $openocd -f $configFile -c "init" `
           -c "jtagspi_init 0 bscan_spi_xc7a75t.bit" `
           -c "jtagspi_program $binFile 0x0" `
           -c "xc7_program xc7.tap" `
           -c "shutdown"

# Check exit code
if ($LASTEXITCODE -eq 0) {
    Write-Host "[SUCCESS] Flash programming complete!" -ForegroundColor Green
} else {
    Write-Host "[ERROR] Flash programming failed!" -ForegroundColor Red
}
```

### 4. Create FlashInterface Class

**File:** `src/Backend/FlashInterface.h`
```cpp
#pragma once
#include <string>
#include <functional>

namespace Backend
{
    struct FlashInfo
    {
        bool success = false;
        std::string chipModel;
        std::string firmwareFile;
        size_t totalBytes = 0;
        size_t bytesWritten = 0;
        int progressPercent = 0;
        std::string statusMessage;
    };

    class FlashInterface
    {
    public:
        // Flash BIN file to FPGA SPI flash
        FlashInfo FlashFirmware(
            const std::string& chipModel,  // e.g., "xc7a75t"
            const std::string& binFilePath,
            int clockSpeedHz = 10000000,   // Default 10 MHz
            std::function<void(const std::string&)> logCallback = nullptr,
            std::function<void(int)> progressCallback = nullptr
        );

        // Flash BIT file to FPGA RAM (temporary)
        FlashInfo FlashBitstream(
            const std::string& chipModel,
            const std::string& bitFilePath,
            int clockSpeedHz = 10000000,
            std::function<void(const std::string&)> logCallback = nullptr
        );

        // Verify flash contents
        bool VerifyFlash(
            const std::string& binFilePath,
            std::function<void(const std::string&)> logCallback = nullptr
        );

        // Erase flash
        bool EraseFlash(
            std::function<void(const std::string&)> logCallback = nullptr
        );

    private:
        std::string ExtractOpenOCD();
        std::string CreateFlashConfig(const std::string& chipModel, int clockSpeedHz);
        bool RunOpenOCD(const std::string& configFile, const std::string& commands);
    };
}
```

### 5. Plan Flash DMA Tab UI

**Layout:**
```
???????????????????????????????????????????????????????
? Flash DMA                                           ?
???????????????????????????????????????????????????????
? ??????????????????????  ??????????????????????????? ?
? ? Firmware Settings  ?  ? Flash Progress          ? ?
? ??????????????????????  ??????????????????????????? ?
? ? Chip Model:        ?  ? Status: Ready           ? ?
? ? [xc7a75t    ?]     ?  ?                         ? ?
? ?                    ?  ? Progress:               ? ?
? ? Flash Mode:        ?  ? ?????????????  60%      ? ?
? ? ? BIT (RAM)        ?  ?                         ? ?
? ? ? BIN (Flash) ?   ?  ? Speed: 75 KB/s          ? ?
? ?                    ?  ? Time Left: 1m 30s       ? ?
? ? Clock Speed:       ?  ?                         ? ?
? ? [10000000  ] Hz    ?  ? Bytes: 1.5 MB / 2.1 MB  ? ?
? ?                    ?  ??????????????????????????? ?
? ? Firmware File:     ?                              ?
? ? 002ced...75T.bin   ?  ??????????????????????????? ?
? ? [Browse...]        ?  ? Flash Operations        ? ?
? ?                    ?  ??????????????????????????? ?
? ? [Flash Firmware]   ?  ? [Verify Flash]          ? ?
? ? [Verify Only]      ?  ? [Erase Flash]           ? ?
? ??????????????????????  ? [Backup Flash]          ? ?
?                         ??????????????????????????? ?
???????????????????????????????????????????????????????
? Status & Log                                        ?
? ??????????????????????????????????????????????????? ?
? [INFO] Ready to flash firmware                      ?
? [INFO] Click "Browse" to select BIN file            ?
? [INFO] Ensure DMA is connected via JTAG             ?
???????????????????????????????????????????????????????
```

---

## ?? Reference Materials

### OpenOCD Documentation
- **Official Docs:** https://openocd.org/doc/html/
- **JTAG SPI Flash:** https://openocd.org/doc/html/Flash-Commands.html
- **Xilinx Support:** https://openocd.org/doc/html/CPU-Configuration.html

### CH347 Resources
- **GitHub:** https://github.com/WCHSoftGroup/ch347
- **Download Tool:** https://github.com/WCHSoftGroup/ch347/tree/main/CH347FPGADownloader

### Xilinx FPGA
- **7-Series Config:** https://www.xilinx.com/support/documentation/user_guides/ug470_7Series_Config.pdf
- **SPI Flash Programming:** https://www.xilinx.com/support/answers/46913.html

---

## ? Completion Checklist

### Manual Testing
- [x] Hardware connected (CH347 + DMA + JTAG)
- [x] Driver installed (USB HighSpeed-JTAG)
- [x] Flashed firmware using CH347FpgaDownloadTool.exe
- [x] Verified flash success
- [ ] Test command-line OpenOCD flash ? **NEXT STEP**

### Command-Line Testing
- [ ] Test PowerShell flash script (`Test-FPGAFlash.ps1`)
- [ ] Verify successful flash via command line
- [ ] Test verify-only mode
- [ ] Test erase-only mode
- [ ] Test different clock speeds (reliability)
- [ ] Document exact commands and output

### DMATool Integration Planning
- [x] Design FlashInterface API (documented in guide)
- [x] Plan Flash DMA tab UI layout (documented in guide)
- [x] Create flash config templates (in PowerShell script)
- [ ] Implement FlashInterface class
- [ ] Add progress callbacks
- [ ] Add file browser for BIN/BIT files
- [ ] Add chip model selector
- [ ] Add verify/erase operations
- [ ] Integrate into build system

### Documentation
- [x] Manual flash procedure documented
- [x] Hardware setup documented
- [x] Troubleshooting guide created
- [x] PowerShell test script created
- [x] Quick reference guide created
- [ ] Command-line process documented (after testing)
- [ ] Integration guide created (after testing)

---

## ?? What You've Learned

1. **CH347 flashing uses OpenOCD under the hood**
   - JTAG interface to FPGA
   - SPI flash access via JTAG
   - Config files for different chips

2. **BIN vs BIT flashing:**
   - **BIN** ? Permanent (SPI flash)
   - **BIT** ? Temporary (FPGA RAM)

3. **Critical settings:**
   - Chip model must match physical FPGA
   - Clock speed affects reliability
   - JTAG connections must be solid

4. **Flash process:**
   - Erase ? Program ? Verify ? Reset

---

## ?? NEXT STEPS - DO THIS NOW

### Immediate Action: Test Command-Line Flashing

```powershell
# Open PowerShell as Administrator
cd C:\Users\suni\source\repos\DMATool

# Run the test script
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t

# This will:
# 1. Check prerequisites
# 2. Detect CH347 adapter
# 3. Flash your 75T firmware
# 4. Verify the flash
# 5. Report success/failure
```

### Expected Results

? **If Successful:**
- Script shows green "[SUCCESS]" messages
- Flash completes in 2-3 minutes
- Verification passes
- DMATool DNA ID tab can detect FPGA
- Ready to integrate into DMATool!

? **If Failed:**
- Check error messages in output
- Review logs in `$env:TEMP\openocd_*.log`
- Try slower clock: `-ClockSpeed 5000000`
- Verify JTAG connections
- Check driver installation

### After Testing

**If successful, update this file with:**
1. Actual test output (copy/paste from PowerShell)
2. Any issues encountered
3. Solutions that worked
4. Timing information (how long did flash take?)

Then proceed to integrate into DMATool Flash DMA tab!

---

## ?? Support

If you encounter issues:
1. Check [Troubleshooting](#??-troubleshooting) section
2. Verify hardware connections
3. Try lower clock speed
4. Join Discord: https://discord.gg/MfH9UHxkdP

---

**Status:** ? Manual flash tested and working  
**Next:** Test command-line OpenOCD flashing  
**Last Updated:** January 2025
