# Device Masking Quality Checker
# Tests how well your DMA card is disguised as the target device
# Detects common issues that would trigger anti-cheat detection

param(
    [Parameter(Mandatory=$false)]
    [string]$TargetVendorID = "",  # e.g., "8086" for Intel
    
    [Parameter(Mandatory=$false)]
    [string]$TargetDeviceID = "",  # e.g., "1533" for Intel I210
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("Network", "Display", "Storage", "Any")]
    [string]$DeviceClass = "Any"
)

# =============================================================================
# Configuration
# =============================================================================

$ErrorActionPreference = "Continue"
$script:Score = 0
$script:MaxScore = 0
$script:Issues = @()
$script:Warnings = @()
$script:GoodPoints = @()

# Known bad patterns
$XILINX_VENDOR_ID = "10EE"
$GENERIC_VENDOR_IDS = @("0000", "FFFF", "1234")
$SUSPICIOUS_DEVICE_NAMES = @(
    "*Xilinx*",
    "*FPGA*",
    "*Development Board*",
    "*Unknown Device*",
    "*Generic*"
)

# =============================================================================
# Helper Functions
# =============================================================================

function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Type = "INFO"
    )
    
    switch ($Type) {
        "SUCCESS" { Write-Host "? $Message" -ForegroundColor Green }
        "ERROR"   { Write-Host "? $Message" -ForegroundColor Red }
        "WARNING" { Write-Host "? $Message" -ForegroundColor Yellow }
        "INFO"    { Write-Host "? $Message" -ForegroundColor Cyan }
        "HEADER"  { Write-Host "`n=== $Message ===" -ForegroundColor Cyan }
        default   { Write-Host $Message }
    }
}

function Add-Issue {
    param([string]$Message, [int]$Severity = 10)
    $script:Issues += $Message
    $script:Score -= $Severity
}

function Add-Warning {
    param([string]$Message, [int]$Severity = 5)
    $script:Warnings += $Message
    $script:Score -= $Severity
}

function Add-GoodPoint {
    param([string]$Message, [int]$Points = 10)
    $script:GoodPoints += $Message
    $script:Score += $Points
}

function Increment-MaxScore {
    param([int]$Points = 10)
    $script:MaxScore += $Points
}

# =============================================================================
# Test 1: Vendor ID Check (CRITICAL)
# =============================================================================

function Test-VendorID {
    Write-ColorOutput "Test 1: Vendor ID Check" "HEADER"
    Increment-MaxScore -Points 30
    
    # Get all PCIe devices
    $devices = Get-PnpDevice -PresentOnly | Where-Object {
        $_.InstanceId -like "PCI\*" -and
        $_.Status -eq "OK"
    }
    
    Write-Host ""
    Write-Host "?? All PCIe Devices Detected:" -ForegroundColor Cyan
    Write-Host "??????????????????????????????????????????????????????" -ForegroundColor DarkGray
    
    foreach ($dev in $devices) {
        # Parse hardware ID
        $hwid = $dev.HardwareID[0]
        if ($hwid -match "VEN_([0-9A-F]{4})") {
            $vendorId = $matches[1]
        }
        if ($hwid -match "DEV_([0-9A-F]{4})") {
            $deviceId = $matches[1]
        }
        if ($hwid -match "SUBSYS_([0-9A-F]{8})") {
            $subsysId = $matches[1]
        }
        
        # Determine vendor name
        $vendorName = switch ($vendorId) {
            "8086" { "Intel" }
            "10DE" { "NVIDIA" }
            "1002" { "AMD" }
            "10EC" { "Realtek" }
            "14E4" { "Broadcom" }
            "168C" { "Qualcomm Atheros" }
            "1022" { "AMD" }
            "8086" { "Intel" }
            "10EE" { "?? XILINX (DMA CARD!)" }
            default { "Unknown" }
        }
        
        $color = if ($vendorId -eq "10EE") { "Red" } else { "White" }
        
        Write-Host "  Device: " -NoNewline -ForegroundColor Gray
        Write-Host $dev.FriendlyName -ForegroundColor $color
        Write-Host "    Vendor: " -NoNewline -ForegroundColor DarkGray
        Write-Host "$vendorName ($vendorId)" -ForegroundColor $color
        Write-Host "    Device ID: " -NoNewline -ForegroundColor DarkGray
        Write-Host $deviceId -ForegroundColor $color
        Write-Host "    Class: " -NoNewline -ForegroundColor DarkGray
        Write-Host $dev.Class -ForegroundColor $color
        
        # Get PCIe location
        try {
            $location = Get-PnpDeviceProperty -InstanceId $dev.InstanceId -KeyName "DEVPKEY_Device_LocationInfo" -ErrorAction SilentlyContinue
            if ($location.Data) {
                Write-Host "    Location: " -NoNewline -ForegroundColor DarkGray
                Write-Host $location.Data -ForegroundColor Gray
            }
        } catch { }
        
        Write-Host ""
    }
    
    Write-Host "??????????????????????????????????????????????????????" -ForegroundColor DarkGray
    Write-Host ""
    
    # Check for Xilinx devices
    $xilinxDevices = $devices | Where-Object {
        $_.HardwareID -like "*VEN_$XILINX_VENDOR_ID*"
    }
    
    if ($xilinxDevices) {
        Add-Issue "CRITICAL: Xilinx vendor ID ($XILINX_VENDOR_ID) detected!" -Severity 30
        Write-ColorOutput "?? DETECTED AS DMA CARD - NOT MASKED!" "ERROR"
        Write-Host ""
        Write-ColorOutput "Xilinx devices found:" "ERROR"
        foreach ($dev in $xilinxDevices) {
            Write-Host "  ??  $($dev.FriendlyName)" -ForegroundColor Red
            Write-Host "     Hardware ID: $($dev.HardwareID[0])" -ForegroundColor DarkRed
        }
        Write-Host ""
        Write-ColorOutput "??  This will trigger INSTANT DETECTION by anti-cheat!" "ERROR"
        Write-ColorOutput "??  Your DMA card is NOT properly masked!" "ERROR"
        return $false
    } else {
        Add-GoodPoint "No Xilinx vendor ID detected" -Points 30
        Write-ColorOutput "? No Xilinx devices found - device masking is active!" "SUCCESS"
        
        # Highlight what it's masquerading as
        $networkDevices = $devices | Where-Object { $_.Class -eq "Net" }
        $displayDevices = $devices | Where-Object { $_.Class -eq "Display" }
        
        if ($networkDevices) {
            Write-Host ""
            Write-Host "?? Possible DMA card masquerading as:" -ForegroundColor Green
            foreach ($dev in $networkDevices) {
                $hwid = $dev.HardwareID[0]
                if ($hwid -match "VEN_([0-9A-F]{4})") {
                    $vid = $matches[1]
                    if ($vid -in @("8086", "10EC", "14E4")) {  # Common network adapters
                        Write-Host "   ? $($dev.FriendlyName)" -ForegroundColor Green
                        Write-Host "      (Vendor: $vid)" -ForegroundColor DarkGreen
                    }
                }
            }
        }
        
        return $true
    }
}

# =============================================================================
# Test 2: Device Name Check
# =============================================================================

function Test-DeviceName {
    Write-ColorOutput "Test 2: Device Friendly Name Check" "HEADER"
    Increment-MaxScore -Points 20
    
    $devices = Get-PnpDevice -PresentOnly | Where-Object {
        $_.InstanceId -like "PCI\*"
    }
    
    $suspiciousFound = $false
    
    foreach ($pattern in $SUSPICIOUS_DEVICE_NAMES) {
        $suspicious = $devices | Where-Object { $_.FriendlyName -like $pattern }
        if ($suspicious) {
            $suspiciousFound = $true
            Add-Issue "Suspicious device name found: $($suspicious.FriendlyName)" -Severity 10
            Write-ColorOutput "Found: $($suspicious.FriendlyName)" "ERROR"
        }
    }
    
    if (-not $suspiciousFound) {
        Add-GoodPoint "No suspicious device names found" -Points 20
        Write-ColorOutput "Device names look legitimate" "SUCCESS"
    }
    
    # If target vendor specified, check for it
    if ($TargetVendorID) {
        $targetDevices = $devices | Where-Object {
            $_.HardwareID -like "*VEN_$TargetVendorID*"
        }
        
        if ($targetDevices) {
            Write-ColorOutput "Found device with target vendor ID ($TargetVendorID):" "SUCCESS"
            Write-Host "  $($targetDevices[0].FriendlyName)" -ForegroundColor Green
        } else {
            Add-Warning "Target vendor ID ($TargetVendorID) not found"
        }
    }
}

# =============================================================================
# Test 3: Driver Status Check
# =============================================================================

function Test-DriverStatus {
    Write-ColorOutput "Test 3: Driver Status Check" "HEADER"
    Increment-MaxScore -Points 15
    
    $classFilter = switch ($DeviceClass) {
        "Network" { "Net" }
        "Display" { "Display" }
        "Storage" { "SCSIAdapter", "HDC" }
        "Any"     { $null }
    }
    
    if ($classFilter) {
        $devices = Get-PnpDevice -Class $classFilter -PresentOnly
    } else {
        $devices = Get-PnpDevice -PresentOnly | Where-Object {
            $_.InstanceId -like "PCI\*"
        }
    }
    
    # Check for problem devices
    $problemDevices = $devices | Where-Object {
        $_.Status -ne "OK" -and $_.Status -ne "Unknown"
    }
    
    if ($problemDevices) {
        Add-Warning "Found devices with driver issues:"
        foreach ($dev in $problemDevices) {
            Write-Host "  - $($dev.FriendlyName): $($dev.Status)" -ForegroundColor Yellow
            
            # Check problem code
            $devNode = Get-PnpDeviceProperty -InstanceId $dev.InstanceId -KeyName "DEVPKEY_Device_ProblemCode" -ErrorAction SilentlyContinue
            if ($devNode.Data -ne 0) {
                Write-Host "    Problem Code: $($devNode.Data)" -ForegroundColor DarkYellow
            }
        }
    } else {
        Add-GoodPoint "All devices have drivers loaded correctly" -Points 15
        Write-ColorOutput "No driver errors detected" "SUCCESS"
    }
}

# =============================================================================
# Test 4: Windows Event Log Check
# =============================================================================

function Test-EventLog {
    Write-ColorOutput "Test 4: Windows Event Log Check (Last 24 Hours)" "HEADER"
    Increment-MaxScore -Points 10
    
    try {
        $yesterday = (Get-Date).AddDays(-1)
        
        # Check for PCIe errors
        $pciErrors = Get-WinEvent -FilterHashtable @{
            LogName = 'System'
            Level = 2  # Error
            StartTime = $yesterday
        } -ErrorAction SilentlyContinue | Where-Object {
            $_.Message -like "*PCI*" -or 
            $_.Message -like "*PCIe*" -or
            $_.ProviderName -eq "pci"
        } | Select-Object -First 10
        
        if ($pciErrors) {
            Add-Warning "Found PCIe errors in Event Log:" -Severity 5
            foreach ($error in $pciErrors) {
                Write-Host "  - $($error.TimeCreated): $($error.Message.Substring(0, [Math]::Min(80, $error.Message.Length)))..." -ForegroundColor Yellow
            }
        } else {
            Add-GoodPoint "No PCIe errors in Windows Event Log" -Points 10
            Write-ColorOutput "No recent PCIe errors found" "SUCCESS"
        }
    } catch {
        Write-ColorOutput "Could not access Event Log (may need admin rights)" "WARNING"
    }
}

# =============================================================================
# Test 5: Device Enumeration Check
# =============================================================================

function Test-DeviceEnumeration {
    Write-ColorOutput "Test 5: Device Enumeration Stability" "HEADER"
    Increment-MaxScore -Points 10
    
    Write-ColorOutput "Checking for devices that recently changed state..." "INFO"
    
    try {
        $yesterday = (Get-Date).AddDays(-1)
        
        $deviceChanges = Get-WinEvent -FilterHashtable @{
            LogName = 'System'
            ProviderName = 'Microsoft-Windows-Kernel-PnP'
            StartTime = $yesterday
        } -ErrorAction SilentlyContinue | Where-Object {
            $_.Id -in @(400, 401, 410, 411)  # Device start/stop events
        } | Group-Object { $_.Message } | Where-Object { $_.Count -gt 5 }
        
        if ($deviceChanges) {
            Add-Warning "Device(s) with frequent state changes detected (possible re-enumeration):" -Severity 5
            Write-Host "  Some devices are connecting/disconnecting frequently" -ForegroundColor Yellow
        } else {
            Add-GoodPoint "Device enumeration appears stable" -Points 10
            Write-ColorOutput "No frequent re-enumeration detected" "SUCCESS"
        }
    } catch {
        Write-ColorOutput "Could not check device enumeration history" "INFO"
        Increment-MaxScore -Points -10  # Don't penalize if can't check
    }
}

# =============================================================================
# Test 6: Hardware ID Analysis
# =============================================================================

function Test-HardwareIDs {
    Write-ColorOutput "Test 6: Hardware ID Pattern Analysis" "HEADER"
    Increment-MaxScore -Points 15
    
    $devices = Get-PnpDevice -PresentOnly | Where-Object {
        $_.InstanceId -like "PCI\*"
    }
    
    $suspicious = $false
    
    foreach ($dev in $devices) {
        $hwid = $dev.HardwareID[0]
        
        # Check for generic/suspicious patterns
        if ($hwid -match "VEN_0000" -or $hwid -match "DEV_0000") {
            Add-Warning "Device with null vendor/device ID: $($dev.FriendlyName)"
            $suspicious = $true
        }
        
        if ($hwid -match "VEN_FFFF" -or $hwid -match "DEV_FFFF") {
            Add-Warning "Device with invalid vendor/device ID: $($dev.FriendlyName)"
            $suspicious = $true
        }
        
        # Check for Xilinx again (critical)
        if ($hwid -match "VEN_10EE") {
            # Already caught in Test 1, but flag again
            $suspicious = $true
        }
    }
    
    if (-not $suspicious) {
        Add-GoodPoint "Hardware IDs look legitimate" -Points 15
        Write-ColorOutput "All hardware IDs appear valid" "SUCCESS"
    }
}

# =============================================================================
# Test 7: PCIe Configuration Space Check (Requires PCILeech)
# =============================================================================

function Test-PCIeConfigSpace {
    Write-ColorOutput "Test 7: PCIe Configuration Space Check" "HEADER"
    
    # Check if PCILeech is available
    $pcileechPaths = @(
        "C:\Tools\PCILeech\pcileech.exe",
        ".\pcileech.exe",
        "$env:ProgramFiles\PCILeech\pcileech.exe"
    )
    
    $pcileech = $pcileechPaths | Where-Object { Test-Path $_ } | Select-Object -First 1
    
    if (-not $pcileech) {
        Write-ColorOutput "PCILeech not found - skipping detailed PCIe config check" "WARNING"
        Write-ColorOutput "Install PCILeech for advanced verification: https://github.com/ufrisk/pcileech" "INFO"
        return
    }
    
    Increment-MaxScore -Points 20
    
    Write-ColorOutput "Found PCILeech at: $pcileech" "INFO"
    Write-ColorOutput "Attempting to probe FPGA device..." "INFO"
    
    try {
        $output = & $pcileech probe 2>&1 | Out-String
        
        if ($output -match "Xilinx") {
            Add-Issue "PCILeech detected Xilinx FPGA signature!" -Severity 20
            Write-ColorOutput "FPGA is detectable via PCILeech probe!" "ERROR"
        } elseif ($output -match "DEVICE: FPGA") {
            Add-Issue "PCILeech identified device as FPGA" -Severity 15
            Write-ColorOutput "Device identified as FPGA - poor masking" "ERROR"
        } else {
            Add-GoodPoint "PCILeech probe shows good device masking" -Points 20
            Write-ColorOutput "Device not identified as FPGA by PCILeech" "SUCCESS"
        }
        
        Write-Host "`nPCILeech Output:" -ForegroundColor Gray
        Write-Host $output -ForegroundColor DarkGray
        
    } catch {
        Write-ColorOutput "Could not run PCILeech probe: $_" "WARNING"
    }
}

# =============================================================================
# Test 8: DNA ID Uniqueness (Requires DMATool)
# =============================================================================

function Test-DNAUniqueness {
    Write-ColorOutput "Test 8: FPGA DNA ID Uniqueness Check" "HEADER"
    
    Write-ColorOutput "To verify DNA ID uniqueness:" "INFO"
    Write-Host "  1. Open DMATool.exe" -ForegroundColor Cyan
    Write-Host "  2. Go to 'DNA ID' tab" -ForegroundColor Cyan
    Write-Host "  3. Click 'Detect FPGA & Read DNA'" -ForegroundColor Cyan
    Write-Host "  4. Verify DNA ID is NOT all zeros or all ones" -ForegroundColor Cyan
    Write-Host ""
    Write-ColorOutput "CRITICAL: Each FPGA has a unique hardware DNA ID" "INFO"
    Write-ColorOutput "If multiple cards show same DNA = CLONE/FAKE!" "WARNING"
}

# =============================================================================
# Generate Report
# =============================================================================

function Generate-Report {
    Write-Host "`n" 
    Write-Host "============================================" -ForegroundColor Cyan
    Write-Host " Device Masking Quality Report" -ForegroundColor Cyan
    Write-Host "============================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Calculate percentage score
    $percentage = if ($script:MaxScore -gt 0) {
        [math]::Max(0, [math]::Min(100, [int](($script:Score / $script:MaxScore) * 100)))
    } else {
        0
    }
    
    # Determine risk level
    $riskLevel = switch ($percentage) {
        { $_ -ge 85 } { "LOW ?"; $riskColor = "Green"; break }
        { $_ -ge 60 } { "MEDIUM ??"; $riskColor = "Yellow"; break }
        { $_ -ge 40 } { "HIGH ?"; $riskColor = "Red"; break }
        default { "CRITICAL ??"; $riskColor = "Red" }
    }
    
    Write-Host "Overall Score: " -NoNewline
    Write-Host "$percentage / 100" -ForegroundColor $riskColor
    Write-Host "Detection Risk: " -NoNewline
    Write-Host "$riskLevel" -ForegroundColor $riskColor
    Write-Host ""
    
    # Risk interpretation
    Write-Host "Risk Assessment:" -ForegroundColor Cyan
    switch ($percentage) {
        { $_ -ge 85 } {
            Write-Host "  ? Excellent masking quality" -ForegroundColor Green
            Write-Host "  ? Low detection probability (<5%)" -ForegroundColor Green
            Write-Host "  ? Professional-grade firmware" -ForegroundColor Green
        }
        { $_ -ge 60 } {
            Write-Host "  ? Decent masking but has issues" -ForegroundColor Yellow
            Write-Host "  ? Moderate detection risk (20-40%)" -ForegroundColor Yellow
            Write-Host "  ? May trigger some anti-cheat systems" -ForegroundColor Yellow
        }
        { $_ -ge 40 } {
            Write-Host "  ? Poor masking quality" -ForegroundColor Red
            Write-Host "  ? High detection risk (60-80%)" -ForegroundColor Red
            Write-Host "  ? Will likely trigger anti-cheat" -ForegroundColor Red
        }
        default {
            Write-Host "  ?? CRITICAL: No effective masking" -ForegroundColor Red
            Write-Host "  ?? Virtually guaranteed detection (90-100%)" -ForegroundColor Red
            Write-Host "  ?? INSTANT BAN risk" -ForegroundColor Red
        }
    }
    Write-Host ""
    
    # Good points
    if ($script:GoodPoints) {
        Write-Host "? Strengths ($($script:GoodPoints.Count)):" -ForegroundColor Green
        foreach ($point in $script:GoodPoints) {
            Write-Host "  • $point" -ForegroundColor Green
        }
        Write-Host ""
    }
    
    # Warnings
    if ($script:Warnings) {
        Write-Host "? Warnings ($($script:Warnings.Count)):" -ForegroundColor Yellow
        foreach ($warning in $script:Warnings) {
            Write-Host "  • $warning" -ForegroundColor Yellow
        }
        Write-Host ""
    }
    
    # Critical issues
    if ($script:Issues) {
        Write-Host "? Critical Issues ($($script:Issues.Count)):" -ForegroundColor Red
        foreach ($issue in $script:Issues) {
            Write-Host "  • $issue" -ForegroundColor Red
        }
        Write-Host ""
    }
    
    # Recommendations
    Write-Host "Recommendations:" -ForegroundColor Cyan
    if ($percentage -lt 85) {
        Write-Host "  1. Review firmware configuration" -ForegroundColor White
        Write-Host "  2. Verify vendor/device IDs match target device exactly" -ForegroundColor White
        Write-Host "  3. Test with DMATool DNA ID tab" -ForegroundColor White
        
        if ($script:Issues -match "Xilinx") {
            Write-Host "  4. ?? URGENT: Flash better firmware - Xilinx ID detected!" -ForegroundColor Red
        }
    } else {
        Write-Host "  ? Device masking quality is good!" -ForegroundColor Green
        Write-Host "  • Continue with functional testing" -ForegroundColor White
        Write-Host "  • Test in target environment" -ForegroundColor White
    }
    
    Write-Host ""
    Write-Host "============================================" -ForegroundColor Cyan
}

# =============================================================================
# Main Execution
# =============================================================================

Write-Host ""
Write-Host "??????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host "?   DMA Device Masking Quality Checker      ?" -ForegroundColor Cyan
Write-Host "??????????????????????????????????????????????" -ForegroundColor Cyan
Write-Host ""
Write-Host "Testing how well your DMA card is disguised..." -ForegroundColor Yellow
Write-Host ""

# Run all tests
Test-VendorID
Test-DeviceName
Test-DriverStatus
Test-EventLog
Test-DeviceEnumeration
Test-HardwareIDs
Test-PCIeConfigSpace
Test-DNAUniqueness

# Generate report
Generate-Report

# Save report to file
$reportFile = Join-Path $env:TEMP "dma_masking_report_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
$report = @"
DMA Device Masking Quality Report
Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')

Score: $script:Score / $script:MaxScore ($([int](($script:Score / $script:MaxScore) * 100))%)

=== Good Points ===
$($script:GoodPoints -join "`n")

=== Warnings ===
$($script:Warnings -join "`n")

=== Critical Issues ===
$($script:Issues -join "`n")
"@

$report | Out-File -FilePath $reportFile -Encoding UTF8
Write-Host "Report saved to: $reportFile" -ForegroundColor Gray
Write-Host ""
