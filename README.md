# DMATool - Modern DMA Hardware Interface Tool

A modern C++ desktop application using ImGui for interfacing with DMA hardware, JTAG adapters, and FPGA programming.

## Features

- **Modern UI**: Clean, shadcn-inspired theme with smooth animations
- **Three Interface Modes**:
  - JTAG Port of DMA: JTAG chain detection and control
  - JTAG Port of DMA Flash Tool: FPGA firmware programming
  - Data Port of DMA: Direct memory access and analysis
- **Project Management**: Create and load projects with saved configurations
- **Hardware Integration Ready**: Structured for integration with industry-standard tools

## Planned Hardware Integration

This application is architected to integrate with the following hardware interface libraries:

### DMA & Memory Access
- **[PCILeech](https://github.com/ufrisk/pcileech)**: DMA attack framework and memory acquisition
- **[LeechCore](https://github.com/ufrisk/LeechCore)**: Physical memory acquisition library
- **[MemProcFS](https://github.com/ufrisk/MemProcFS)**: Virtual file system for physical memory
- **[MemProcFS-plugins](https://github.com/ufrisk/MemProcFS-plugins)**: Extension plugins

### FPGA & JTAG Programming
- **[PCILeech-FPGA](https://github.com/ufrisk/pcileech-fpga)**: FPGA firmware for PCILeech
- **[CH347](https://github.com/WCHSoftGroup/ch347)**: USB-JTAG/SPI bridge driver

## Building

### Prerequisites

- Visual Studio 2019 or later
- C++17 compatible compiler
- CMake 3.15+ (optional)

### Required Libraries

1. **ImGui** (https://github.com/ocornut/imgui)
   - Core library for GUI
   - Download and place in `vendor/imgui/`

2. **GLFW** (https://www.glfw.org/)
   - Window management and input
   - Download precompiled binaries or build from source
   - Place in `vendor/glfw/`

3. **OpenGL** (included with graphics drivers)

### Build Steps

1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd DMATool
   ```

2. Download and set up dependencies:
   - Download ImGui from https://github.com/ocornut/imgui/releases
   - Extract to `vendor/imgui/`
   - Download GLFW from https://www.glfw.org/download.html
   - Extract to `vendor/glfw/`

3. Open `DMATool.sln` in Visual Studio

4. Build the solution (F7)

5. Run the application (F5)

## Project Structure

```
DMATool/
??? src/
?   ??? main.cpp                    # Application entry point
?   ??? Application.h/cpp           # Main application class
?   ??? UI/
?   ?   ??? Theme.h/cpp            # Shadcn-inspired theme
?   ?   ??? MainWindow.h/cpp       # Main window with tabs
?   ?   ??? Tabs/
?   ?       ??? JTAGPortTab.h/cpp     # JTAG control interface
?   ?       ??? JTAGFlashTab.h/cpp    # Flash programming interface
?   ?       ??? DataPortTab.h/cpp     # DMA memory interface
?   ??? Backend/
?       ??? ProjectManager.h/cpp    # Project management
?       ??? DMAInterface.h/cpp      # DMA operations (LeechCore integration)
?       ??? JTAGInterface.h/cpp     # JTAG operations (CH347 integration)
??? vendor/                         # Third-party libraries
?   ??? imgui/                     # ImGui library
?   ??? glfw/                      # GLFW library
??? README.md
```

## Architecture

### UI Layer (`src/UI/`)
- Handles all rendering and user interaction
- Uses ImGui for immediate mode GUI
- Implements shadcn-inspired theme with smooth animations
- Separated into tabs for different hardware interfaces

### Backend Layer (`src/Backend/`)
- Provides hardware abstraction
- Placeholder implementations for future integration
- Clear interface definitions for:
  - DMA operations (via LeechCore)
  - JTAG operations (via CH347)
  - Project management and persistence

## Usage

### Getting Started

1. Launch DMATool
2. Choose to create a new project or load an existing one
3. Navigate between tabs for different operations

### JTAG Port of DMA Tab
- Connect to JTAG adapter (CH347 or compatible)
- Detect JTAG chain
- Read device IDs
- Perform boundary scan operations

### JTAG Flash Tool Tab
- Detect flash memory devices
- Read flash contents
- Program FPGA firmware (PCILeech-FPGA compatible)
- Verify programming

### Data Port of DMA Tab
- Initialize DMA device
- Read/write physical memory
- Search for patterns
- View memory in hex/ASCII format

## Future Development

### Immediate Next Steps
1. Integrate LeechCore library for DMA operations
2. Integrate CH347 driver for JTAG operations
3. Implement file dialogs for project/firmware selection
4. Add configuration persistence (JSON)

### Planned Features
- Memory search and pattern matching
- Signature-based scanning
- Process memory analysis via MemProcFS
- Plugin system for extensions
- Scripting support for automation
- Log export and session recording

## Contributing

Contributions are welcome! Please follow these guidelines:
- Maintain the separation between UI and backend layers
- Use the existing code style and conventions
- Add comments for hardware-specific implementations
- Test with actual hardware when possible

## License

[Specify your license here]

## Disclaimer

This tool is intended for legitimate hardware development, debugging, and research purposes. Users are responsible for ensuring their use complies with applicable laws and regulations.

## References

- ImGui: https://github.com/ocornut/imgui
- GLFW: https://www.glfw.org/
- PCILeech: https://github.com/ufrisk/pcileech
- LeechCore: https://github.com/ufrisk/LeechCore
- MemProcFS: https://github.com/ufrisk/MemProcFS
- CH347: https://github.com/WCHSoftGroup/ch347
- shadcn/ui: https://ui.shadcn.com/ (design inspiration)
