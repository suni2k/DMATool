# FT601 Driver Embedding Instructions

## Step 1: Update DMATool.rc

Open `DMATool.rc` in a text editor and add the following lines at the end, before the closing of the file:

```rc
// FT601 Driver Files
IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"
IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"
```

## Complete DMATool.rc File

```rc
#include "src\resource.h"

IDR_OPENOCD_EXE RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\openocd.exe"
IDR_CH347_CFG RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\ch347.cfg"
IDR_LIBUSB_DLL RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\libusb-1.0.dll"
IDR_LIBHIDAPI_DLL RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\libhidapi-0.dll"
IDR_XILINX_DNA_347_CFG RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\xilinx-dna-347.cfg"
IDR_XILINX_XC7_CFG RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\xilinx-xc7.cfg"
IDR_JTAGSPI_CFG RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\jtagspi.cfg"
IDR_XILINX_DNA_CFG RCDATA "dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\xilinx-dna.cfg"

// CH341/CH347 Driver Files
IDR_CH341_INF RCDATA "tools\\ch347\\drivers\\CH341WDM.INF"
IDR_CH341_SYS RCDATA "tools\\ch347\\drivers\\CH341WDM.SYS"
IDR_CH341_M64_SYS RCDATA "tools\\ch347\\drivers\\CH341M64.SYS"
IDR_CH341_W64_SYS RCDATA "tools\\ch347\\drivers\\CH341W64.SYS"
IDR_CH341_CAT RCDATA "tools\\ch347\\drivers\\CH341WDM.CAT"
IDR_CH347_DLL RCDATA "tools\\ch347\\drivers\\CH347DLL.DLL"
IDR_CH347_DLL_A64 RCDATA "tools\\ch347\\drivers\\CH347DLLA64.DLL"

// FT601 Driver Files
IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"
IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"
```

## What This Does

1. **Embeds driver files into .exe**: The FT601 driver INF and CAT files are now embedded as resources
2. **No external files needed**: Users don't need the `dmafiles` folder
3. **Automatic extraction**: The driver files are extracted to a temp folder when installing
4. **Cleanup**: Temp files are removed after installation completes

## Driver Detection Improvements

The updated code now:

1. **Detects wrong driver**: If "FTDI SuperSpeed-FIFO Bridge" is detected, shows "Wrong Driver"
2. **Detects correct driver**: If "FT601 USB 3.0 Bridge Device" is detected, shows "Installed"
3. **Shows actionable messages**: Tells users to uninstall wrong driver first

## Expected Console Output

### With Correct Driver
```
[INFO] Checking FT601 driver status...
[SUCCESS] FT601 driver is installed
[INFO] Device: FTDI FT601 USB 3.0 Bridge Device
[INFO] Version: 1.4.0.1
[INFO] VID/PID: VID_0403 / PID_601F
```

### With Wrong Driver
```
[INFO] Checking FT601 driver status...
[WARNING] Wrong FT601 driver is installed
[INFO] Current: FTDI SuperSpeed-FIFO Bridge
[INFO] Expected: FTDI FT601 USB 3.0 Bridge Device
[INFO] Action: Uninstall wrong driver, then install correct driver
```

### Not Installed
```
[INFO] Checking FT601 driver status...
[WARNING] FT601 driver is not installed
[INFO] Device may show as 'FTDI SuperSpeed-FIFO Bridge'
```

### Install Process
```
[INFO] Installing FT601 driver...
[INFO] FT601 driver files extracted to: C:\Users\...\Temp\DMATool_FT601_Driver
[SUCCESS] FT601 driver installation initiated
[INFO] Please follow UAC prompts if they appear
[INFO] Cleaned up temporary driver files
[SUCCESS] Driver installed successfully
```

## Files Modified

1. `src/resource.h` - Added `IDR_FT601_INF` and `IDR_FT601_CAT` resource IDs
2. `src/Backend/FT601DriverInterface.h` - Added resource extraction and driver detection
3. `src/Backend/FT601DriverInterface.cpp` - Implemented embedded resource extraction
4. `src/UI/Tabs/DataPortTab.cpp` - Updated UI to show wrong driver vs not installed
5. `DMATool.rc` - **NEEDS MANUAL UPDATE** (add FT601 resource entries)

## Next Steps

1. Manually update `DMATool.rc` with the FT601 entries
2. Rebuild the project
3. Test the following scenarios:
   - Check driver with wrong driver installed
   - Uninstall wrong driver
   - Check driver when not installed
   - Install correct driver
   - Check driver with correct driver installed
