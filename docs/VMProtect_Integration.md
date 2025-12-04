# VMProtect Integration Guide for DMATool

## Overview

This document explains how VMProtect SDK has been integrated into DMATool to protect proprietary code from reverse engineering using tools like IDA Pro and Hex-Rays decompiler.

---

## What is Protected

### ?? Protection Levels

The codebase uses three protection levels strategically:

1. **Ultra** (Maximum Security - 30-50% slowdown)
   - Entry points (`WinMain`, `Application::Initialize`)
   - Most critical initialization logic
   
2. **Virtualization** (Strong Security - 20-40% slowdown)
   - LeechCore DMA initialization
   - Benchmark test execution
   - Throughput test algorithms
   
3. **Mutation** (Good Security - 10-20% slowdown)
   - DMA read operations
   - Flash programming/verification
   - JTAG command execution
   - Rating calculation algorithm
   - SHA256 hashing

### ?? What is NOT Protected

- UI rendering code (would cause lag)
- ImGui library code (third-party)
- DirectX 11 rendering (third-party)
- PCILeech/LeechCore libraries (open source)
- Resource extraction helpers (not critical)

---

## Files Modified

### Core Files

| File | Protection Added | Purpose |
|------|------------------|---------|
| `src/VMProtectConfig.h` | ? Created | Central protection configuration |
| `src/main.cpp` | ? Updated | Already has VMP markers (kept) |
| `src/Application.cpp` | ? Updated | Added Ultra protection to Init |
| `src/Backend/BenchmarkInterface.cpp` | ? Updated | Protected benchmark algorithms |
| `src/Backend/LeechCoreWrapper.cpp` | ? Updated | Protected DMA initialization |
| `src/Backend/FlashInterface.cpp` | ? Updated | Protected flash operations |
| `src/Backend/OpenOCDInterface.cpp` | ? Updated | Protected JTAG operations |
| `DMATool.vmp` | ? Created | VMProtect project file |

### Project Configuration

| File | Change | Purpose |
|------|--------|---------|
| `DMATool.vcxproj` | ? Updated | Added VMProtect SDK paths and linker |
| `vendor/VMProtectSDK/` | ? Created | SDK header and library files |

---

## Build Process

### 1. Debug Builds (No Protection)

```powershell
# Standard debug build - markers are disabled via preprocessor
msbuild DMATool.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64
```

**Result**: No VMProtect overhead, fast iteration for development.

### 2. Release Builds (With SDK Markers)

```powershell
# Build release with protection markers compiled in
msbuild DMATool.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

**Result**: `bin\Release-x64\DMATool.exe` with protection markers embedded.

### 3. Apply VMProtect Protection

```powershell
# Option A: Use VMProtect GUI
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" "DMATool.vmp"

# Option B: Use command-line
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" DMATool.vmp -pf
```

**Result**: Creates `bin\Release-x64\DMATool.vmp.exe` (protected version).

---

## How Protection Works

### Conditional Compilation

```cpp
// src/VMProtectConfig.h
#if defined(NDEBUG) && defined(_M_X64)
    #define VMPROTECT_ENABLED
    #include "../vendor/VMProtectSDK/VMProtectSDK.h"
#else
    // Debug: Empty macros (zero overhead)
    #define VMProtectBeginUltra(x)
    #define VMProtectEnd()
#endif
```

### Protection Markers in Code

```cpp
// Example: Application::Initialize() with Ultra protection
bool Application::Initialize()
{
    VMPROTECT_ULTRA_FUNCTION("AppInitialize");
    
    // Critical initialization code here
    // In Debug: runs normally
    // In Release after VMProtect: virtualized
    
    VMPROTECT_END_FUNCTION();
    return true;
}
```

### What Reverse Engineers See

**Without Protection**:
```assembly
; Clear assembly showing logic
mov eax, [password]
cmp eax, 0x12345678
je valid_license
```

**With Ultra Protection**:
```assembly
; VM bytecode garbage
lea rax, [rip+0x1A3C]
call vm_handler_0x42
db 0x8A, 0x3C, 0x5D, 0x2E  ; Random VM opcodes
vm_enter_context_0x89
; IDA Pro shows: ???
```

---

## Testing the Protection

### 1. Test Debug Build

```powershell
# Should run at full speed (no protection)
.\bin\Debug-x64\DMATool.exe
```

? **Expected**: Fast performance, no slowdown.

### 2. Test Release Build (Before VMProtect)

```powershell
# Has markers but not protected yet
.\bin\Release-x64\DMATool.exe
```

? **Expected**: Slightly slower but functional.

### 3. Test Protected Build

```powershell
# After running through VMProtect
.\bin\Release-x64\DMATool.vmp.exe
```

? **Expected**: 10-30% slower overall, but key functions protected.

### 4. Verify Protection with IDA Pro

Open `DMATool.vmp.exe` in IDA Pro:

- ? **Before**: Clear decompiled code
- ? **After**: VM handlers, garbage opcodes, no readable logic

---

## Performance Impact

| Component | Protection Level | Expected Slowdown |
|-----------|------------------|-------------------|
| App Initialization | Ultra | 30-50% (one-time) |
| Benchmark Tests | Virtualization | 20-40% |
| DMA Operations | Mutation | 10-20% |
| Flash Programming | Mutation | 10-20% |
| JTAG Operations | Mutation | 10-20% |
| **Overall App** | **Mixed** | **~15-25%** |

**Note**: Slowdown is acceptable because:
- Init happens once
- Tests run for minutes (users won't notice)
- DMA/Flash are hardware-limited anyway

---

## Important Notes

### ?? Do NOT Protect

Never add protection markers to:

1. **UI Code** (causes visible lag):
   ```cpp
   void RenderFrame() {
       // NO VMProtect markers here!
       ImGui::Begin("Window");
       ImGui::End();
   }
   ```

2. **Tight Loops** (massive slowdown):
   ```cpp
   for (int i = 0; i < 1000000; i++) {
       // NO VMProtect markers here!
       ProcessItem(i);
   }
   ```

3. **Frequently Called Functions** (>1000 calls/sec):
   ```cpp
   int GetCount() {
       // NO VMProtect markers here!
       return m_count;
   }
   ```

### ? DO Protect

Always protect:

1. **Proprietary Algorithms**:
   ```cpp
   void CalculateRating() {
       VMPROTECT_MUTATE_BLOCK("Rating");
       // Your unique rating logic
       VMPROTECT_END_BLOCK();
   }
   ```

2. **License Validation**:
   ```cpp
   bool CheckLicense() {
       VMPROTECT_ULTRA_FUNCTION("License");
       // License checking code
       VMPROTECT_END_FUNCTION();
   }
   ```

3. **Critical Entry Points**:
   ```cpp
   int WINAPI WinMain(...) {
       VMPROTECT_VIRTUALIZE_BLOCK("Main");
       // Startup code
       VMPROTECT_END_BLOCK();
   }
   ```

---

## Distribution

### Final Protected Build

```powershell
# 1. Clean rebuild
Remove-Item -Recurse -Force bin\Release-x64\* -ErrorAction SilentlyContinue
msbuild DMATool.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64

# 2. Apply VMProtect
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" DMATool.vmp -pf

# 3. Rename protected exe
Move-Item bin\Release-x64\DMATool.vmp.exe bin\Release-x64\DMATool_Protected.exe

# 4. Test protected exe
.\bin\Release-x64\DMATool_Protected.exe
```

### What to Ship

```
DMATool_v1.0/
  ? DMATool.exe (protected version)
  ? README.md
  ? LICENSE.txt
```

**Do NOT ship**:
- ? Original unprotected exe
- ? .vmp project file
- ? .pdb debug symbols
- ? Source code

---

## Troubleshooting

### Problem: Build fails with "VMProtectSDK.h not found"

**Solution**: Verify paths in `DMATool.vcxproj`:
```xml
<AdditionalIncludeDirectories>$(SolutionDir)vendor\VMProtectSDK;...</AdditionalIncludeDirectories>
```

### Problem: Linker error "unresolved external symbol VMProtectBegin"

**Solution**: Check linker dependencies:
```xml
<AdditionalDependencies>$(SolutionDir)vendor\VMProtectSDK\VMProtectSDK64.lib;...</AdditionalDependencies>
```

### Problem: Protected exe crashes on startup

**Solution**: 
1. Check for nested markers (not allowed):
   ```cpp
   // ? BAD
   VMProtectBeginUltra("Outer");
       VMProtectBeginMutation("Inner");  // CRASH!
       VMProtectEnd();
   VMProtectEnd();
   
   // ? GOOD
   VMProtectBeginUltra("First");
   DoSomething();
   VMProtectEnd();
   
   VMProtectBeginMutation("Second");
   DoSomethingElse();
   VMProtectEnd();
   ```

2. Remove markers from constructors/destructors:
   ```cpp
   // ? BAD
   MyClass() {
       VMProtectBeginUltra("Ctor");  // Can cause issues
       // ...
       VMProtectEnd();
   }
   ```

### Problem: App is too slow after protection

**Solution**: Reduce protection levels:
- Change `Ultra` ? `Virtualization`
- Change `Virtualization` ? `Mutation`
- Remove markers from frequently called code

---

## Security Recommendations

1. **Keep .vmp file private** - Don't commit to public repos
2. **Test protected exe thoroughly** - All features must work
3. **Don't over-protect** - 80% of code doesn't need protection
4. **Update VMProtect regularly** - New versions have better protection
5. **Monitor for cracks** - Check forums/sites periodically

---

## Next Steps

### For Development

1. Work in Debug builds (no protection overhead)
2. Test features thoroughly
3. Only build Release+Protected for final distribution

### For Distribution

1. Build final Release exe
2. Run through VMProtect
3. Test protected exe thoroughly
4. Ship protected exe only
5. Keep unprotected exe and .vmp file secure

---

## Summary

? **SDK Integrated**: All files modified, project configured  
? **Protection Applied**: 8 key functions protected  
? **Build Working**: Debug (fast), Release (protected)  
? **Documentation Complete**: This guide covers everything  

**You're ready to ship a protected DMATool.exe!**
