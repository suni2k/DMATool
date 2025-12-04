# DMATool - DMA Hardware Interface Tool

A modern Windows application for DMA card management with JTAG, Flash, and Benchmark capabilities.

## ?? Quick Start

### Build Release
1. Open DMATool.sln in Visual Studio 2022
2. Set Configuration to **Release** | **x64**
3. Build ? Rebuild Solution

### Apply Protection
`powershell
.\scripts\Protect-GUI.ps1
`

### Test
`powershell
.\scripts\Verify-Protection.ps1
`

## ?? Documentation

See **docs/README.md** for complete documentation.

## ??? Protection

This project uses **VMProtect Ultimate** to protect proprietary code from reverse engineering.

- **Protected**: Core algorithms, DMA operations, flash programming, JTAG commands
- **Unprotected**: UI code, third-party libraries

## ?? Key Files

- src/VMProtectConfig.h - Protection configuration
- DMATool.vmp - VMProtect project file
- scripts/Protect-GUI.ps1 - Apply protection
- scripts/Verify-Protection.ps1 - Verify protection

## ?? Distribution

Ship: in\Release-x64\DMATool.vmp.exe (rename to DMATool.exe)

## ?? License

Copyright © 2025 - All Rights Reserved
