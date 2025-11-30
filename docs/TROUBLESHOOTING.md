# DMATool - Troubleshooting Guide

## Common Issues & Solutions

### Driver Installation Issues

#### ? "Failed to add driver package: The system cannot find the file specified"

**Symptoms:**
```
[ERROR] pnputil failed with exit code: 2
[INFO] This may mean the driver is incompatible or already installed
```

**Root Cause:**
- Missing `CH341DLL.DLL` or `CH341DLLA64.DLL` files
- INF file references files that don't exist in driver folder

**Solution:**
1. Ensure complete driver package in `tools/ch347/drivers/`:
   ```
   tools/ch347/drivers/
   ?? CH341WDM.INF
   ?? CH341WDM.SYS
   ?? CH341M64.SYS
   ?? CH341W64.SYS
   ?? CH341WDM.CAT
   ?? CH341DLL.DLL          ? Must have!
   ?? CH341DLLA64.DLL       ? Must have!
   ?? CH347DLL.DLL
   ?? CH347DLLA64.DLL
   ```

2. If files missing, copy from complete driver package:
   ```
   Source: C:\Users\[You]\Desktop\dma\75T-driver\Driver Installer (open)\
   Dest:   C:\Users\[You]\source\repos\DMATool\tools\ch347\drivers\
   ```

3. Rebuild and try again

---

#### ? "User cancelled UAC prompt"

**Symptoms:**
```
[ERROR] Failed to run pnputil (error: 1223)
[INFO] User cancelled UAC prompt
```

**Root Cause:**
- User clicked "No" on UAC elevation prompt
- Driver installation requires administrator privileges

**Solution:**
1. Click "Install CH347 Driver" again
2. Click "Yes" on UAC prompt
3. **Alternative:** Run DMATool as Administrator:
   - Right-click `DMATool.exe` ? "Run as administrator"

---

#### ? "Wrong CH347 driver detected: USB to UART+JTAG"

**Symptoms:**
```
[WARNING] Wrong CH347 driver detected: USB to UART+JTAG
[ERROR] Cannot detect FPGA with current driver
[INFO] Expected: USB HighSpeed-JTAG/I2C... CH347T
[INFO] Current: USB to UART+JTAG
```

**Root Cause:**
- Serial/UART driver installed instead of JTAG driver
- This is a different driver for the same VID/PID

**Solution:**
1. Click "Uninstall CH347 Driver"
2. Approve UAC prompt
3. Wait for "Driver uninstalled" message
4. Click "Install CH347 Driver"
5. Approve UAC prompt again
6. Wait for "Driver installed" message
7. Click "Detect FPGA & Read DNA"

---

#### ? "Driver update failed - device may need manual driver installation"

**Symptoms:**
```
[WARNING] Driver update failed - device may need manual driver installation
[INFO] Try: Device Manager ? Right-click device ? Update driver
```

**Root Cause:**
- Windows blocked automatic driver update
- Device in use by another program
- Driver signature verification failed

**Solution (Manual Installation):**
1. Open Device Manager (`devmgmt.msc`)
2. Find device under:
   - "Universal Serial Bus controllers"
   - "Other devices" (if not recognized)
3. Look for:
   - "USB to UART+JTAG" (wrong driver)
   - "Unknown Device" (no driver)
   - "CH347" (any variant)
4. Right-click device ? "Update driver"
5. Choose "Browse my computer for drivers"
6. Click "Browse" and navigate to:
   ```
   C:\Users\[You]\AppData\Local\Temp\DMATool\drivers\
   ```
7. Click "Next" and wait for installation
8. Restart DMATool and click "Detect FPGA & Read DNA"

---

### FPGA Detection Issues

#### ? "No JTAG adapter detected (CH347 or FTDI)"

**Symptoms:**
```
[ERROR] No JTAG adapter detected (CH347 or FTDI)
```

**Root Cause:**
- Adapter not plugged in
- Driver not installed
- USB port issue

**Solution:**
1. Verify CH347 adapter is plugged into USB port
2. Check Device Manager for CH347 device
3. Try different USB port (USB 3.0 preferred)
4. Install driver if not detected:
   ```
   Click "Install CH347 Driver"
   ```

---

#### ? "Failed to detect FPGA" (after successful driver install)

**Symptoms:**
```
[ERROR] Failed to detect FPGA
[INFO] Possible issues:
[INFO]   - JTAG cable not connected to DMA card
[INFO]   - DMA card not powered on
[INFO]   - Wrong JTAG pins (check TDI, TDO, TCK, TMS, GND)
```

**Root Causes & Solutions:**

**1. JTAG Cable Not Connected**
- **Check:** 6-pin JTAG cable from CH347 to DMA card
- **Verify:** Connector fully seated on both ends

**2. DMA Card Not Powered**
- **Check:** DMA card has power (LED should be on)
- **Verify:** PCIe power connector (if applicable)
- **Try:** Re-seat card in PCIe slot

**3. Wrong JTAG Pins**
```
Correct JTAG Pin Mapping:
???????????????????????????????
? CH347    ?    DMA Card      ?
???????????????????????????????
? TDI      ?    TDI           ?
? TDO      ?    TDO           ?
? TCK      ?    TCK           ?
? TMS      ?    TMS           ?
? GND      ?    GND           ?
? VCC      ?    (optional)    ?
???????????????????????????????
```
- **Check:** Cable orientation (notch/key alignment)
- **Verify:** Pin 1 indicator on both connectors

**4. Run as Administrator**
```
Right-click DMATool.exe ? "Run as administrator"
```

---

#### ? "Error: unable to open ftdi device" (RS232/FTDI)

**Symptoms:**
```
Error: unable to open ftdi device with description '*', serial '*' at bus location '*'
```

**Root Cause:**
- FTDI adapter detected but OpenOCD can't access it
- FTDI D2XX driver not installed
- Wrong FTDI driver version

**Solution:**
1. Install FTDI D2XX drivers from: https://ftdichip.com/drivers/d2xx-drivers/
2. Choose "setup executable" for Windows
3. Run installer
4. Restart DMATool
5. Click "Detect FPGA & Read DNA"

**Note:** RS232/FTDI support is implemented but **not tested** (no hardware available)

---

### UI Issues

#### ? "Window doesn't appear" or "Black screen"

**Root Cause:**
- OpenGL initialization failed
- Graphics driver outdated

**Solution:**
1. Update graphics drivers:
   - **NVIDIA:** GeForce Experience or nvidia.com
   - **AMD:** AMD Software or amd.com
   - **Intel:** Intel Driver & Support Assistant
2. Restart computer
3. Try running DMATool again

---

#### ? "Floating notification stuck on screen"

**Symptoms:**
- Progress popup doesn't disappear
- UI frozen/unresponsive

**Root Cause:**
- Long-running operation (driver install/FPGA detect)
- Application hung

**Solution:**
1. **Wait:** Some operations take 10-30 seconds
2. **Check Console:** Look for error messages
3. **Force Close:** If truly frozen (5+ minutes), close via Task Manager

---

### Build Issues

#### ? "Cannot open include file: 'imgui.h'"

**Solution:**
See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) ? "Issue 1"

#### ? "LNK1104: cannot open file 'glfw3.lib'"

**Solution:**
See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) ? "Issue 2"

#### ? "RC.exe not found"

**Solution:**
See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) ? "Issue 3"

---

## Diagnostic Information

### Collecting Logs

**Console Output:**
- DMATool shows all operations in console window
- Copy output for troubleshooting:
  1. Right-click console window title bar
  2. Edit ? Select All
  3. Edit ? Copy
  4. Paste into text file

**Example Useful Output:**
```
[INFO] Auto-detecting FPGA and adapter...
[INFO] Checking CH347 driver status...
[SUCCESS] Correct CH347 JTAG driver detected
[INFO] Device: USB HighSpeed-JTAG/I2C... CH347T
[INFO] Version: 2.5.2024.03
[INFO] OpenOCD found at: C:\Users\...\Temp\DMATool\openocd.exe
[INFO] Detected adapter: CH347
[INFO] Executing OpenOCD...
[SUCCESS] FPGA detected: XC7A75T
[INFO] DNA ID: 00542417dc636678
```

### System Information

**Check Driver Status (PowerShell):**
```powershell
Get-PnpDevice | Where-Object {$_.FriendlyName -like '*CH347*'} | 
    Select-Object FriendlyName, Status, DriverVersion
```

**Expected Output:**
```
FriendlyName                         Status DriverVersion
------------                         ------ -------------
USB HighSpeed-JTAG/I2C... CH347T     OK     2.5.2024.03
```

**Check USB Devices:**
```powershell
Get-PnpDevice -Class USB | Where-Object {$_.Status -eq "OK"} | 
    Select-Object FriendlyName, InstanceId
```

**Find CH347 VID/PID:**
```powershell
Get-PnpDeviceProperty -InstanceId "USB\VID_1A86&PID_55DD*" | 
    Where-Object {$_.KeyName -like '*HardwareID*'}
```

---

## Known Limitations

### RS232/FTDI Adapter
- ? Code implemented
- ? Not tested (no hardware)
- ? Driver installation method unknown
- ? Pin configuration not validated

**If you have RS232/FTDI hardware:**
Please report issues to: https://discord.gg/MfH9UHxkdP

### Multiple FPGAs
- ? Not supported yet
- Only detects first FPGA in JTAG chain
- **Future Enhancement:** Multi-device detection

### Driver Auto-Update
- ? Not implemented
- Manual re-install required for driver updates
- **Future Enhancement:** Check for newer versions

---

## Getting Help

### Before Asking for Help

**Gather this information:**
1. **DMATool Version:** Check console or About dialog
2. **Operating System:** Windows 10/11, build number
3. **Adapter Type:** CH347 or RS232/FTDI
4. **Driver Status:** 
   ```
   Click "Check Driver Status" and copy output
   ```
5. **Console Output:** Copy entire console log
6. **FPGA Model:** XC7A35T, XC7A75T, or XC7A100T
7. **Error Messages:** Exact error text from UI or console

### Support Channels

**Discord (Fastest):**
- Join: https://discord.gg/MfH9UHxkdP
- Channel: `#dma-tool-support`
- Response time: Usually < 1 hour

**GitHub Issues:**
- Create issue: https://github.com/akwanmn/DMATool/issues
- Use template provided
- Include diagnostic information above

**Setup Guide:**
- Documentation: https://injectkings.gitbook.io/dma-kings
- Step-by-step tutorials
- Video guides

---

## FAQ

### Q: Do I need to run DMATool as Administrator?
**A:** Not required, but recommended:
- ? Reduces UAC prompts
- ? Better driver access
- ? Not needed if drivers already installed

### Q: Can I use DMATool without driver installation?
**A:** No, correct driver required for FPGA detection:
- ? "USB to UART+JTAG" driver ? Won't work
- ? "USB HighSpeed-JTAG/I2C... CH347T" ? Works

### Q: Why does auto-detection take 3 seconds?
**A:** Normal timing:
- Driver check: ~500ms
- OpenOCD extraction: ~100ms
- FPGA detection: ~2-3 seconds
- **Total: ~3 seconds**

### Q: Can I detect FPGA without CH347 adapter?
**A:** Yes, RS232/FTDI adapter also supported (untested)
- VID/PID: `0403:6011`
- Driver: FTDI D2XX
- Status: ?? Not tested

### Q: What if my DNA ID is all zeros (0000000000000000)?
**A:** This indicates detection failed:
- Check JTAG cable connection
- Verify FPGA is powered
- Try manual detection again

### Q: Why does the UI freeze during detection?
**A:** Operations run synchronously:
- UI updates every frame
- Input blocked during operations
- **This is normal behavior**

### Q: Can I close the floating notification?
**A:** No, it closes automatically when operation completes:
- Prevents accidental clicks during operations
- Shows real-time progress
- Disappears when done

---

**Still having issues?**
Join our Discord: https://discord.gg/MfH9UHxkdP

**Last Updated:** January 2025
**Version:** 1.0
