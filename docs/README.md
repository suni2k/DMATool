# DMATool Documentation

## Quick Start Guides

### For Users
- **HOW_TO_USE_VMPROTECT.md** - How to protect your exe with VMProtect

### For Developers
- **VMProtect_Integration.md** - Complete VMProtect integration guide
- **VMProtect_SDK_Complete_Guide.md** - SDK usage reference

## Archived Documentation

The rchive/ folder contains historical documentation from the development process:
- Build fixes
- Resource embedding fixes
- Benchmark fixes
- Driver integration fixes

These are kept for reference but are no longer needed for day-to-day development.

## Current Project Structure

`
DMATool/
??? src/                    # Source code
?   ??? VMProtectConfig.h   # Protection configuration
?   ??? Backend/            # Core logic (protected)
?   ??? UI/                 # Interface (not protected)
??? scripts/                # PowerShell automation
?   ??? Protect-GUI.ps1     # Apply VMProtect (recommended)
?   ??? Verify-Protection.ps1 # Verify protection worked
?   ??? archive/            # Old scripts
??? docs/                   # Documentation
?   ??? archive/            # Historical docs
??? bin/Release-x64/
?   ??? DMATool.exe         # Unprotected (don't ship)
?   ??? DMATool.vmp.exe     # Protected (ship this!)
??? DMATool.vmp             # VMProtect project file

`

## Protection Summary

**Protected Components** (12 functions):
- Application initialization (Ultra)
- Benchmark algorithms (Virtualization)
- DMA operations (Virtualization/Mutation)
- Flash programming (Mutation)
- JTAG operations (Mutation)

**Unprotected** (intentionally):
- UI rendering
- ImGui library
- DirectX 11
- Third-party DLLs

## Quick Commands

**Protect the exe**:
`powershell
.\scripts\Protect-GUI.ps1
`

**Verify protection**:
`powershell
.\scripts\Verify-Protection.ps1
`

**Test protected exe**:
`powershell
.\bin\Release-x64\DMATool.vmp.exe
`

## Distribution

Ship ONLY: DMATool.vmp.exe (rename to DMATool.exe)

DO NOT ship:
- Original DMATool.exe
- DMATool.vmp project file
- .pdb debug files
