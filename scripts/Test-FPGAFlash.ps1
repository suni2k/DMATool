# Test OpenOCD FPGA Flash Programming
# This script tests flashing firmware to FPGA using OpenOCD command-line

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("xc7a35t", "xc7a75t", "xc7a100t")]
    [string]$ChipModel = "xc7a75t",
    
    [Parameter(Mandatory=$false)]
    [string]$BinFile = "",
    
    [Parameter(Mandatory=$false)]
    [int]$ClockSpeed = 10000000,  # 10 MHz default
    
    [Parameter(Mandatory=$false)]
    [switch]$BitMode,  # If set, flash to RAM (temporary) instead of SPI flash
    
    [Parameter(Mandatory=$false)]
    [switch]$VerifyOnly,  # Only verify existing flash, don't program
    
    [Parameter(Mandatory=$false)]
    [switch]$EraseOnly  # Only erase flash, don't program
)

# =============================================================================
# Configuration
# =============================================================================

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# Paths - Updated to use dmafiles directory
$toolsDir = "C:\Users\suni\source\repos\DMATool\dmafiles\ch347\CH347FPGATool"
$openocdExe = Join-Path $toolsDir "OpenOCD_CH347\bin\openocd.exe"
$openocdBin = Join-Path $toolsDir "OpenOCD_CH347\bin"
$openocdScripts = Join-Path $toolsDir "OpenOCD_CH347\share\openocd\scripts"

# Set OpenOCD scripts path environment variable
$env:OPENOCD_SCRIPTS = $openocdScripts

# Default firmware files
$defaultFirmware = @{
    "xc7a75t" = Join-Path $toolsDir "002ced811686a854_ACE_75T.bin"
    "xc7a100t" = Join-Path $toolsDir "003ccd8c77d04854_BEEAC_100T.bin"
}

# BSCAN SPI bitstreams for different chips
$bscanBitstreams = @{
    "xc7a35t"  = "bscan_spi_xc7a35t.bit"
    "xc7a75t"  = "bscan_spi_xc7a75t.bit"
    "xc7a100t" = "bscan_spi_xc7a100t.bit"
}

# =============================================================================
# Functions
# =============================================================================

function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Type = "INFO"
    )
    
    $timestamp = Get-Date -Format "HH:mm:ss"
    
    switch ($Type) {
        "SUCCESS" { Write-Host "[$timestamp] [SUCCESS] $Message" -ForegroundColor Green }
        "ERROR"   { Write-Host "[$timestamp] [ERROR] $Message" -ForegroundColor Red }
        "WARNING" { Write-Host "[$timestamp] [WARNING] $Message" -ForegroundColor Yellow }
        "INFO"    { Write-Host "[$timestamp] [INFO] $Message" -ForegroundColor Cyan }
        default   { Write-Host "[$timestamp] $Message" }
    }
}

function Test-Prerequisites {
    Write-ColorOutput "Checking prerequisites..." "INFO"
    
    # Check if OpenOCD exists
    if (-not (Test-Path $openocdExe)) {
        Write-ColorOutput "OpenOCD not found at: $openocdExe" "ERROR"
        Write-ColorOutput "Please ensure CH347FPGATool is installed" "ERROR"
        return $false
    }
    
    # Check if required DLLs exist
    $requiredDlls = @("libusb-1.0.dll", "libhidapi-0.dll")
    foreach ($dll in $requiredDlls) {
        $dllPath = Join-Path $openocdBin $dll
        if (-not (Test-Path $dllPath)) {
            Write-ColorOutput "Missing required DLL: $dll" "ERROR"
            return $false
        }
    }
    
    Write-ColorOutput "Prerequisites OK" "SUCCESS"
    return $true
}

function Test-CH347Connection {
    Write-ColorOutput "Checking CH347 adapter connection..." "INFO"
    
    # Check if CH347 device is present
    $ch347Device = Get-PnpDevice | Where-Object { 
        $_.FriendlyName -like "*CH347*" -and $_.Status -eq "OK" 
    }
    
    if (-not $ch347Device) {
        Write-ColorOutput "CH347 device not found or not working" "ERROR"
        Write-ColorOutput "Please check:" "ERROR"
        Write-ColorOutput "  1. CH347 is plugged into USB port" "ERROR"
        Write-ColorOutput "  2. Driver is installed (USB HighSpeed-JTAG)" "ERROR"
        Write-ColorOutput "  3. Device shows in Device Manager" "ERROR"
        return $false
    }
    
    Write-ColorOutput "CH347 device found: $($ch347Device.FriendlyName)" "SUCCESS"
    return $true
}

function Create-OpenOCDConfig {
    param(
        [string]$ChipModel,
        [int]$ClockSpeed
    )
    
    $configFile = Join-Path $env:TEMP "flash_config.cfg"
    
    $config = @"
# OpenOCD Flash Configuration
# Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
# Chip: $ChipModel
# Clock: $ClockSpeed Hz

# CH347 adapter
adapter driver ch347
ch347 vid_pid 0x1a86 0x55dd

# CRITICAL: Must select transport type (JTAG for FPGA programming)
transport select jtag

# Set JTAG clock speed
adapter speed $ClockSpeed

# Load Xilinx 7-series configuration
source [find cpld/xilinx-xc7.cfg]

# Load JTAG SPI flash support
source [find cpld/jtagspi.cfg]
"@
    
    $config | Out-File -FilePath $configFile -Encoding ASCII -Force
    
    Write-ColorOutput "Created OpenOCD config: $configFile" "INFO"
    return $configFile
}

function Flash-Firmware {
    param(
        [string]$ConfigFile,
        [string]$BinFile,
        [string]$ChipModel
    )
    
    Write-ColorOutput "Starting flash programming..." "INFO"
    Write-ColorOutput "Chip: $ChipModel" "INFO"
    Write-ColorOutput "File: $BinFile" "INFO"
    Write-ColorOutput "Clock: $ClockSpeed Hz" "INFO"
    
    # Get BSCAN bitstream for this chip
    $bscanBit = $bscanBitstreams[$ChipModel.ToLower()]
    
    # Change to OpenOCD working directory
    Push-Location $openocdBin
    
    try {
        # Build command line - use & to call directly instead of Start-Process
        # This gives us better control over argument escaping
        $outputLog = Join-Path $env:TEMP "openocd_output.log"
        $errorLog = Join-Path $env:TEMP "openocd_error.log"
        
        Write-ColorOutput "Executing OpenOCD..." "INFO"
        
        # Call OpenOCD directly with proper argument handling
        & $openocdExe `
            -f $ConfigFile `
            -c "init" `
            -c "jtagspi_init 0 $bscanBit" `
            -c "jtagspi_program `"$BinFile`" 0x0" `
            -c "xc7_program xc7.tap" `
            -c "shutdown" `
            > $outputLog 2> $errorLog
        
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-ColorOutput "Flash programming completed successfully!" "SUCCESS"
            
            # Show output log
            $output = Get-Content $outputLog -Raw
            Write-ColorOutput "`nOpenOCD Output:" "INFO"
            Write-Host $output
            
            return $true
        } else {
            Write-ColorOutput "Flash programming failed with exit code: $exitCode" "ERROR"
            
            # Show error log
            $errorOutput = Get-Content $errorLog -Raw
            Write-ColorOutput "`nError Output:" "ERROR"
            Write-Host $errorOutput -ForegroundColor Red
            
            # Also show standard output in case there's useful info
            $output = Get-Content $outputLog -Raw
            if ($output) {
                Write-ColorOutput "`nStandard Output:" "INFO"
                Write-Host $output
            }
            
            return $false
        }
    }
    finally {
        Pop-Location
    }
}

function Flash-Bitstream {
    param(
        [string]$ConfigFile,
        [string]$BitFile
    )
    
    Write-ColorOutput "Programming bitstream to FPGA RAM (temporary)..." "INFO"
    Write-ColorOutput "File: $BitFile" "INFO"
    
    # Change to OpenOCD working directory
    Push-Location $openocdBin
    
    try {
        $outputLog = Join-Path $env:TEMP "openocd_output.log"
        $errorLog = Join-Path $env:TEMP "openocd_error.log"
        
        & $openocdExe `
            -f $ConfigFile `
            -c "init" `
            -c "pld load 0 `"$BitFile`"" `
            -c "shutdown" `
            > $outputLog 2> $errorLog
        
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-ColorOutput "Bitstream programming completed!" "SUCCESS"
            Write-ColorOutput "Note: This is temporary - will be lost on power cycle" "WARNING"
            return $true
        } else {
            Write-ColorOutput "Bitstream programming failed" "ERROR"
            $errorOutput = Get-Content $errorLog -Raw
            Write-Host $errorOutput -ForegroundColor Red
            return $false
        }
    }
    finally {
        Pop-Location
    }
}

function Verify-Flash {
    param(
        [string]$ConfigFile,
        [string]$BinFile,
        [string]$ChipModel
    )
    
    Write-ColorOutput "Verifying flash contents..." "INFO"
    
    $bscanBit = $bscanBitstreams[$ChipModel.ToLower()]
    
    # Change to OpenOCD working directory
    Push-Location $openocdBin
    
    try {
        $outputLog = Join-Path $env:TEMP "openocd_output.log"
        $errorLog = Join-Path $env:TEMP "openocd_error.log"
        
        & $openocdExe `
            -f $ConfigFile `
            -c "init" `
            -c "jtagspi_init 0 $bscanBit" `
            -c "flash verify_bank 0 `"$BinFile`"" `
            -c "shutdown" `
            > $outputLog 2> $errorLog
        
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-ColorOutput "Flash verification passed!" "SUCCESS"
            return $true
        } else {
            Write-ColorOutput "Flash verification failed!" "ERROR"
            $errorOutput = Get-Content $errorLog -Raw
            Write-Host $errorOutput -ForegroundColor Red
            return $false
        }
    }
    finally {
        Pop-Location
    }
}

function Erase-Flash {
    param(
        [string]$ConfigFile,
        [string]$ChipModel
    )
    
    Write-ColorOutput "Erasing flash memory..." "INFO"
    Write-ColorOutput "This may take 30-60 seconds..." "WARNING"
    
    $bscanBit = $bscanBitstreams[$ChipModel.ToLower()]
    
    # Change to OpenOCD working directory
    Push-Location $openocdBin
    
    try {
        $outputLog = Join-Path $env:TEMP "openocd_output.log"
        $errorLog = Join-Path $env:TEMP "openocd_error.log"
        
        & $openocdExe `
            -f $ConfigFile `
            -c "init" `
            -c "jtagspi_init 0 $bscanBit" `
            -c "flash erase_sector 0 0 last" `
            -c "shutdown" `
            > $outputLog 2> $errorLog
        
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-ColorOutput "Flash erase completed!" "SUCCESS"
            return $true
        } else {
            Write-ColorOutput "Flash erase failed!" "ERROR"
            $errorOutput = Get-Content $errorLog -Raw
            Write-Host $errorOutput -ForegroundColor Red
            return $false
        }
    }
    finally {
        Pop-Location
    }
}

# =============================================================================
# Main Script
# =============================================================================

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " OpenOCD FPGA Flash Test Script        " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Check prerequisites
if (-not (Test-Prerequisites)) {
    exit 1
}

# Step 2: Check CH347 connection
if (-not (Test-CH347Connection)) {
    exit 1
}

# Step 3: Determine firmware file
if ([string]::IsNullOrEmpty($BinFile)) {
    if ($defaultFirmware.ContainsKey($ChipModel)) {
        $BinFile = $defaultFirmware[$ChipModel]
        Write-ColorOutput "Using default firmware for $ChipModel`: $BinFile" "INFO"
    } else {
        Write-ColorOutput "No default firmware for $ChipModel" "ERROR"
        Write-ColorOutput "Please specify -BinFile parameter" "ERROR"
        exit 1
    }
}

# Verify file exists
if (-not (Test-Path $BinFile)) {
    Write-ColorOutput "Firmware file not found: $BinFile" "ERROR"
    exit 1
}

# Get file info
$fileInfo = Get-Item $BinFile
$fileSizeMB = [math]::Round($fileInfo.Length / 1MB, 2)
Write-ColorOutput "Firmware file size: $fileSizeMB MB" "INFO"

# Step 4: Create OpenOCD configuration
$configFile = Create-OpenOCDConfig -ChipModel $ChipModel -ClockSpeed $ClockSpeed

# Step 5: Perform operation
$success = $false

try {
    if ($EraseOnly) {
        # Erase flash only
        $success = Erase-Flash -ConfigFile $configFile -ChipModel $ChipModel
    }
    elseif ($VerifyOnly) {
        # Verify flash only
        $success = Verify-Flash -ConfigFile $configFile -BinFile $BinFile -ChipModel $ChipModel
    }
    elseif ($BitMode) {
        # Flash bitstream to RAM (temporary)
        $success = Flash-Bitstream -ConfigFile $configFile -BitFile $BinFile
    }
    else {
        # Flash BIN to SPI flash (permanent)
        $success = Flash-Firmware -ConfigFile $configFile -BinFile $BinFile -ChipModel $ChipModel
        
        # Optionally verify after programming
        if ($success) {
            Write-ColorOutput "`nVerifying programmed flash..." "INFO"
            $verifySuccess = Verify-Flash -ConfigFile $configFile -BinFile $BinFile -ChipModel $ChipModel
            
            if (-not $verifySuccess) {
                Write-ColorOutput "Flash programming succeeded but verification failed!" "WARNING"
                $success = $false
            }
        }
    }
}
finally {
    # Cleanup temp config file
    if (Test-Path $configFile) {
        Remove-Item $configFile -Force
    }
}

# Step 6: Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
if ($success) {
    Write-ColorOutput "Operation completed successfully!" "SUCCESS"
    exit 0
} else {
    Write-ColorOutput "Operation failed!" "ERROR"
    Write-ColorOutput "Check logs in $env:TEMP\openocd_*.log" "INFO"
    exit 1
}
