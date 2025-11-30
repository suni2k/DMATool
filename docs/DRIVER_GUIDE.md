# DMA Card Driver Management Guide

## Overview
Your DMA card's JTAG/Update port requires drivers to be installed **on the same PC** where the card is installed (the PC doing the DNA extraction, not a target PC).

## Important Clarifications

### Where Drivers Are Installed
✅ **YES** - Drivers are installed on the **SAME PC** where the DMA card is installed  
✅ **YES** - This is the PC you use to flash firmware and extract DNA ID  
❌ **NO** - Not on target/victim PC (the target PC doesn't know about the JTAG port at all)

### Why Install Drivers Here
The JTAG/Update port on your DMA card connects back to the same PC via USB:
1. DMA card sits in PCIe slot
2. USB cable connects from card's JTAG port → USB port on same motherboard
3. This allows you to program/read the FPGA chip via JTAG
4. OpenOCD uses these drivers to communicate with the FPGA

---

## Driver Types

### 1. CH347 Driver (Your Current Setup)
**Used for:** Modern DMA cards with WCH CH347 chip (USB-C interface)
- **Manufacturer:** WCH (wch.cn)
- **VID/PID:** 0x1A86 / 0x55DD (or 0x55DE)
- **Interface:** USB 2.0 High Speed (480 Mbps)
- **Speed:** Faster than RS232/FTDI

**Your Current Installation:**
```
Driver Version: 2.4.2023.10
Driver Date: 10/09/2023
Provider: wch.cn
Status: ✓ Installed and Working
```

### 2. RS232/FTDI Driver (Older Cards)
**Used for:** Older DMA cards with FTDI FT232H/FT2232H chips
- **Manufacturer:** FTDI
- **VID/PID:** 0x0403 / 0x6011 (35T) or 0x0403 / 0x6014 (75T)
- **Interface:** USB 2.0 via FTDI bridge
- **Speed:** Slower than CH347

---

## DNA ID Formatting

### Raw Format (from OpenOCD)
```
DNA = 000111100110011011000110001110111110100000100100001010100 (0x003ccd8c77d04854)
```

### Formatted for Firmware Providers
**Remove the "0x" prefix:**
```
003ccd8c77d04854
```

### Code Example (C++)
```cpp
std::string dna_raw = "0x003ccd8c77d04854";
std::string dna_formatted = dna_raw.substr(2); // Remove "0x"
// Result: "003ccd8c77d04854"
```

---

## Driver Management Features for Your Tool

### 1. Check Driver Installation Status

#### CH347 Driver Check
```powershell
# Check if CH347 device exists
$ch347 = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*CH347*"}

if ($ch347) {
    Write-Host "✓ CH347 Device Found"
    
    # Get driver version
    $version = Get-PnpDeviceProperty -InstanceId $ch347.InstanceId `
               -KeyName "DEVPKEY_Device_DriverVersion"
    Write-Host "Driver Version: $($version.Data)"
    
    # Get driver date
    $date = Get-PnpDeviceProperty -InstanceId $ch347.InstanceId `
            -KeyName "DEVPKEY_Device_DriverDate"
    Write-Host "Driver Date: $($date.Data)"
} else {
    Write-Host "✗ CH347 Device Not Found"
}
```

#### FTDI Driver Check
```powershell
# Check if FTDI device exists
$ftdi = Get-PnpDevice | Where-Object {
    $_.FriendlyName -like "*FT232*" -or 
    $_.FriendlyName -like "*FT2232*" -or
    $_.HardwareID -like "*VID_0403*"
}

if ($ftdi) {
    Write-Host "✓ FTDI Device Found"
    # Get version info same as CH347
}
```

### 2. Get Driver Version from Windows Driver Store
```powershell
Get-WindowsDriver -Online | Where-Object {
    $_.ProviderName -like "*wch*" -or 
    $_.ProviderName -like "*ftdi*"
} | Select-Object Driver, ClassName, ProviderName, Version, Date
```

### 3. Install CH347 Driver

#### Method 1: Via CH347FpgaDownloadTool.exe
```
Location: C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool\CH347FpgaDownloadTool.exe

Action: Run the tool - it will auto-install drivers on first launch
Requires: Administrator privileges
```

#### Method 2: Manual Driver Installation (if .inf available)
```powershell
# If you have CH347 driver .inf file
pnputil.exe /add-driver "path\to\ch347driver.inf" /install
```

#### Method 3: WCH Official Driver Package
```
Download from: https://www.wch-ic.com/downloads/CH347EVT_ZIP.html
Or: https://github.com/WCHSoftGroup/ch347/releases
Install: Run setup.exe as administrator
```

### 4. Uninstall Driver

#### Remove from Driver Store
```powershell
# Find the driver in driver store
$driver = Get-WindowsDriver -Online | Where-Object {
    $_.ProviderName -like "*wch*" -and 
    $_.ClassName -eq "WCH"
}

# Remove driver (requires admin)
pnputil.exe /delete-driver $driver.Driver /uninstall /force
```

#### Uninstall Device (keeps driver in store)
```powershell
# Get device instance
$device = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*CH347*"}

# Uninstall device
pnputil /remove-device $device.InstanceId
```

---

## GUI Implementation Checklist

### Driver Status Display
- [ ] Show driver installation status (Installed / Not Installed)
- [ ] Display driver version
- [ ] Display driver date
- [ ] Show provider (wch.cn / FTDI)
- [ ] Indicate if driver is working (device detected)

### Install Driver Button
- [ ] Check for admin privileges
- [ ] Launch CH347FpgaDownloadTool.exe for auto-install
- [ ] OR provide wizard to download/install WCH driver package
- [ ] Show progress/status
- [ ] Verify installation after completion

### Uninstall Driver Button
- [ ] Check for admin privileges
- [ ] Warn user about consequences
- [ ] Remove driver from driver store
- [ ] Remove device instance
- [ ] Confirm uninstallation

### Update Driver Button
- [ ] Check current version
- [ ] Download latest from WCH/FTDI
- [ ] Install updated driver
- [ ] Restart device if needed

### DNA Formatting
- [ ] Extract DNA ID with OpenOCD
- [ ] Remove "0x" prefix automatically
- [ ] Display: `003ccd8c77d04854`
- [ ] Copy to clipboard button
- [ ] Export to text file option

---

## PowerShell Commands for C++ Integration

### Check if Running as Administrator
```powershell
([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
```

### Elevate to Administrator (if needed)
```cpp
// C++ - ShellExecute to elevate
ShellExecute(NULL, L"runas", L"YourApp.exe", L"install-driver", NULL, SW_SHOWNORMAL);
```

### Get All Driver Info (JSON output for parsing)
```powershell
$ch347 = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*CH347*"} | Select-Object -First 1

if ($ch347) {
    $info = @{
        Status = $ch347.Status
        FriendlyName = $ch347.FriendlyName
        DriverVersion = (Get-PnpDeviceProperty -InstanceId $ch347.InstanceId -KeyName "DEVPKEY_Device_DriverVersion").Data
        DriverDate = (Get-PnpDeviceProperty -InstanceId $ch347.InstanceId -KeyName "DEVPKEY_Device_DriverDate").Data
        DriverProvider = (Get-PnpDeviceProperty -InstanceId $ch347.InstanceId -KeyName "DEVPKEY_Device_DriverProvider").Data
    }
    
    $info | ConvertTo-Json
}
```

---

## Important Notes

1. **Administrator Rights Required**
   - Installing drivers requires admin privileges
   - Uninstalling drivers requires admin privileges
   - Your GUI should check for this and elevate if needed

2. **Driver Conflicts**
   - CH347 and FTDI drivers can coexist
   - Make sure correct driver matches device connected
   - Some cards may have both interfaces (rare)

3. **Driver Updates**
   - WCH updates CH347 drivers occasionally
   - Check https://www.wch-ic.com for latest versions
   - Current latest: Check releases on GitHub

4. **Troubleshooting**
   - If device not detected: Try different USB port
   - If driver fails: Uninstall completely and reinstall
   - If OpenOCD fails: Check driver is installed for correct VID/PID

---

## Example: Complete Driver Status Check

```cpp
// Pseudo C++ code for your GUI

class DriverManager {
public:
    struct DriverInfo {
        bool installed;
        std::string version;
        std::string date;
        std::string provider;
        std::string deviceName;
    };
    
    DriverInfo CheckCH347Driver() {
        // Run PowerShell command
        std::string psCommand = R"(
            $device = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*CH347*"} | Select-Object -First 1
            if ($device) {
                @{
                    Installed = $true
                    Version = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName "DEVPKEY_Device_DriverVersion").Data
                    Date = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName "DEVPKEY_Device_DriverDate").Data
                    Provider = (Get-PnpDeviceProperty -InstanceId $device.InstanceId -KeyName "DEVPKEY_Device_DriverProvider").Data
                    DeviceName = $device.FriendlyName
                } | ConvertTo-Json
            } else {
                @{Installed = $false} | ConvertTo-Json
            }
        )";
        
        // Execute and parse JSON
        std::string output = ExecutePowerShell(psCommand);
        return ParseDriverInfo(output);
    }
    
    bool InstallCH347Driver() {
        // Launch CH347FpgaDownloadTool.exe as admin
        return ShellExecuteAsAdmin("CH347FpgaDownloadTool.exe");
    }
    
    bool UninstallCH347Driver() {
        // Remove from driver store
        std::string psCommand = R"(
            $driver = Get-WindowsDriver -Online | Where-Object {$_.ProviderName -like "*wch*"}
            pnputil.exe /delete-driver $driver.Driver /uninstall /force
        )";
        
        return ExecutePowerShellAsAdmin(psCommand);
    }
};
```

---

## Summary

✅ **Drivers install on same PC as DMA card**  
✅ **CH347 driver currently installed and working**  
✅ **DNA ID should be formatted without "0x" prefix**  
✅ **Use PowerShell commands for driver management**  
✅ **Require admin privileges for install/uninstall**  

Your current setup is perfect - CH347 driver v2.4.2023.10 is working properly!
