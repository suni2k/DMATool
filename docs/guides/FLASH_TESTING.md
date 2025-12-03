# Quick Flash Testing - Command Reference

## ?? Testing Command-Line Flashing

Before integrating into DMATool, test the OpenOCD flashing process manually.

### Prerequisites Check
```powershell
# 1. Verify CH347 is connected
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*CH347*" }

# Should show:
# Status: OK
# Class: USBDevice
# FriendlyName: USB HighSpeed-JTAG/I2C... CH347T

# 2. Check driver version
Get-PnpDevice -FriendlyName "*CH347*" | Get-PnpDeviceProperty -KeyName "DEVPKEY_Device_DriverVersion"

# 3. Verify OpenOCD exists
Test-Path "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\OpenOCD_CH347\bin\openocd.exe"
```

---

## ?? Flash Testing Commands

### Test 1: Flash Your 75T Firmware
```powershell
cd C:\Users\suni\source\repos\DMATool

# Run flash script with default 75T firmware
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t

# Expected output:
# [INFO] Checking prerequisites...
# [SUCCESS] Prerequisites OK
# [INFO] Checking CH347 adapter connection...
# [SUCCESS] CH347 device found: USB HighSpeed-JTAG...
# [INFO] Using default firmware for xc7a75t: 002ced811686a854_ACE_75T.bin
# [INFO] Firmware file size: 1.87 MB
# [INFO] Starting flash programming...
# [INFO] Chip: xc7a75t
# [INFO] File: 002ced811686a854_ACE_75T.bin
# [INFO] Clock: 10000000 Hz
# [INFO] Executing OpenOCD...
# [SUCCESS] Flash programming completed successfully!
# [INFO] Verifying programmed flash...
# [SUCCESS] Flash verification passed!
# [SUCCESS] Operation completed successfully!
```

### Test 2: Verify Existing Flash
```powershell
# Verify flash contents match your firmware file
.\scripts\Test-FPGAFlash.ps1 -VerifyOnly

# This reads back flash and compares with BIN file
# Useful after manual flash with CH347FpgaDownloadTool
```

### Test 3: Slower Clock Speed (If Errors)
```powershell
# If you get flash errors, try slower clock
.\scripts\Test-FPGAFlash.ps1 -ClockSpeed 5000000  # 5 MHz

# Or even slower for very noisy environments
.\scripts\Test-FPGAFlash.ps1 -ClockSpeed 1000000  # 1 MHz
```

### Test 4: Erase Flash
```powershell
# Completely erase flash (WARNING: FPGA won't boot after this until reflashed)
.\scripts\Test-FPGAFlash.ps1 -EraseOnly

# You'll see:
# [WARNING] This may take 30-60 seconds...
# [SUCCESS] Flash erase completed!
```

---

## ?? Understanding Output

### Successful Flash Output
```
========================================
 OpenOCD FPGA Flash Test Script        
========================================

[10:30:15] [INFO] Checking prerequisites...
[10:30:15] [SUCCESS] Prerequisites OK
[10:30:15] [INFO] Checking CH347 adapter connection...
[10:30:16] [SUCCESS] CH347 device found: USB HighSpeed-JTAG/I2C... CH347T
[10:30:16] [INFO] Using default firmware for xc7a75t: 002ced811686a854_ACE_75T.bin
[10:30:16] [INFO] Firmware file size: 1.87 MB
[10:30:16] [INFO] Created OpenOCD config: C:\Users\...\flash_config.cfg
[10:30:16] [INFO] Starting flash programming...
[10:30:16] [INFO] Chip: xc7a75t
[10:30:16] [INFO] File: 002ced811686a854_ACE_75T.bin
[10:30:16] [INFO] Clock: 10000000 Hz
[10:30:16] [INFO] Executing OpenOCD...

OpenOCD Output:
Open On-Chip Debugger 0.12.0
Licensed under GNU GPL v2
adapter speed: 10000 kHz

Info : CH347 device found (VID:PID = 1A86:55DD)
Info : clock speed 10000 kHz
Info : JTAG tap: xc7.tap tap/device found: 0x1362d093 (mfg: 0x049 (Xilinx), part: 0x362d, ver: 0x1)
Warn : gdb services need one or more targets defined
Info : Found flash device 'win w25q32' (ID 0x001640ef)
Info : Flash size: 4 MiB
Info : Erasing flash...
Info : sector 0 at 0x00000000 erased
Info : sector 1 at 0x00010000 erased
...
Info : Programming flash...
Info : Wrote 1966080 bytes from file 002ced811686a854_ACE_75T.bin
Info : Resetting target

[10:32:45] [SUCCESS] Flash programming completed successfully!
[10:32:45] [INFO] Verifying programmed flash...
[10:32:50] [SUCCESS] Flash verification passed!

========================================
[10:32:50] [SUCCESS] Operation completed successfully!
```

### Error Output Examples

#### Error: CH347 Not Connected
```
[ERROR] CH347 device not found or not working
[ERROR] Please check:
[ERROR]   1. CH347 is plugged into USB port
[ERROR]   2. Driver is installed (USB HighSpeed-JTAG)
[ERROR]   3. Device shows in Device Manager
```

#### Error: JTAG Chain Not Detected
```
OpenOCD Output:
Error: JTAG scan chain interrogation failed: all ones
Error: Check JTAG interface, timings, target power, etc.
Error: Trying to use configured scan chain anyway...

[ERROR] Flash programming failed with exit code: 1
```
**Solution:** Check JTAG cable connections (TDI, TDO, TCK, TMS, GND)

#### Error: Flash Programming Failed
```
Info : Found flash device 'win w25q32' (ID 0x001640ef)
Error: Flash write failed at address 0x00010000
Error: embedded:startup.tcl:26: Error

[ERROR] Flash programming failed with exit code: 1
```
**Solution:** Try slower clock speed: `-ClockSpeed 5000000`

---

## ?? Testing Workflow

### Complete Test Sequence
```powershell
# Step 1: Check hardware
Get-PnpDevice | Where-Object { $_.FriendlyName -like "*CH347*" }

# Step 2: Flash firmware
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t

# Step 3: Verify flash
.\scripts\Test-FPGAFlash.ps1 -VerifyOnly

# Step 4: Test with DMATool DNA ID tab
# Open DMATool ? DNA ID tab ? "Detect FPGA & Read DNA"
# Should detect XC7A75T successfully

# Step 5: Test with PCILeech (if installed)
cd C:\Tools\PCILeech
.\pcileech.exe probe

# Should show:
# DEVICE: FPGA: Artix-7 XC7A75T
# FIRMWARE: 002ced811686a854_ACE_75T
# STATUS: Connected
```

---

## ?? Log Files

Detailed logs are saved to temp folder:

```powershell
# View OpenOCD output log
Get-Content "$env:TEMP\openocd_output.log"

# View error log
Get-Content "$env:TEMP\openocd_error.log"

# Open temp folder
explorer $env:TEMP
```

---

## ? Advanced Usage

### Custom Firmware Path
```powershell
# Flash custom firmware from different location
$customFirmware = "D:\Downloads\my_custom_75t.bin"
.\scripts\Test-FPGAFlash.ps1 -BinFile $customFirmware -ChipModel xc7a75t
```

### Multiple FPGAs on Same PC
```powershell
# If you have multiple DMA cards, flash them sequentially:

# Flash first DMA (75T)
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t -BinFile "firmware_card1.bin"

# Swap JTAG cable to second DMA

# Flash second DMA (100T)
.\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a100t -BinFile "firmware_card2.bin"
```

### Automated Testing Loop
```powershell
# Test flash reliability - flash 5 times and verify each
1..5 | ForEach-Object {
    Write-Host "`n=== Test $_ of 5 ===" -ForegroundColor Yellow
    
    .\scripts\Test-FPGAFlash.ps1 -ChipModel xc7a75t
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Flash test $_ FAILED!" -ForegroundColor Red
        break
    } else {
        Write-Host "Flash test $_ PASSED!" -ForegroundColor Green
    }
    
    Start-Sleep -Seconds 2
}
```

---

## ?? Troubleshooting Script Issues

### Issue: "Execution Policy Restriction"
```powershell
# Error: cannot be loaded because running scripts is disabled
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

# Try again
.\scripts\Test-FPGAFlash.ps1
```

### Issue: "OpenOCD Not Found"
```powershell
# Check if OpenOCD exists
$openocdPath = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\OpenOCD_CH347\bin\openocd.exe"
Test-Path $openocdPath

# If False, verify CH347FPGATool directory structure
Get-ChildItem "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool" -Recurse -Filter "*.exe"
```

### Issue: "Missing BSCAN Bitstream"
```powershell
# Error: bscan_spi_xc7a75t.bit not found
# This file should be in OpenOCD installation

# Check if it exists in OpenOCD share folder
Get-ChildItem "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\OpenOCD_CH347" -Recurse -Filter "bscan*.bit"

# If missing, you may need to extract from full OpenOCD distribution
# Download from: https://github.com/openocd-org/openocd/releases
```

---

## ? Success Checklist

After running `Test-FPGAFlash.ps1`, verify:

- [ ] Script exits with code 0 (success)
- [ ] "[SUCCESS] Flash programming completed successfully!" message shown
- [ ] "[SUCCESS] Flash verification passed!" message shown
- [ ] No errors in `$env:TEMP\openocd_error.log`
- [ ] DMATool DNA ID tab detects FPGA correctly
- [ ] PCILeech probe command works (if PCILeech installed)

---

## ?? Next Steps

Once command-line flashing works:

1. ? Document exact OpenOCD commands needed
2. ? Verify script works reliably (test 5-10 times)
3. ?? Create `FlashInterface` class in DMATool backend
4. ?? Implement Flash DMA tab UI
5. ?? Add progress callbacks for real-time updates
6. ?? Integrate into DMATool build

---

**Status:** Ready for testing  
**Command:** `.\scripts\Test-FPGAFlash.ps1`  
**Next:** Run the script and report results!
