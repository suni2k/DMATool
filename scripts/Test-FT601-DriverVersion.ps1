# Test script to check what driver version PowerShell returns for FT601

Write-Host "=== Testing FT601 Driver Version Detection ===" -ForegroundColor Cyan
Write-Host ""

$DeviceVID = "0403"
$DevicePID = "601F"

Write-Host "Step 1: Searching for USB Composite Device (VID/PID only)..." -ForegroundColor Yellow
$compositeDevice = Get-PnpDevice | Where-Object {$_.InstanceId -like "*VID_$DeviceVID&PID_$DevicePID*"} | Select-Object -First 1

if ($compositeDevice) {
    Write-Host "  Found: $($compositeDevice.FriendlyName)" -ForegroundColor White
    $provider1 = (Get-PnpDeviceProperty -InstanceId $compositeDevice.InstanceId -KeyName 'DEVPKEY_Device_DriverProvider' -ErrorAction SilentlyContinue).Data
    Write-Host "  Provider: $provider1" -ForegroundColor $(if ($provider1 -eq "FTDI") {"Green"} else {"Red"})
} else {
    Write-Host "  Not found" -ForegroundColor Red
}

Write-Host ""
Write-Host "Step 2: Searching for FTDI FIFO Bridge Device (by name + VID/PID)..." -ForegroundColor Yellow
$device = Get-PnpDevice | Where-Object {
    ($_.FriendlyName -like '*FTDI*FIFO*' -or $_.FriendlyName -like '*SuperSpeed*FIFO*') -and
    $_.InstanceId -like "*VID_$DeviceVID&PID_$DevicePID*"
} | Select-Object -First 1

if ($device) {
    Write-Host "  Found: $($device.FriendlyName)" -ForegroundColor Green
    Write-Host ""
    
    # Get all relevant properties
    $instanceId = $device.InstanceId
    
    Write-Host "Instance ID: $instanceId" -ForegroundColor White
    Write-Host ""
    
    # Get Provider
    Write-Host "Getting Driver Provider..." -ForegroundColor Yellow
    $provider = (Get-PnpDeviceProperty -InstanceId $instanceId -KeyName 'DEVPKEY_Device_DriverProvider' -ErrorAction SilentlyContinue).Data
    Write-Host "  Provider: $provider" -ForegroundColor $(if ($provider -eq "FTDI") {"Green"} else {"Red"})
    
    # Get Driver Version
    Write-Host "Getting Driver Version (DEVPKEY_Device_DriverVersion)..." -ForegroundColor Yellow
    $driverVersion = (Get-PnpDeviceProperty -InstanceId $instanceId -KeyName 'DEVPKEY_Device_DriverVersion' -ErrorAction SilentlyContinue).Data
    Write-Host "  DriverVersion: $driverVersion" -ForegroundColor $(if ($driverVersion -like "1.4.*") {"Green"} else {"White"})
    
    # Get INF Path and read version from INF
    Write-Host "Getting INF file path..." -ForegroundColor Yellow
    $driverInf = (Get-PnpDeviceProperty -InstanceId $instanceId -KeyName 'DEVPKEY_Device_DriverInfPath' -ErrorAction SilentlyContinue).Data
    Write-Host "  INF File: $driverInf" -ForegroundColor White
    
    if ($driverInf) {
        $infFile = Join-Path $env:SystemRoot ("INF\$driverInf")
        if (Test-Path $infFile) {
            Write-Host "  Full path: $infFile" -ForegroundColor White
            Write-Host ""
            Write-Host "Reading INF file content..." -ForegroundColor Yellow
            $infContent = Get-Content $infFile -Raw
            
            # Look for DriverVer line
            if ($infContent -match 'DriverVer\s*=\s*([^,]+),\s*([\d.]+)') {
                $driverDate = $Matches[1]
                $driverInfVersion = $Matches[2]
                Write-Host "  INF DriverVer Date: $driverDate" -ForegroundColor White
                Write-Host "  INF DriverVer Version: $driverInfVersion" -ForegroundColor Green
            } else {
                Write-Host "  Could not parse DriverVer from INF" -ForegroundColor Red
            }
        } else {
            Write-Host "  INF file not found at: $infFile" -ForegroundColor Red
        }
    }
    
    Write-Host ""
    Write-Host "=== Summary ===" -ForegroundColor Cyan
    Write-Host "Device Name: $($device.FriendlyName)"
    Write-Host "Provider: $provider"
    Write-Host "DEVPKEY_Device_DriverVersion: $driverVersion"
    if ($driverInfVersion) {
        Write-Host "INF DriverVer: $driverInfVersion"
    }
    
    Write-Host ""
    if ($provider -eq "FTDI") {
        Write-Host "? FTDI driver is installed" -ForegroundColor Green
        if ($driverVersion) {
            if ($driverVersion -ge "1.4.0.1") {
                Write-Host "? Version is CORRECT (>= 1.4.0.1)" -ForegroundColor Green
            } else {
                Write-Host "? Version is OUT OF DATE (< 1.4.0.1)" -ForegroundColor Red
            }
        }
    } else {
        Write-Host "? Using default Windows driver (no FTDI driver)" -ForegroundColor Red
    }
} else {
    Write-Host "  Not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "No FTDI FIFO device found!" -ForegroundColor Red
    Write-Host "Make sure the DMA device is connected and recognized by Windows." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
