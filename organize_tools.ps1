# DMATool - Tool Organization Script
# This script sets up the proper folder structure and copies necessary files

Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "DMATool - Tool Organization Script" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

$projectRoot = "C:\Users\suni\source\repos\DMATool"
$toolsRoot = "$projectRoot\tools"

# Create folder structure
Write-Host "Creating folder structure..." -ForegroundColor Yellow

$folders = @(
    "$toolsRoot\openocd",
    "$toolsRoot\openocd\lib",
    "$toolsRoot\openocd\configs",
    "$toolsRoot\openocd\scripts",
    "$toolsRoot\ch347",
    "$toolsRoot\ch347\drivers",
    "$toolsRoot\ch347\lib",
    "$toolsRoot\ch347\tools",
    "$toolsRoot\ftdi",
    "$toolsRoot\ftdi\drivers",
    "$toolsRoot\ftdi\lib",
    "$toolsRoot\pcileech",
    "$projectRoot\docs"
)

foreach ($folder in $folders) {
    if (!(Test-Path $folder)) {
        New-Item -ItemType Directory -Path $folder -Force | Out-Null
        Write-Host "  ✓ Created: $folder" -ForegroundColor Green
    } else {
        Write-Host "  ○ Exists: $folder" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Copying OpenOCD tools..." -ForegroundColor Yellow
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan

# Copy OpenOCD files
$openocdSource = "C:\Users\suni\Desktop\dma\DNA_ID\DNA_ID"
$openocdDest = "$toolsRoot\openocd"

if (Test-Path $openocdSource) {
    # Executables
    Copy-Item "$openocdSource\openocd-347.exe" "$openocdDest\" -Force
    Copy-Item "$openocdSource\openocd.exe" "$openocdDest\" -Force
    Write-Host "  ✓ Copied OpenOCD executables" -ForegroundColor Green
    
    # DLLs
    Copy-Item "$openocdSource\cygwin1.dll" "$openocdDest\lib\" -Force
    Copy-Item "$openocdSource\cygusb-1.0.dll" "$openocdDest\lib\" -Force
    Copy-Item "$openocdSource\libhidapi-0.dll" "$openocdDest\lib\" -Force
    Copy-Item "$openocdSource\libusb-1.0.dll" "$openocdDest\lib\" -Force
    Write-Host "  ✓ Copied OpenOCD libraries" -ForegroundColor Green
    
    # Config files
    Copy-Item "$openocdSource\init_347_35t.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\init_347_75t.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\init_232_35t.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\init_232_75t.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\xilinx-dna.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\xilinx-dna-347.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\xilinx-xc7.cfg" "$openocdDest\configs\" -Force
    Copy-Item "$openocdSource\jtagspi.cfg" "$openocdDest\configs\" -Force
    Write-Host "  ✓ Copied OpenOCD config files" -ForegroundColor Green
} else {
    Write-Host "  ✗ Source not found: $openocdSource" -ForegroundColor Red
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Copying OpenOCD scripts..." -ForegroundColor Yellow
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan

# Copy OpenOCD scripts from ch347 repo
$scriptsSource = "$projectRoot\dmafiles\ch347\CH347FPGATool\OpenOCD_CH347\scripts"
$scriptsDest = "$toolsRoot\openocd\scripts"

if (Test-Path $scriptsSource) {
    Copy-Item "$scriptsSource\*" "$scriptsDest\" -Recurse -Force
    Write-Host "  ✓ Copied OpenOCD scripts directory" -ForegroundColor Green
} else {
    Write-Host "  ✗ Source not found: $scriptsSource" -ForegroundColor Red
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Copying CH347 drivers and tools..." -ForegroundColor Yellow
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan

# Copy CH347 files
$ch347Source = "C:\Users\suni\Desktop\dma\75T-driver"
$ch347Dest = "$toolsRoot\ch347"

if (Test-Path $ch347Source) {
    # Drivers
    $driverSource = "$ch347Source\Driver Installer (open)"
    if (Test-Path $driverSource) {
        Copy-Item "$driverSource\CH341WDM.INF" "$ch347Dest\drivers\" -Force
        Copy-Item "$driverSource\CH341WDM.CAT" "$ch347Dest\drivers\" -Force
        Copy-Item "$driverSource\CH341WDM.SYS" "$ch347Dest\drivers\" -Force
        Copy-Item "$driverSource\CH341M64.SYS" "$ch347Dest\drivers\" -Force
        Copy-Item "$driverSource\CH341W64.SYS" "$ch347Dest\drivers\" -Force
        Copy-Item "$driverSource\CH347DLL.DLL" "$ch347Dest\drivers\" -Force
        Copy-Item "$driverSource\CH347DLLA64.DLL" "$ch347Dest\drivers\" -Force
        Write-Host "  ✓ Copied CH347 drivers" -ForegroundColor Green
        
        # Tools
        Copy-Item "$driverSource\75TDriver.exe.EXE" "$ch347Dest\tools\75TDriver.exe" -Force
        if (Test-Path "$driverSource\DRVSETUP64\DRVSETUP64.exe") {
            Copy-Item "$driverSource\DRVSETUP64\DRVSETUP64.exe" "$ch347Dest\tools\" -Force
        }
        Write-Host "  ✓ Copied CH347 tools" -ForegroundColor Green
    }
    
    # Libraries
    $libSource = "$ch347Source\LIB"
    if (Test-Path $libSource) {
        Copy-Item "$libSource\*" "$ch347Dest\lib\" -Recurse -Force
        Write-Host "  ✓ Copied CH347 development libraries" -ForegroundColor Green
    }
} else {
    Write-Host "  ✗ Source not found: $ch347Source" -ForegroundColor Red
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Copying CH347 FPGA Tool..." -ForegroundColor Yellow
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan

# Copy CH347 FPGA Tool
$fpgaToolSource = "$projectRoot\dmafiles\ch347\CH347FPGATool\CH347FpgaDownloadTool.exe"
if (Test-Path $fpgaToolSource) {
    Copy-Item $fpgaToolSource "$ch347Dest\tools\" -Force
    Write-Host "  ✓ Copied CH347FpgaDownloadTool.exe" -ForegroundColor Green
} else {
    Write-Host "  ✗ Source not found: $fpgaToolSource" -ForegroundColor Red
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Moving documentation..." -ForegroundColor Yellow
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan

# Move documentation
$docSource = "$projectRoot\dmafiles"
$docDest = "$projectRoot\docs"

if (Test-Path "$docSource\DETECTION_GUIDE.txt") {
    Move-Item "$docSource\DETECTION_GUIDE.txt" "$docDest\" -Force
    Write-Host "  ✓ Moved DETECTION_GUIDE.txt" -ForegroundColor Green
}
if (Test-Path "$docSource\DRIVER_GUIDE.md") {
    Move-Item "$docSource\DRIVER_GUIDE.md" "$docDest\" -Force
    Write-Host "  ✓ Moved DRIVER_GUIDE.md" -ForegroundColor Green
}
if (Test-Path "$docSource\SETUP_GUIDE.md") {
    Move-Item "$docSource\SETUP_GUIDE.md" "$docDest\" -Force
    Write-Host "  ✓ Moved SETUP_GUIDE.md" -ForegroundColor Green
}
if (Test-Path "$docSource\QUICK_SUMMARY.txt") {
    Move-Item "$docSource\QUICK_SUMMARY.txt" "$docDest\" -Force
    Write-Host "  ✓ Moved QUICK_SUMMARY.txt" -ForegroundColor Green
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Setup Complete!" -ForegroundColor Green
Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

# Summary
Write-Host "Project Structure:" -ForegroundColor Yellow
Write-Host ""
Write-Host "  C:\Users\suni\source\repos\DMATool\" -ForegroundColor Cyan
Write-Host "    ├── tools\" -ForegroundColor White
Write-Host "    │   ├── openocd\          (OpenOCD executables & configs)" -ForegroundColor Gray
Write-Host "    │   ├── ch347\            (CH347 drivers & tools)" -ForegroundColor Gray
Write-Host "    │   ├── ftdi\             (FTDI drivers - future)" -ForegroundColor Gray
Write-Host "    │   └── pcileech\         (PCILeech - future)" -ForegroundColor Gray
Write-Host "    ├── docs\                 (Documentation)" -ForegroundColor Gray
Write-Host "    └── PROJECT_SPECIFICATION.md" -ForegroundColor Gray
Write-Host ""

Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "  1. Open Visual Studio" -ForegroundColor White
Write-Host "  2. Read PROJECT_SPECIFICATION.md" -ForegroundColor White
Write-Host "  3. Start implementing core classes" -ForegroundColor White
Write-Host "  4. Test with your XC7A100T card" -ForegroundColor White
Write-Host ""

Write-Host "Press any key to exit..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
