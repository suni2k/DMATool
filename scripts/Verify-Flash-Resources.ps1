# Verify Flash Resources for DMATool
# This script verifies that all required files for the Flash tab exist

Write-Host "==================================" -ForegroundColor Cyan
Write-Host " Flash Resources Verification" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

$basePath = "C:\Users\suni\source\repos\DMATool"
$errors = 0
$warnings = 0

# Function to check file exists
function Test-FileExists {
    param(
        [string]$Path,
        [string]$Description
    )
    
    $fullPath = Join-Path $basePath $Path
    
    if (Test-Path $fullPath) {
        $size = (Get-Item $fullPath).Length
        $sizeMB = [math]::Round($size / 1MB, 2)
        Write-Host "[OK]" -ForegroundColor Green -NoNewline
        Write-Host " $Description" -NoNewline
        Write-Host " ($sizeMB MB)" -ForegroundColor Gray
        return $true
    } else {
        Write-Host "[MISSING]" -ForegroundColor Red -NoNewline
        Write-Host " $Description"
        Write-Host "         Expected: $fullPath" -ForegroundColor Yellow
        $script:errors++
        return $false
    }
}

Write-Host "Checking OpenOCD Files..." -ForegroundColor Yellow
Write-Host ""

# OpenOCD Executable and Dependencies
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\openocd.exe" "OpenOCD 0.11 Executable"
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\libusb-1.0.dll" "LibUSB DLL"
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\libhidapi-0.dll" "LibHIDAPI DLL"

Write-Host ""
Write-Host "Checking OpenOCD Config Files..." -ForegroundColor Yellow
Write-Host ""

# Config Files
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\ch347.cfg" "CH347 Config"
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-xc7.cfg" "Xilinx 7-Series Config"
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-dna-347.cfg" "Xilinx DNA 347 Config"
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-dna.cfg" "Xilinx DNA Config"
Test-FileExists "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\jtagspi.cfg" "JTAG SPI Config"

Write-Host ""
Write-Host "Checking BSCAN Bitstreams..." -ForegroundColor Yellow
Write-Host ""

# BSCAN Bitstreams (most common ones)
$bscanPath = "dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx"
Test-FileExists "$bscanPath\bscan_spi_xc7a35t.bit" "BSCAN XC7A35T"
Test-FileExists "$bscanPath\bscan_spi_xc7a50t.bit" "BSCAN XC7A50T"
Test-FileExists "$bscanPath\bscan_spi_xc7a75t.bit" "BSCAN XC7A75T ?"
Test-FileExists "$bscanPath\bscan_spi_xc7a100t.bit" "BSCAN XC7A100T ?"
Test-FileExists "$bscanPath\bscan_spi_xc7a200t.bit" "BSCAN XC7A200T"

Write-Host ""
Write-Host "Checking Source Files..." -ForegroundColor Yellow
Write-Host ""

# Source Files
Test-FileExists "src\Backend\FlashInterface.h" "FlashInterface Header"
Test-FileExists "src\Backend\FlashInterface.cpp" "FlashInterface Implementation"
Test-FileExists "src\UI\Tabs\JTAGFlashTab.h" "JTAG Flash Tab Header"
Test-FileExists "src\UI\Tabs\JTAGFlashTab.cpp" "JTAG Flash Tab Implementation"

Write-Host ""
Write-Host "Checking DMATool.rc Resources..." -ForegroundColor Yellow
Write-Host ""

# Check DMATool.rc file
$rcPath = Join-Path $basePath "DMATool.rc"
if (Test-Path $rcPath) {
    $rcContent = Get-Content $rcPath -Raw
    
    # Check for OpenOCD resources
    if ($rcContent -match 'IDR_OPENOCD_EXE.*RCDATA.*"dmafiles\\CH347FPGATool\\OpenOCD_CH347\\bin\\openocd.exe"') {
        Write-Host "[OK]" -ForegroundColor Green -NoNewline
        Write-Host " OpenOCD EXE embedded in DMATool.rc"
    } else {
        Write-Host "[MISSING]" -ForegroundColor Red -NoNewline
        Write-Host " OpenOCD EXE not found in DMATool.rc"
        $script:errors++
    }
    
    # Check for BSCAN resources
    $bscanModels = @("XC7A35T", "XC7A50T", "XC7A75T", "XC7A100T", "XC7A200T")
    foreach ($model in $bscanModels) {
        if ($rcContent -match "IDR_BSCAN_$model") {
            Write-Host "[OK]" -ForegroundColor Green -NoNewline
            Write-Host " BSCAN $model embedded in DMATool.rc"
        } else {
            Write-Host "[WARNING]" -ForegroundColor Yellow -NoNewline
            Write-Host " BSCAN $model not found in DMATool.rc"
            $script:warnings++
        }
    }
    
    # Check for config files
    $configs = @("IDR_XILINX_XC7_CFG", "IDR_JTAGSPI_CFG", "IDR_XILINX_DNA_347_CFG")
    foreach ($cfg in $configs) {
        if ($rcContent -match $cfg) {
            Write-Host "[OK]" -ForegroundColor Green -NoNewline
            Write-Host " $cfg embedded in DMATool.rc"
        } else {
            Write-Host "[MISSING]" -ForegroundColor Red -NoNewline
            Write-Host " $cfg not found in DMATool.rc"
            $script:errors++
        }
    }
} else {
    Write-Host "[MISSING]" -ForegroundColor Red -NoNewline
    Write-Host " DMATool.rc file not found"
    $script:errors++
}

Write-Host ""
Write-Host "Checking resource.h Definitions..." -ForegroundColor Yellow
Write-Host ""

# Check resource.h
$resourceHPath = Join-Path $basePath "src\resource.h"
if (Test-Path $resourceHPath) {
    $resourceContent = Get-Content $resourceHPath -Raw
    
    # Check for BSCAN resource IDs
    $bscanModels = @("XC7A35T", "XC7A50T", "XC7A75T", "XC7A100T", "XC7A200T")
    foreach ($model in $bscanModels) {
        if ($resourceContent -match "IDR_BSCAN_$model") {
            Write-Host "[OK]" -ForegroundColor Green -NoNewline
            Write-Host " IDR_BSCAN_$model defined in resource.h"
        } else {
            Write-Host "[WARNING]" -ForegroundColor Yellow -NoNewline
            Write-Host " IDR_BSCAN_$model not defined in resource.h"
            $script:warnings++
        }
    }
} else {
    Write-Host "[MISSING]" -ForegroundColor Red -NoNewline
    Write-Host " src\resource.h file not found"
    $script:errors++
}

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host " Verification Summary" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

if ($errors -eq 0 -and $warnings -eq 0) {
    Write-Host "? All checks passed!" -ForegroundColor Green
    Write-Host "   Flash tab resources are properly configured." -ForegroundColor Green
    exit 0
} elseif ($errors -eq 0) {
    Write-Host "??  $warnings warning(s) found" -ForegroundColor Yellow
    Write-Host "   Flash tab should work but some optional resources are missing." -ForegroundColor Yellow
    exit 0
} else {
    Write-Host "? $errors error(s) and $warnings warning(s) found" -ForegroundColor Red
    Write-Host "   Flash tab may not work correctly. Please fix the errors above." -ForegroundColor Red
    exit 1
}
