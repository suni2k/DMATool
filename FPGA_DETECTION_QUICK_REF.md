# FPGA Detection - Quick Reference Card

## ? **Status: WORKING**

### Quick Test
```powershell
# 1. Build DMATool
# 2. Run DMATool.exe
# 3. Click "JTAG Port" tab
# 4. Click "Detect FPGA & Read DNA"
# 5. Should see: XC7A75T detected with DNA ID
```

---

## Expected Success Output
```
[SUCCESS] FPGA detected: XC7A75T
[INFO] IDCODE: 0x13632093
[INFO] DNA ID: 014ccde79de9218c  (example)
[INFO] Logic Cells: 75,520
```

---

## Common Issues & Fixes

| Issue | Quick Fix |
|-------|-----------|
| `CH347_Read failure` | Unplug/replug CH347 USB cable |
| No FPGA detected | Check JTAG cable connection |
| Wrong chip (50T) | Already fixed - should show 75T |
| No DNA extracted | Already fixed - `xilinx-dna.cfg` now sourced |
| OpenOCD not found | Rebuild - resources should embed |
| 50+ openocd.exe processes | Run `scripts/Kill-OpenOCD-Admin.ps1` as admin |

---

## IDCODE Reference (Your DMA Cards)

| IDCODE | Chip Report | Actual Chip | Reason |
|--------|-------------|-------------|---------|
| `0x13632093` | XC7A50T | **XC7A75T** | Remarked chip |
| `0x1362D093` | XC7A75T | **XC7A75T** | Standard |

**Note**: Both map to 75T in DMATool (no 50T DMA cards exist)

---

## Hardware Checklist (30 seconds)

Quick check before debugging:
1. ? CH347 USB plugged in
2. ? JTAG cable connected
3. ? FPGA board has power
4. ? No other apps using CH347

---

## Emergency Recovery

### CH347 Not Responding
```powershell
# Option 1: Quick reset
1. Unplug CH347 USB
2. Wait 5 seconds
3. Plug back in

# Option 2: Script reset (as Admin)
.\scripts\Reset-CH347-Adapter.ps1
```

### Too Many OpenOCD Processes
```powershell
# As Administrator
.\scripts\Kill-OpenOCD-Admin.ps1
```

### Clean Temp Directory
```powershell
Remove-Item "$env:TEMP\DMATool" -Recurse -Force
# Re-run DMATool to re-extract
```

---

## File Locations

### Embedded Resources (in DMATool.exe)
- `openocd.exe` - JTAG debugger
- `libusb-1.0.dll` - USB library
- `libhidapi-0.dll` - HID library
- `xilinx-dna-347.cfg` - CH347 + DNA config
- `xilinx-dna.cfg` - DNA extraction procedures
- `xilinx-xc7.cfg` - Xilinx 7-series chip config
- `jtagspi.cfg` - JTAG SPI config

### Runtime Extraction (temp directory)
```
C:\Users\<user>\AppData\Local\Temp\DMATool\
??? openocd.exe
??? *.dll
??? cpld\
    ??? *.cfg
```

---

## Diagnostic Commands

### Check CH347 Device
```powershell
Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'}
```
**Expected**: `USB HighSpeed-JTAG/I2C... CH347T` with Status `OK`

### Test OpenOCD Manually
```powershell
$env:OPENOCD_SCRIPTS = "$env:TEMP\DMATool"
& "$env:TEMP\DMATool\openocd.exe" `
    -c "source [find cpld/xilinx-dna-347.cfg]" `
    -c "source [find cpld/xilinx-dna.cfg]" `
    -c "init" `
    -c "set dna [xc7_get_dna xc7.tap]" `
    -c "xilinx_print_dna `$dna" `
    -c "shutdown"
```

### Check for Orphaned Processes
```powershell
Get-Process openocd -ErrorAction SilentlyContinue
```
**Expected**: Nothing (or 1 if actively detecting)

---

## Version Info

**OpenOCD Version**: 0.11.0+dev-00706-g822097a35-dirty (2022-09-29-16:46)  
**Adapter**: CH347 (VID:0x1a86 PID:0x55dd)  
**Clock Speed**: 10 MHz (10000 kHz)  
**Supported Chips**: XC7A35T, XC7A75T, XC7A100T

---

## Documentation Quick Links

| Topic | File |
|-------|------|
| Complete fix summary | `docs/FPGA_DETECTION_COMPLETE_FIX_SUMMARY.md` |
| IDCODE aliasing | `docs/XC7A50T_75T_IDCODE_ALIASING.md` |
| CH347 errors | `docs/CH347_COMMUNICATION_ERROR_GUIDE.md` |
| Process cleanup | `docs/OPENOCD_PROCESS_LEAK_FIX.md` |
| Config paths | `docs/OPENOCD_CONFIG_CPLD_PATH_FIX.md` |

---

## What Changed (For Reference)

### Key Fixes
1. ? Extract configs to `cpld/` subdirectory
2. ? Set `OPENOCD_SCRIPTS` environment variable
3. ? Source `xilinx-dna.cfg` for DNA commands
4. ? Map 50T IDCODE ? 75T chip model
5. ? Fix process cleanup (no more orphans)

### Files Modified
- `src/Backend/OpenOCDInterface.cpp` - Main fixes
- `src/Backend/OpenOCDInterface.h` - Already had enums

### Scripts Created
- `scripts/Kill-OpenOCD-Admin.ps1` - Process cleanup
- `scripts/Reset-CH347-Adapter.ps1` - Hardware reset
- `scripts/Test-OpenOCD-Manual.ps1` - Diagnostics

---

## Success Criteria

? FPGA detected as **XC7A75T** (not 50T)  
? IDCODE displayed correctly  
? **DNA ID extracted** (16-digit hex)  
? Logic cells shown (75,520)  
? No orphaned OpenOCD processes  
? Works on other PCs (embedded resources)  

---

**Last Updated**: 2025-12-02  
**Status**: ? Production Ready  
**Tested**: Working with XC7A75T DMA cards
