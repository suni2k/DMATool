# ?? MANUAL STEP REQUIRED: Update DMATool.rc

## Current Status
? All code changes complete and building successfully  
?? **DMATool.rc needs manual update to embed driver files**

## What to Do

### Step 1: Open DMATool.rc
Open the file `DMATool.rc` in Visual Studio or a text editor.

### Step 2: Add FT601 Resources
Add the following lines **at the end of the file**, after the CH347 driver entries:

```rc
// FT601 Driver Files
IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"
IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"
```

### Step 3: Complete File Should Look Like

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

// FT601 Driver Files  <-- ADD THESE TWO LINES
IDR_FT601_INF RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.Inf"
IDR_FT601_CAT RCDATA "dmafiles\\Winusb_D3XX_Release_1.4.0.1\\WU_FTD3XX_Driver\\FTD3XXWU.cat"
```

### Step 4: Save and Rebuild
1. Save `DMATool.rc`
2. In Visual Studio: `Build > Rebuild Solution`
3. Wait for build to complete

### Step 5: Verify
After rebuild:
- ? Check .exe size increased by ~27 KB
- ? No build errors
- ? Ready to test driver installation

## Why Manual Update?

The `.rc` file couldn't be edited automatically because:
- Visual Studio locks the file when it's open
- RC files have special formatting requirements
- Manual editing is safer for resource files

## What Happens After Update?

Once you rebuild with the updated `DMATool.rc`:

1. **Driver files embedded**: FT601 INF and CAT files are now part of the .exe
2. **No external dependencies**: Users don't need the `dmafiles` folder
3. **Installation works**: "Install FT601 Driver" button will extract and install
4. **Detection works**: Properly distinguishes correct vs wrong driver

## Testing After Rebuild

Test these scenarios:

### 1. Wrong Driver Installed
```
Expected Output:
[INFO] Checking FT601 driver status...
[WARNING] Wrong FT601 driver is installed
[INFO] Current: FTDI SuperSpeed-FIFO Bridge
[INFO] Expected: FTDI FT601 USB 3.0 Bridge Device
```

### 2. Install Driver
```
Expected Output:
[INFO] Installing FT601 driver...
[INFO] FT601 driver files extracted to: C:\Users\...\Temp\DMATool_FT601_Driver
[SUCCESS] FT601 driver installation initiated
```

### 3. Correct Driver Installed
```
Expected Output:
[INFO] Checking FT601 driver status...
[SUCCESS] FT601 driver is installed
[INFO] Device: FTDI FT601 USB 3.0 Bridge Device
```

## Summary of All Changes

### ? Code Changes (Complete)
1. `src/resource.h` - Added FT601 resource IDs
2. `src/Backend/FT601DriverInterface.h` - Resource extraction and driver detection
3. `src/Backend/FT601DriverInterface.cpp` - Implementation
4. `src/UI/Tabs/DataPortTab.cpp` - UI updates for driver status

### ?? Manual Changes (Required)
1. `DMATool.rc` - Add FT601 resource entries (see above)

### ?? Documentation (Complete)
1. `docs/FT601_DRIVER_EMBEDDING.md` - Embedding instructions
2. `docs/FT601_DRIVER_MANAGEMENT_COMPLETE.md` - Complete implementation guide
3. `docs/MANUAL_RC_UPDATE_REQUIRED.md` - This file

## Questions?

If you encounter issues:
1. Make sure the driver files exist at the specified path
2. Check that paths use double backslashes (`\\`)
3. Verify resource IDs match between `.rc` and `resource.h`
4. Rebuild solution (not just build)

## Build Status

Current: ? **Building successfully** (without FT601 resources)  
After RC update: ? **Will build with embedded FT601 drivers**

---

**Ready to proceed?** Update `DMATool.rc` and rebuild!
