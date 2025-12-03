# DMATool - Professional DMA Hardware Interface

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Platform](https://img.shields.io/badge/platform-Windows-blue)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()
[![Version](https://img.shields.io/badge/version-1.0-orange)]()

**DMATool** is a professional-grade hardware interface tool for DMA (Direct Memory Access) hardware, providing FPGA detection, DNA ID extraction, firmware flashing, and performance benchmarking capabilities.

<p align="center">
  <img src="https://img.shields.io/badge/DMA-KINGS-gold?style=for-the-badge" alt="DMA Kings">
</p>

---

## ? Features

### ?? **DNA ID Tab** ? **COMPLETE**
- **Automatic FPGA Detection** - Detects Xilinx Artix-7 FPGAs (XC7A35T, XC7A75T, XC7A100T)
- **DNA ID Extraction** - Reads 57-bit unique device identifier
- **Smart Driver Management** - Automated driver installation/uninstallation
- **Auto-Detection on Startup** - Intelligent detection with driver validation
- **Copy to Clipboard** - One-click DNA ID copying
- **Support for Multiple Adapters:**
  - ? **CH347 USB-JTAG** (Fully tested and working)
  - ?? **RS232/FTDI** (Implemented, not tested - no hardware)

### ? **Flash DMA Tab** ?? **TODO**
- FPGA firmware programming via JTAG
- Bitstream upload and verification
- Flash memory operations

### ?? **Benchmark DMA Tab** ?? **TODO**
- DMA read/write speed tests
- Memory throughput benchmarks
- Performance analysis and reporting

---

## ?? Quick Start

### Prerequisites
- Windows 10/11 (x64)
- CH347 USB-JTAG adapter (or RS232/FTDI adapter)
- Visual Studio 2022 (for building from source)

### Download & Run
1. Download latest release: `DMATool.exe`
2. Run `DMATool.exe` (Administrator recommended)
3. Click "ENTER TOOL" on splash screen
4. First-time setup:
   - Tool auto-detects CH347 adapter
   - If driver needed, click "Install CH347 Driver"
   - Approve UAC prompt
   - Wait for installation
5. Click "Detect FPGA & Read DNA"
6. Copy DNA ID to clipboard

**That's it!** ?

---

## ?? Screenshots

### Splash Screen
```
???????????????????????????????????????????????????
?                                             [X] ?
?                                                 ?
?                 DMA KINGS                       ?
?      Professional DMA Hardware Interface Tool   ?
?                                                 ?
?         ????????????????????????????            ?
?         ?     ENTER TOOL           ?            ?
?         ????????????????????????????            ?
?                                                 ?
?  [Website]   [Discord]   [Setup Guide]          ?
?                                                 ?
?  DMA Kings Tool v1.0 | © 2025 DMA Kings         ?
???????????????????????????????????????????????????
```

### DNA ID Tab
```
???????????????????????????????????????????????????
? DMA KINGS TOOL                              [X] ?
???????????????????????????????????????????????????
?  [DNA ID]  [Flash DMA]  [Benchmark DMA]         ?
???????????????????????????????????????????????????
? FPGA Device Info     ? JTAG Driver Info         ?
? ????????????????     ? ????????????????         ?
? Chip: XC7A75T        ? Status: Installed ?      ?
? Adapter: CH347       ? Version: 2.5.2024.03     ?
? Manufacturer: Xilinx ? Device: HighSpeed-JTAG   ?
? Family: Artix-7      ? VID/PID: 1A86:55DD       ?
?                      ?                          ?
? DNA ID               ? Management               ?
? ??????               ? ??????????               ?
? 00542417dc636678     ? [Check Driver Status]    ?
?                      ? [Install CH347 Driver]   ?
? [Detect FPGA & DNA]  ? [Uninstall CH347 Driver] ?
? [Copy to Clipboard]  ?                          ?
???????????????????????????????????????????????????
? Status & Log                                    ?
? ????????????????????????????????????????????    ?
? Connection: Connected    Detection: Detected    ?
? Last Operation: Auto-Detection                  ?
? ????????????????????????????????????????????    ?
? [SUCCESS] FPGA detected: XC7A75T               ?
? [INFO] DNA ID: 00542417dc636678                ?
? [SUCCESS] Driver is installed                  ?
???????????????????????????????????????????????????
```

---

## ?? Documentation

### Core Documentation
- ?? **[DNA ID Tab - Complete Guide](docs/DNA_ID_TAB_COMPLETE.md)** - Full feature documentation
- ?? **[DMA Flash Guide](docs/DMA_FLASH_GUIDE.md)** - Firmware flashing guide & OpenOCD usage
- ?? **[Flash Testing Reference](docs/FLASH_TESTING_QUICK_REF.md)** - Command-line flash testing
- ?? **[Flash Implementation Summary](docs/FLASH_IMPLEMENTATION_SUMMARY.md)** - Integration roadmap
- ?? **[DMA Benchmarking Guide](docs/DMA_BENCHMARKING_GUIDE.md)** - Performance testing and validation
- ?? **[Benchmark Tab Specification](docs/BENCHMARK_TAB_SPEC.md)** - Technical implementation details
- ?? **[DMA Testing Guide](docs/DMA_TESTING_GUIDE.md)** - Manual testing with PCILeech
- ??? **[Architecture](docs/ARCHITECTURE.md)** - Project structure and design
- ?? **[Build Instructions](docs/BUILD_INSTRUCTIONS.md)** - How to compile from source
- ?? **[Troubleshooting](docs/TROUBLESHOOTING.md)** - Common issues and solutions

### External Resources
- ?? **Website:** https://www.dmakings.com
- ?? **Discord:** https://discord.gg/MfH9UHxkdP
- ?? **Setup Guide:** https://injectkings.gitbook.io/dma-kings

---

## ??? Building from Source

### Quick Build
```bash
# 1. Clone repository
git clone https://github.com/akwanmn/DMATool.git
cd DMATool

# 2. Open in Visual Studio 2022
DMATool.sln

# 3. Build
Ctrl + Shift + B
```

**Output:** `bin/Release-x64/DMATool.exe`

See [BUILD_INSTRUCTIONS.md](docs/BUILD_INSTRUCTIONS.md) for detailed instructions.

---

## ?? System Requirements

### Minimum Requirements
- **OS:** Windows 10 (1903 or later) or Windows 11
- **Architecture:** x64 (64-bit)
- **RAM:** 4 GB
- **Disk Space:** 50 MB
- **Graphics:** OpenGL 3.3 compatible GPU
- **USB:** USB 2.0 or higher

### Recommended Requirements
- **OS:** Windows 11
- **RAM:** 8 GB or more
- **USB:** USB 3.0 for faster operations
- **Permissions:** Administrator privileges (for driver installation)

### Hardware Requirements
- **Required:** CH347 USB-JTAG adapter or RS232/FTDI adapter
- **Required:** JTAG cable (6-pin)
- **Required:** DMA card with JTAG interface

---

## ?? Supported Hardware

### JTAG Adapters

#### ? **CH347 USB-JTAG** (Fully Tested)
- **VID/PID:** `1A86:55DD` or `1A86:55DE`
- **Driver:** USB HighSpeed-JTAG/I2C... CH347T
- **Speed:** Up to 10 MHz JTAG clock
- **Status:** ? Working perfectly
- **Where to buy:** AliExpress, eBay (search "CH347 JTAG")

#### ?? **RS232/FTDI Adapter** (Implemented, Not Tested)
- **VID/PID:** `0403:6011`
- **Driver:** FTDI D2XX
- **Speed:** Up to 10 MHz JTAG clock
- **Status:** ?? Code ready, needs testing
- **Where to buy:** FTDI official distributors

### FPGA Chips Supported
- ? **Xilinx Artix-7 XC7A35T** (33,280 logic cells)
- ? **Xilinx Artix-7 XC7A75T** (75,520 logic cells)
- ? **Xilinx Artix-7 XC7A100T** (101,440 logic cells)

**Other Xilinx 7-series FPGAs:** May work but not tested

---

## ?? Technology Stack

### Frontend
- **ImGui** - Immediate mode GUI library
- **GLFW** - Window and input management
- **OpenGL 3.3** - Graphics rendering
- **Custom Theme** - DMA Kings brand colors (Gold: `#D4AF37`)

### Backend
- **OpenOCD** - JTAG interface tool
- **PowerShell** - Windows driver management
- **WinAPI** - System operations

### Build Tools
- **Visual Studio 2022** - C++17 compiler
- **CMake** - Optional build system
- **Git** - Version control

---

## ?? Project Structure

```
DMATool/
?? src/                        # Source code
?  ?? Backend/
?  ?  ?? OpenOCDInterface.*    # FPGA detection & driver management
?  ?  ?? ProjectManager.*      # Project state
?  ?? UI/
?     ?? MainWindow.*          # Main window & splash
?     ?? Theme.*               # UI styling
?     ?? Tabs/
?        ?? JTAGPortTab.*      # DNA ID tab (COMPLETE)
?        ?? JTAGFlashTab.*     # Firmware flashing (TODO)
?        ?? DataPortTab.*      # Benchmarking (TODO)
?? vendor/                     # Third-party libraries (ImGui, GLFW, glad)
?? tools/ch347/drivers/        # CH347 driver files (complete package)
?? dmafiles/ch347/             # OpenOCD binaries & configs (embedded)
?? docs/                       # Documentation
?? DMATool.rc                  # Embedded resources
```

---

## ?? Contributing

We welcome contributions! Please follow these guidelines:

### Reporting Issues
1. Check [Troubleshooting](docs/TROUBLESHOOTING.md) first
2. Search existing [GitHub Issues](https://github.com/akwanmn/DMATool/issues)
3. Create new issue with:
   - DMATool version
   - OS version
   - Adapter type (CH347/RS232)
   - Console output
   - Steps to reproduce

### Submitting Pull Requests
1. Fork the repository
2. Create feature branch (`git checkout -b feature/YourFeature`)
3. Commit changes (`git commit -m 'Add YourFeature'`)
4. Push to branch (`git push origin feature/YourFeature`)
5. Open Pull Request

### Code Style
- **C++17** standard
- **4 spaces** indentation
- **PascalCase** for classes/methods
- **camelCase** for variables
- **Comments** for complex logic

---

## ?? License

This project is licensed under the **MIT License** - see [LICENSE](LICENSE) file for details.

---

## ?? Acknowledgments

### Third-Party Libraries
- **ImGui** - Omar Cornut (@ocornut)
- **GLFW** - GLFW Contributors
- **glad** - Dav1dde
- **OpenOCD** - OpenOCD Project

### Hardware Support
- **WCH (Nanjing Qinheng Microelectronics)** - CH347 adapter
- **FTDI** - FTDI adapters

### Community
- **DMA Kings** - Testing and feedback
- **Discord Members** - Bug reports and suggestions

---

## ?? Support & Community

### Get Help
- ?? **Discord (Fastest):** https://discord.gg/MfH9UHxkdP
- ?? **GitHub Issues:** https://github.com/akwanmn/DMATool/issues
- ?? **Documentation:** https://injectkings.gitbook.io/dma-kings

### Follow Us
- ?? **Website:** https://www.dmakings.com
- ?? **YouTube:** Coming soon
- ?? **Twitter:** Coming soon

---

## ??? Roadmap

### ? Completed
- [x] DNA ID tab with auto-detection
- [x] CH347 driver management
- [x] Smart driver validation
- [x] Auto-refresh after driver changes
- [x] Floating progress notifications
- [x] Copy DNA to clipboard
- [x] Custom splash screen
- [x] ImGui-based UI

### ?? In Progress
- [ ] RS232/FTDI adapter testing (waiting for hardware)

### ?? Planned
- [ ] Flash DMA tab (firmware programming)
- [ ] Benchmark DMA tab (performance testing)
- [ ] Multi-FPGA support
- [ ] Driver auto-update checker
- [ ] Configuration profiles
- [ ] Async operations (background threads)
- [ ] Progress bars for long operations
- [ ] Installer (Inno Setup/WiX)
- [ ] CI/CD pipeline (GitHub Actions)

---

## ?? Status

| Feature | Status | Notes |
|---------|--------|-------|
| DNA ID Tab | ? Complete | Fully tested with CH347 |
| CH347 Support | ? Tested | All features working |
| RS232/FTDI Support | ?? Untested | Code ready, needs hardware |
| Driver Installation | ? Working | Automatic via pnputil |
| Driver Uninstallation | ? Working | Automatic via pnputil |
| Auto-Detection | ? Working | Smart driver validation |
| Flash DMA Tab | ?? TODO | Planned for v1.1 |
| Benchmark DMA Tab | ?? TODO | Planned for v1.2 |

---

<p align="center">
  <b>Built with ?? by DMA Kings</b><br>
  <a href="https://www.dmakings.com">Website</a> •
  <a href="https://discord.gg/MfH9UHxkdP">Discord</a> •
  <a href="https://injectkings.gitbook.io/dma-kings">Docs</a>
</p>

<p align="center">
  <sub>© 2025 DMA Kings. All rights reserved.</sub>
</p>
