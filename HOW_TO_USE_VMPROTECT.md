# How to Apply VMProtect to DMATool

## Quick Start (Recommended)

Open PowerShell in your DMATool directory and run:

```powershell
.\scripts\Apply-VMProtect.ps1
```

This will:
1. ? Build Release configuration
2. ? Apply VMProtect protection
3. ? Create `DMATool.vmp.exe` (protected version)

---

## Step-by-Step Guide

### Option 1: Automatic (Use the Script)

```powershell
# Method 1: Build + Protect in one step (recommended)
.\scripts\Apply-VMProtect.ps1

# Method 2: Skip build if you already built Release
.\scripts\Apply-VMProtect.ps1 -SkipBuild

# Method 3: Open VMProtect GUI instead of command-line
.\scripts\Apply-VMProtect.ps1 -OpenGUI
```

### Option 2: Manual Steps

If you prefer to do it manually:

**Step 1: Build Release**
```powershell
msbuild DMATool.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

**Step 2: Apply VMProtect**
```powershell
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" DMATool.vmp -pf
```

**Step 3: Test Protected Exe**
```powershell
.\bin\Release-x64\DMATool.vmp.exe
```

---

## Understanding the Command

```powershell
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" DMATool.vmp -pf
```

Breaking it down:

| Part | Meaning |
|------|---------|
| `&` | PowerShell operator to execute a program with spaces in path |
| `"C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe"` | VMProtect Console executable (command-line version) |
| `DMATool.vmp` | Your VMProtect project file (contains protection settings) |
| `-pf` | "Protect File" - applies protection to the exe specified in the .vmp file |

---

## What Happens During Protection

### Input
- **File**: `bin\Release-x64\DMATool.exe` (unprotected)
- **Size**: ~4-6 MB
- **Protection**: None - readable in IDA Pro

### VMProtect Process
1. Reads `DMATool.vmp` project file
2. Finds protection markers in the compiled exe:
   - `VMProtectBeginUltra("AppInitialize")`
   - `VMProtectBeginVirtualization("BenchmarkTest")`
   - etc.
3. Converts marked code to VM bytecode
4. Packs/compresses the exe
5. Adds anti-debugging/anti-VM checks

### Output
- **File**: `bin\Release-x64\DMATool.vmp.exe` (protected)
- **Size**: ~4-7 MB (slightly larger due to VM code)
- **Protection**: ? Code unreadable in IDA Pro/Hex-Rays

---

## Verification

### Test the Protected Exe

```powershell
# Run the protected version
.\bin\Release-x64\DMATool.vmp.exe

# Test each feature:
# - JTAG Port tab (DNA ID reading)
# - Flash DMA tab (flash programming)
# - Data Port tab (benchmarks)
# - Driver management
```

### Check Protection (Optional)

Open `DMATool.vmp.exe` in IDA Pro:

**Before Protection** (DMATool.exe):
```assembly
; Clear, readable code
mov eax, [rdi+10h]
test eax, eax
jz short loc_12345
```

**After Protection** (DMATool.vmp.exe):
```assembly
; VM bytecode garbage
lea rax, [rip+1A3Ch]
call vm_handler_42h
db 8Ah, 3Ch, 5Dh, 2Eh
; Hex-Rays: "Cannot decompile"
```

? **If IDA Pro shows garbage/VM handlers, protection worked!**

---

## Troubleshooting

### Problem: "VMProtect not found"

**Solution**: Install VMProtect Ultimate from your license

```powershell
# Check if installed
Test-Path "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe"
```

### Problem: "DMATool.vmp not found"

**Solution**: The .vmp file should be in your solution root

```powershell
# Check if exists
Test-Path ".\DMATool.vmp"

# If missing, it was created during integration
# Location: C:\Users\suni\source\repos\DMATool\DMATool.vmp
```

### Problem: "Build failed"

**Solution**: Fix compilation errors first

```powershell
# Build and see errors
msbuild DMATool.sln /p:Configuration=Release /p:Platform=x64
```

### Problem: Protected exe crashes

**Possible causes**:
1. **Nested markers** - Check for VMProtect markers inside other markers
2. **Protected constructors** - Don't protect constructors/destructors
3. **Protected tight loops** - Remove markers from loops

**Solution**: Review `src/VMProtectConfig.h` guidelines

---

## Distribution

### Files to Ship

? **Ship this**: `DMATool.vmp.exe` (protected version)

### Files to Keep Private

? **DON'T ship**:
- `DMATool.exe` (unprotected - attackers can analyze this)
- `DMATool.vmp` (project file - contains your protection settings)
- `VMProtectSDK64.lib` (only needed for building)
- `.pdb` files (debug symbols)

### Recommended Distribution

```
DMATool_v1.0/
  ??? DMATool.exe  (rename DMATool.vmp.exe to DMATool.exe)
  ??? README.md
  ??? LICENSE.txt
```

**Important**: Rename `DMATool.vmp.exe` ? `DMATool.exe` for distribution so users just see "DMATool.exe"

---

## Performance Impact

After VMProtect, you may notice:

| Operation | Slowdown | Noticeable? |
|-----------|----------|-------------|
| App startup | ~30-40% | Barely (one-time cost) |
| Benchmark tests | ~20-30% | No (tests are already slow) |
| UI navigation | ~5-10% | No (barely measurable) |
| Flash operations | ~10-20% | No (hardware-limited anyway) |

**Overall**: ~15-25% slower, but **worth it** for protection!

---

## Quick Reference

### Common Commands

```powershell
# Full process (build + protect)
.\scripts\Apply-VMProtect.ps1

# Just protect (if already built)
.\scripts\Apply-VMProtect.ps1 -SkipBuild

# Open VMProtect GUI
.\scripts\Apply-VMProtect.ps1 -OpenGUI

# Manual command
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" DMATool.vmp -pf

# Test protected exe
.\bin\Release-x64\DMATool.vmp.exe
```

### File Locations

| File | Location | Purpose |
|------|----------|---------|
| Unprotected exe | `bin\Release-x64\DMATool.exe` | Original (don't ship) |
| Protected exe | `bin\Release-x64\DMATool.vmp.exe` | Protected (ship this) |
| VMProtect project | `DMATool.vmp` | Protection settings |
| VMProtect exe | `C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe` | Protection tool |

---

## Next Steps

1. **Run the script**:
   ```powershell
   .\scripts\Apply-VMProtect.ps1
   ```

2. **Test the protected exe** - Make sure all features work

3. **Ship `DMATool.vmp.exe`** - Your code is now protected!

**That's it!** Your proprietary DMA/Flash/JTAG algorithms are now safe from reverse engineering. ??
