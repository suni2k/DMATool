# Flash Detection Not Working - Troubleshooting

## Issue
"Detect FPGA" works perfectly and detects XC7A75T, but "Detect Flash Device" fails with:
```
[PROGRESS] CH347 detected! Probing JTAG chain...
[PROGRESS] Failed to detect flash device
```

## Root Cause

The Flash detection tries to load BSCAN bitstreams, but they're not being found. The code looks for:
```
.\dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a75t.bit
```

## Quick Fix

Run this PowerShell command to check if BSCAN files exist:

```powershell
Get-ChildItem ".\dmafiles\CH347FPGATool\OpenOCD_CH347" -Recurse -Filter "bscan*.bit" | Select-Object FullName
```

If no files found, you need to copy them from the full OpenOCD installation.

## Temporary Workaround

Since "Detect FPGA" works and we know you have an XC7A75T, you can:

1. **Select chip manually**: In the Flash tab, disable "Auto-detect chip model" and select "XC7A75T" from dropdown
2. **Browse firmware**: Click "Browse..." and select your `.bin` file  
3. **Flash directly**: Click "Program Firmware"

The flash will work even without detection because:
- We know the chip model (XC7A75T)
- We have the firmware file
- The BSCAN file will be loaded during flash operation

## Files Needed

The Flash detection needs these files:
```
dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\
??? bscan_spi_xc7a35t.bit
??? bscan_spi_xc7a50t.bit
??? bscan_spi_xc7a75t.bit  ? This one for your card
??? bscan_spi_xc7a100t.bit
??? ... (other chip variants)
```

## Next Steps

1. Try manual chip selection + flash
2. If that works, flash detection is just a convenience feature
3. We can fix detection later by copying BSCAN files

The important part (actual flashing) should still work!
