# OpenOCD Flash Development Progress

## ? BREAKTHROUGH - OpenOCD Connection Working!

**Date:** January 2025  
**Status:** Successfully connected to CH347, now debugging flash commands

---

## ?? Critical Finding

OpenOCD requires **explicit transport selection** before JTAG operations:

```tcl
transport select jtag  # MUST come before any JTAG commands
```

### Test Results

```
PS> .\scripts\Test-OpenOCD-Simple.ps1

Info : CH347 Open Succ.  ? ? CH347 connected!
Info : clock speed 10000 kHz
Error: session transport was not selected. Use 'transport select <transport>'
Error: Transports available:
Error: jtag  ? This is what we need
Error: swd
```

**Solution:** Add `transport select jtag` to config file after adapter driver setup.

---

## ?? Updated OpenOCD Config Template

```tcl
# CH347 FPGA Flash Configuration
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd

# CRITICAL: Select JTAG transport
transport select jtag

# Set clock speed
adapter speed 10000

# Load Xilinx support
source [find cpld/xilinx-xc7.cfg]
source [find cpld/jtagspi.cfg]
```

---

## ?? Next Testing Steps

### 1. Test JTAG Chain Detection
```powershell
.\scripts\Test-OpenOCD-Simple.ps1
```

**Expected:** Should now detect FPGA on JTAG chain

### 2. Test Firmware Flash
```powershell
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t
```

**Expected:** Should program firmware to SPI flash

### 3. Verify Flash Success
- Check DMATool DNA ID tab detects FPGA
- OR use PCILeech: `pcileech.exe probe`

---

## ?? File Locations for DMATool Integration

### Source Files (for embedding)
```
C:\Users\suni\source\repos\DMATool\
??? dmafiles\ch347\CH347FPGATool\
?   ??? OpenOCD_CH347\
?   ?   ??? bin\
?   ?   ?   ??? openocd.exe
?   ?   ?   ??? libusb-1.0.dll
?   ?   ?   ??? libhidapi-0.dll
?   ?   ??? share\openocd\scripts\  ? Config files
?   ??? *.bin  ? Firmware files
```

### Target Location (after integration)
```
C:\Users\suni\source\repos\DMATool\tools\ch347\
??? openocd\
?   ??? bin\
?   ?   ??? openocd.exe
?   ?   ??? libusb-1.0.dll
?   ?   ??? libhidapi-0.dll
?   ??? scripts\  ? Required configs
?       ??? cpld\
?       ?   ??? xilinx-xc7.cfg
?       ?   ??? jtagspi.cfg
?       ??? fpga\
?           ??? xilinx-dna.cfg
??? firmware\
    ??? 002ced811686a854_ACE_75T.bin
    ??? 003ccd8c77d04854_BEEAC_100T.bin
```

---

## ?? Integration Plan

### Phase 1: Test Scripts (Current)
- ? Simple OpenOCD test
- ? Full flash test
- ? Verify test

### Phase 2: Copy to tools/ch347
```powershell
# Copy OpenOCD binaries
Copy-Item "dmafiles\ch347\CH347FPGATool\OpenOCD_CH347\bin\*" `
          "tools\ch347\openocd\bin\" -Recurse

# Copy required scripts only (to minimize size)
$scriptsNeeded = @(
    "cpld\xilinx-xc7.cfg",
    "cpld\jtagspi.cfg",
    "fpga\xilinx-dna.cfg"
)

foreach ($script in $scriptsNeeded) {
    Copy-Item "dmafiles\ch347\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\$script" `
              "tools\ch347\openocd\scripts\$script" -Force
}

# Copy firmware files
Copy-Item "dmafiles\ch347\CH347FPGATool\*.bin" `
          "tools\ch347\firmware\" -Force
```

### Phase 3: Embed in DMATool.rc
```rc
// OpenOCD executables
IDR_OPENOCD_EXE       RCDATA "..\\tools\\ch347\\openocd\\bin\\openocd.exe"
IDR_LIBUSB_DLL        RCDATA "..\\tools\\ch347\\openocd\\bin\\libusb-1.0.dll"
IDR_LIBHIDAPI_DLL     RCDATA "..\\tools\\ch347\\openocd\\bin\\libhidapi-0.dll"

// OpenOCD scripts
IDR_XILINX_XC7_CFG    RCDATA "..\\tools\\ch347\\openocd\\scripts\\cpld\\xilinx-xc7.cfg"
IDR_JTAGSPI_CFG       RCDATA "..\\tools\\ch347\\openocd\\scripts\\cpld\\jtagspi.cfg"
IDR_XILINX_DNA_CFG    RCDATA "..\\tools\\ch347\\openocd\\scripts\\fpga\\xilinx-dna.cfg"

// Firmware files (optional - can be user-provided)
IDR_FIRMWARE_75T      RCDATA "..\\tools\\ch347\\firmware\\002ced811686a854_ACE_75T.bin"
IDR_FIRMWARE_100T     RCDATA "..\\tools\\ch347\\firmware\\003ccd8c77d04854_BEEAC_100T.bin"
```

### Phase 4: FlashInterface Class
```cpp
namespace Backend {
    class FlashInterface {
    public:
        FlashInfo FlashFirmware(
            const std::string& chipModel,
            const std::string& binFilePath,
            int clockSpeedHz = 10000000,
            std::function<void(const std::string&)> logCallback = nullptr,
            std::function<void(int)> progressCallback = nullptr
        );
    private:
        std::string ExtractOpenOCD();
        std::string CreateFlashConfig(const std::string& chipModel, int clockSpeedHz);
        bool RunOpenOCD(const std::string& configFile, const std::vector<std::string>& commands);
    };
}
```

---

## ?? OpenOCD Command Reference

### Flash BIN to SPI Flash (Permanent)
```tcl
init
jtagspi_init 0 bscan_spi_xc7a75t.bit
jtagspi_program "firmware.bin" 0x0
xc7_program xc7.tap
shutdown
```

### Flash BIT to RAM (Temporary)
```tcl
init
pld load 0 "firmware.bit"
shutdown
```

### Verify Flash
```tcl
init
jtagspi_init 0 bscan_spi_xc7a75t.bit
flash verify_bank 0 "firmware.bin"
shutdown
```

### Erase Flash
```tcl
init
jtagspi_init 0 bscan_spi_xc7a75t.bit
flash erase_sector 0 0 last
shutdown
```

---

## ?? Common Issues & Solutions

### Issue 1: "transport was not selected"
**Error:**
```
Error: session transport was not selected
```

**Solution:**
```tcl
transport select jtag  # Add this line
```

### Issue 2: "Cannot find openocd scripts"
**Error:**
```
Error: unable to find cpld/xilinx-xc7.cfg
```

**Solution:**
```powershell
$env:OPENOCD_SCRIPTS = "path\to\scripts"
```

### Issue 3: "CH347 device not found"
**Solution:**
- Check driver: "USB HighSpeed-JTAG/I2C... CH347T"
- Verify VID/PID: 1A86:55DD or 1A86:55DE
- Ensure device is plugged in

### Issue 4: "JTAG scan chain empty"
**Solution:**
- Check JTAG cable connections (TDI, TDO, TCK, TMS, GND)
- Verify DMA card is powered
- Try lower clock speed: `adapter speed 5000`

### Issue 5: "Missing BSCAN bitstream files" ?? **CRITICAL**
**Error:**
```
Error: unable to find bscan_spi_xc7a75t.bit
```

**Problem:** OpenOCD needs special BSCAN bitstream files to access SPI flash through JTAG. These files are NOT included in the CH347 OpenOCD build.

**Solution - Download BSCAN files:**

1. **Download from Xilinx/AMD:**
   - Visit: https://github.com/quartiq/bscan_spi_bitstreams
   - OR generate using Xilinx Vivado (see below)

2. **Place in OpenOCD scripts directory:**
   ```
   OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\
   ??? bscan_spi_xc7a35t.bit
   ??? bscan_spi_xc7a75t.bit
   ??? bscan_spi_xc7a100t.bit
   ```

3. **Alternative: Generate with Vivado (advanced):**
   ```tcl
   # In Vivado TCL console
   source $env(XILINX_VIVADO)/data/xicom/cable_drivers/nt64/bscan_spi/create_bscan_spi.tcl
   create_bscan_spi -part xc7a75tfgg484-2
   ```

**Temporary Workaround (DNA ID only):**
- You can still detect FPGA and read DNA ID without BSCAN
- Flash programming requires BSCAN bitstreams
- See simple test script: `Test-OpenOCD-Simple.ps1`
