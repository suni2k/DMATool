# DMA Tools Setup & Testing Guide

This guide will help you set up the necessary tools to identify your 100T DMA card and extract DNA ID information.

## Prerequisites

- Windows 10/11
- Git installed
- PowerShell 7+
- USB connection to your DMA card's JTAG port

## Step 1: Download Required Tools

Run these commands in PowerShell from the `dmafiles` directory:

```powershell
cd C:\Users\suni\source\repos\DMATool\dmafiles

# Clone CH347 tools (for JTAG communication)
git clone https://github.com/WCHSoftGroup/ch347.git

# Clone PCILeech tools (DMA attack toolkit)
git clone https://github.com/ufrisk/pcileech.git

# Clone PCILeech-FPGA (firmware and configs)
git clone https://github.com/ufrisk/pcileech-fpga.git

# Clone LeechCore (core DMA library)
git clone https://github.com/ufrisk/LeechCore.git
```

## Step 2: Install CH347 Drivers

1. Navigate to `ch347\CH347FPGATool\`
2. Run `CH347FpgaDownloadTool.exe`
3. This will install the necessary USB drivers for the CH347 adapter

## Step 3: Hardware Setup

1. Connect your 100T DMA card to your PC via PCIe
2. Connect the JTAG port on the DMA card back to a USB port on the same PC using CH347 adapter
3. Power on the system

## Step 4: Identify FPGA Chip

### Using CH347 FPGA Tool (GUI Method)

1. Open `CH347FpgaDownloadTool.exe` from `ch347\CH347FPGATool\`
2. Click "Detect Device" or similar option
3. The tool should show:
   - **Chip Type**: XC7A35T, XC7A75T, or XC7A100T
   - **IDCODE**: Unique JTAG identifier
   - **Interface Type**: RS232 or CH347

### Using OpenOCD (Command Line Method)

Navigate to the OpenOCD directory:
```powershell
cd C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\OpenOCD_CH347
```

Run OpenOCD with Xilinx Artix-7 config:
```powershell
.\bin\openocd.exe -f scripts\interface\ch347.cfg -f scripts\cpld\xilinx-xc7.cfg
```

Expected output:
```
Info : JTAG tap: xc7.tap tap/device found: 0x03631093 (mfg: 0x049 (Xilinx), part: 0x3631, ver: 0x0)
```

**IDCODE to Chip Type Mapping:**
- `0x03622093` = XC7A35T
- `0x0362D093` = XC7A75T
- `0x03631093` = XC7A100T

## Step 5: Extract FPGA DNA ID

The DNA ID is a unique 57-bit identifier burned into each Xilinx FPGA during manufacturing.

### Method 1: Using OpenOCD Interactive Shell

Start OpenOCD:
```powershell
.\bin\openocd.exe -f scripts\interface\ch347.cfg -f scripts\cpld\xilinx-xc7.cfg
```

In another terminal, connect via telnet:
```powershell
telnet localhost 4444
```

Run these commands in the telnet session:
```tcl
# Initialize the JTAG chain
init

# Select the tap
targets

# Read USERCODE register (contains DNA)
irscan xc7.tap 0x08
drscan xc7.tap 32 0

# Read DNA register
irscan xc7.tap 0x17
drscan xc7.tap 57 0
```

### Method 2: Using Vivado TCL Console (if you have Vivado installed)

```tcl
open_hw_manager
connect_hw_server
open_hw_target

# Get current hardware device
current_hw_device [lindex [get_hw_devices] 0]

# Read DNA
get_property REGISTER.EFUSE.FUSE_DNA [current_hw_device]
```

### Method 3: Custom Script for Automated Extraction

Create a TCL script file `read_dna.tcl`:
```tcl
init
scan_chain

# Select XC7 device
jtag arp_init

# Read DNA sequence
irscan xc7.tap 0x17
set dna [drscan xc7.tap 57 0]

# Display DNA
puts "FPGA DNA: $dna"

# Format without 0x prefix
set dna_clean [string range $dna 2 end]
puts "DNA (formatted): $dna_clean"

shutdown
```

Run it:
```powershell
.\bin\openocd.exe -f scripts\interface\ch347.cfg -f scripts\cpld\xilinx-xc7.cfg -f read_dna.tcl
```

## Step 6: Expected DNA ID Format

Your DNA ID example: `0x003ccd8c77d04854`

**Formatted output (without 0x prefix):** `003ccd8c77d04854`

The DNA is 57 bits, typically displayed as:
- Binary: `000111100110011011000110001110111110100000100100001010100`
- Hex: `003ccd8c77d04854` (padded to 64 bits)

## Step 7: Determine Interface Type (RS232 vs CH347)

### RS232 Detection:
- Check if there's a DB9 or similar serial connector on your DMA card
- OpenOCD will show serial-based communication
- Slower speeds (typically 115200 baud max)

### CH347 Detection:
- Check if using USB-to-JTAG adapter (CH347 chip)
- OpenOCD will show `CH347` in the interface name
- Much faster (480 Mbps USB 2.0)

Run this command to verify:
```powershell
# List connected USB devices
Get-PnpDevice | Where-Object {$_.FriendlyName -like "*CH347*"}
```

Or check Device Manager → Universal Serial Bus Controllers → Look for "WCH" or "CH347"

## Expected Test Results

After running the above commands, you should have:

1. **Chip Type**: XC7A100T (confirmed via IDCODE: 0x03631093)
2. **DNA ID**: `003ccd8c77d04854` (without 0x prefix, lowercase)
3. **Interface**: CH347 (USB-based JTAG)

## Integration into C++ Application

Once you've confirmed the above works, we'll create a C++ wrapper that:

1. Uses CH347 DLL/API to communicate with JTAG
2. Sends JTAG commands to read IDCODE
3. Extracts DNA ID via USER instruction
4. Formats output as: `003ccd8c77d04854`
5. Detects chip type (35T/75T/100T)
6. Identifies interface type (RS232/CH347)

## Troubleshooting

### OpenOCD can't find device:
```
Error: unable to open ftdi device
```
**Solution**: Install CH347 drivers from `CH347FPGATool` directory

### Wrong IDCODE detected:
```
Warn : JTAG tap: xc7.tap UNEXPECTED: 0x00000000
```
**Solution**: 
- Check JTAG connections
- Verify DMA card is powered
- Try different USB port

### Permission denied:
```
Error: libusb_open() failed with LIBUSB_ERROR_ACCESS
```
**Solution**: Run OpenOCD as Administrator

## Next Steps

After successful testing:
1. Document all output results
2. Note exact commands that worked
3. Share results so we can create C++ integration code
4. Build wrapper library for your DMATool GUI

---

**Created for DMATool Project**  
Testing folder: `C:\Users\suni\source\repos\DMATool\dmafiles`
