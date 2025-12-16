# DMATool Architecture Documentation

## Overview
DMATool is a Windows desktop application for managing DMA (Direct Memory Access) cards, specifically Xilinx FPGA-based cards with CH347 or RS232/FTDI JTAG interfaces. The tool provides driver management, FPGA detection, DNA ID extraction, and flash programming capabilities.

## Project Structure

### Core Application Files

#### `src/Main.cpp`
- **Purpose**: Application entry point and main window initialization
- **Key Functions**:
  - `WinMain()`: Windows application entry point
  - Window creation and message pump
  - ImGui initialization and setup
  - Main application loop
- **Dependencies**: ImGui, Windows API

#### `src/Application.cpp` / `src/Application.h`
- **Purpose**: Main application state and UI rendering
- **Key Components**:
  - Tab management (Driver Check, JTAG, FPGA, Flash)
  - Main render loop
  - Global state management
  - Tab switching logic
- **Key Functions**:
  - `Render()`: Main UI rendering function
  - Tab rendering methods for each section

### Driver Management

#### `src/DriverManager.cpp` / `src/DriverManager.h`
- **Purpose**: Hardware detection and driver status checking
- **Supported Adapters**:
  - **RS232/FTDI** (35T Squirrel): VID 0x0403, PID 0x6011
  - **CH347** (35T/75T/100T): VID 0x1A86, PID 0x55DD/0x55DE
- **Key Functions**:
  - `DetectHardware()`: Scans for DMA card adapters
  - `CheckDriverStatus()`: Verifies correct driver installation
  - `GetAdapterType()`: Returns detected adapter type

#### `src/RS232DriverInstaller.cpp` / `src/RS232DriverInstaller.h`
- **Purpose**: RS232/FTDI WinUSB driver installation and management
- **Features**:
  - Extracts WinUSB driver files (INF, CAT, coinstallers)
  - Uses Windows API `UpdateDriverForPlugAndPlayDevicesW()` to force driver update
  - Preserves other FTDI interfaces (MI_01, MI_02, MI_03) as COM ports
  - Proper driver backup and restoration
- **Key Functions**:
  - `InstallDriver()`: Installs WinUSB driver on Interface 0 only
  - `UninstallDriver()`: Restores FTDIBUS driver
  - `ExtractDriverFiles()`: Extracts embedded driver resources
- **Technical Details**:
  - Only replaces driver on Interface 0 (MI_00)
  - Keeps FTDIBUS in driver store for other interfaces
  - Uses INSTALLFLAG_FORCE to override existing driver

#### `src/CH347DriverInstaller.cpp` / `src/CH347DriverInstaller.h`
- **Purpose**: CH347 WCH driver installation and management
- **Features**:
  - Installs WCH CH347 JTAG driver
  - Removes conflicting drivers
  - Device enumeration and status checking
- **Key Functions**:
  - `InstallDriver()`: Installs CH347 WCH driver
  - `UninstallDriver()`: Removes CH347 driver packages
  - `FindWCHDrivers()`: Locates installed WCH drivers

### FPGA Detection

#### `src/FPGADetector.cpp` / `src/FPGADetector.h`
- **Purpose**: FPGA chip detection and DNA ID extraction
- **Supported Operations**:
  - FPGA chip type identification (35T, 75T, 100T)
  - DNA ID extraction (57-bit unique identifier)
  - IDCODE parsing
- **Key Functions**:
  - `DetectFPGA()`: Main FPGA detection routine
  - `ExtractChipType()`: Parses IDCODE to determine chip model
  - `ExtractDNAId()`: Extracts DNA from OpenOCD output
- **OpenOCD Integration**:
  - **RS232/FTDI**: Uses `openocd-ftdi.exe` with FTDI interface commands
  - **CH347**: Uses `openocd.exe` with CH347 interface commands
  - Parses OpenOCD output for chip information

### Flash Programming

#### `src/FlashProgrammer.cpp` / `src/FlashProgrammer.h`
- **Purpose**: FPGA flash memory programming
- **Supported Cards**:
  - Squirrel 35T (RS232)
  - CH347 35T
  - CH347 75T
  - CH347 100T
- **Key Functions**:
  - `ProgramFlash()`: Programs bitstream to flash memory
  - `VerifyFlash()`: Verifies programmed data
  - `EraseFlash()`: Erases flash sectors
- **Technical Details**:
  - Uses OpenOCD for flash operations
  - Different configs per card type
  - Progress tracking and error handling

### Utilities

#### `src/CommandExecutor.cpp` / `src/CommandExecutor.h`
- **Purpose**: Execute system commands and capture output
- **Key Functions**:
  - `Execute()`: Runs PowerShell/CMD commands
  - `CaptureOutput()`: Captures command stdout/stderr
  - `GetExitCode()`: Returns process exit code
- **Features**:
  - Synchronous and asynchronous execution
  - Real-time output streaming
  - Error handling

#### `src/ResourceExtractor.cpp` / `src/ResourceExtractor.h`
- **Purpose**: Extract embedded resources to temp directory
- **Embedded Resources**:
  - `openocd.exe` (CH347 support)
  - `openocd-ftdi.exe` (FTDI support)
  - DLL dependencies (libusb, libhidapi, cygwin1, cygusb)
  - Config files (*.cfg for JTAG/flash operations)
  - Driver packages (INF, CAT, SYS files)
- **Key Functions**:
  - `ExtractToTemp()`: Extracts all resources to temp folder
  - `CleanupTemp()`: Removes temporary files
  - `GetResourcePath()`: Returns path to extracted resource

#### `src/Logger.cpp` / `src/Logger.h`
- **Purpose**: Application logging and debug output
- **Features**:
  - Console output for debugging
  - File logging (optional)
  - Log levels (INFO, WARNING, ERROR, DEBUG)
- **Key Functions**:
  - `Log()`: Logs message with level
  - `GetLogBuffer()`: Returns recent log messages
  - `ClearLog()`: Clears log buffer

### UI Components

#### `src/DriverCheckTab.cpp` / `src/DriverCheckTab.h`
- **Purpose**: Driver status checking UI
- **Features**:
  - Hardware detection display
  - Driver status indicators
  - Install/Uninstall buttons
  - Real-time status updates

#### `src/JTAGTab.cpp` / `src/JTAGTab.h`
- **Purpose**: JTAG interface configuration and testing
- **Features**:
  - Adapter information display
  - JTAG connection testing
  - OpenOCD command execution

#### `src/FPGATab.cpp` / `src/FPGATab.h`
- **Purpose**: FPGA detection and information display
- **Features**:
  - Detect FPGA button
  - Chip type display (35T/75T/100T)
  - DNA ID display
  - Card type identification (Squirrel/CH347)

#### `src/FlashTab.cpp` / `src/FlashTab.h`
- **Purpose**: Flash programming interface
- **Features**:
  - Bitstream file selection
  - Program/Verify/Erase operations
  - Progress bar
  - Status messages

## Data Flow

### Driver Installation Flow (RS232)
1. User clicks "Install RS232 Driver"
2. `RS232DriverInstaller::InstallDriver()` called
3. Extract WinUSB driver files to temp
4. Add WinUSB driver to driver store (`pnputil /add-driver`)
5. Call `UpdateDriverForPlugAndPlayDevicesW()` to force Interface 0 to WinUSB
6. FTDIBUS remains in store for other interfaces
7. Verify WinUSB installation

### Driver Uninstallation Flow (RS232)
1. User clicks "Uninstall RS232 Driver"
2. `RS232DriverInstaller::UninstallDriver()` called
3. Remove WinUSB driver package (`pnputil /delete-driver`)
4. Remove device to trigger re-enumeration
5. Windows automatically reinstalls FTDIBUS
6. Verify FTDIBUS restoration

### FPGA Detection Flow
1. User clicks "Detect FPGA"
2. `FPGADetector::DetectFPGA()` called
3. Detect adapter type (RS232 or CH347)
4. Select appropriate OpenOCD binary and commands
5. Execute OpenOCD with chip-specific config
6. Parse output for IDCODE and DNA
7. Update UI with chip type and DNA ID

### Flash Programming Flow
1. User selects bitstream file
2. User clicks "Program Flash"
3. `FlashProgrammer::ProgramFlash()` called
4. Verify card type and chip compatibility
5. Execute OpenOCD flash commands
6. Monitor progress and update UI
7. Verify programming success

## Configuration Files

### OpenOCD Config Files (in resources/cpld/)
- `xilinx-dna.cfg`: DNA extraction commands
- `xilinx-xc7.cfg`: Xilinx 7-series chip definitions
- `xilinx-dna-347.cfg`: CH347-specific DNA extraction
- `cpld-jtagspi.cfg`: SPI flash programming via JTAG

### Driver Files (embedded resources)
- **RS232 WinUSB**:
  - `quad_rs232-hs_(interface_0).inf`
  - `Quad_RS232-HS_(Interface_0).cat`
  - `WinUSBCoInstaller2.dll`
  - `WdfCoInstaller01011.dll`
- **FTDIBUS** (for restoration):
  - `ftdibus.inf`
  - `ftdibus.cat`
  - `ftdibus.sys`
  - `ftser2k.sys`

## Key Technologies

### Windows APIs
- **SetupAPI**: Device enumeration and driver installation
- **Newdev.dll**: `UpdateDriverForPlugAndPlayDevicesW()` for forced driver updates
- **Win32 API**: Process creation, file operations, registry access

### External Tools
- **OpenOCD**: JTAG interface and FPGA communication
  - Version 0.11.0 (CH347 support)
  - Version 0.10.0 (FTDI support)
- **pnputil**: Windows driver management utility
- **PowerShell**: System queries and device management

### UI Framework
- **Dear ImGui**: Immediate mode GUI library
- **DirectX 11**: Graphics rendering backend
- **ImGui Docking**: Tab and window management

## Build Configuration

### Dependencies
- **ImGui**: UI framework (vendor/imgui/)
- **libusb**: USB device access (embedded)
- **libhidapi**: HID device access (embedded)
- **VMProtect**: Code protection (optional)

### Build Targets
- **Debug**: Development build with full logging
- **Release**: Optimized build for distribution
- **x64**: 64-bit Windows platform

## Error Handling

### Common Error Scenarios
1. **Driver Installation Fails**: Admin rights required
2. **FPGA Not Detected**: Check JTAG connections, power, driver
3. **Flash Programming Fails**: Verify bitstream compatibility, chip type
4. **OpenOCD Errors**: Check adapter driver, USB connection

### Error Recovery
- Automatic retry logic for transient failures
- Detailed error messages with troubleshooting steps
- Fallback mechanisms (e.g., Zadig for manual driver install)

## Security Considerations

### Elevated Privileges
- Driver installation requires administrator rights
- USB device access requires proper driver permissions
- Flash programming has direct hardware access

### Code Protection
- VMProtect integration for release builds
- Resource encryption for sensitive data
- Anti-tampering measures

## Future Enhancements

### Planned Features
- Support for additional FPGA models
- Automated firmware updates
- Remote device management
- Multi-device support
- Configuration profiles

### Technical Debt
- Consolidate OpenOCD binaries (single binary with both adapters)
- Improve error handling and recovery
- Add comprehensive unit tests
- Refactor UI code for better modularity

## Development Guidelines

### Code Style
- Use descriptive variable and function names
- Comment complex logic and Windows API calls
- Keep functions focused and single-purpose
- Use RAII for resource management

### Testing
- Test on both RS232 and CH347 hardware
- Verify driver installation/uninstallation
- Test all card types (35T, 75T, 100T)
- Check error handling paths

### Debugging
- Enable verbose logging in Debug builds
- Use temp folder inspection for resource extraction
- Monitor PowerShell command output
- Check Windows Event Viewer for driver issues
