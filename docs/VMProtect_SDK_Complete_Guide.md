# VMProtect SDK Complete Guide for C++
## Understanding Code Protection with Markers

---

## 📚 Table of Contents
1. [What Are VMProtect Markers?](#what-are-vmprotect-markers)
2. [How Markers Work](#how-markers-work)
3. [Available Protection Types](#available-protection-types)
4. [SDK Integration Steps](#sdk-integration-steps)
5. [Real-World Examples](#real-world-examples)
6. [Best Practices](#best-practices)
7. [Common Mistakes](#common-mistakes)
8. [Performance Considerations](#performance-considerations)

---

## 🎯 What Are VMProtect Markers?

**VMProtect markers** are special function calls you insert directly into your C++ source code to tell VMProtect **which code blocks to protect** and **how strongly to protect them**.

### The Problem They Solve:
- ❌ **GUI-only protection** doesn't know which functions are critical
- ❌ **CLI protection** can only apply generic protection
- ✅ **SDK markers** let YOU decide exactly what to protect

### Think of Markers Like This:
```cpp
// Your normal code
void MyFunction() {
    int x = 5;
    
    // 🛡️ Start protection HERE
    VMProtectBeginUltra("CriticalAlgorithm");
    
    // This code becomes VIRTUALIZED + MUTATED
    int secretKey = GenerateLicenseKey();
    ValidateLicense(secretKey);
    
    // 🛡️ End protection HERE
    VMProtectEnd();
    
    // Back to normal code
    return;
}
```

**Result:** Only the code between the markers gets protected. The rest runs at full speed.

---

## 🔧 How Markers Work

### Step 1: You Add Markers to Source Code
```cpp
#include "VMProtectSDK.h"

void LoginFunction() {
    VMProtectBeginUltra("LoginProtection");
    // Check username/password
    VMProtectEnd();
}
```

### Step 2: Compile Normally
- The marker functions **do nothing** during normal compilation
- They're just placeholders with no runtime cost
- Your app runs normally in Debug mode

### Step 3: VMProtect Reads Markers
- When you run VMProtect on your compiled EXE:
  - It finds all `VMProtectBegin...` markers
  - It transforms the code between Begin/End
  - It **removes** the marker calls from final EXE

### Step 4: Protected EXE
- Marked code is now virtualized/mutated
- Markers are gone (can't be found by reverse engineers)
- Protected code runs inside VMProtect's custom VM

---

## 🛡️ Available Protection Types

VMProtect SDK provides **6 main marker types**:

### 1. **VMProtectBegin(name)**
```cpp
VMProtectBegin("MyFunction");
// Code here
VMProtectEnd();
```
- **Protection:** Whatever you set in VMProtect GUI
- **Use case:** When you want GUI control
- **Strength:** Depends on GUI settings

---

### 2. **VMProtectBeginVirtualization(name)** ⭐ RECOMMENDED
```cpp
VMProtectBeginVirtualization("Algorithm");
// Code here
VMProtectEnd();
```
- **Protection:** Converts code to custom VM bytecode
- **What it does:**
  - Original instructions → Custom VM opcodes
  - IDA Pro/Hex-Rays shows garbage
  - Requires reverse engineer to rebuild your VM
- **Strength:** 8/10
- **Performance:** 20-40% slower
- **Use for:** 
  - License validation
  - Encryption algorithms
  - Core business logic
  - API keys / secrets

---

### 3. **VMProtectBeginMutation(name)**
```cpp
VMProtectBeginMutation("Obfuscated");
// Code here
VMProtectEnd();
```
- **Protection:** Replaces instructions with equivalent complex ones
- **What it does:**
  - `x = 5` becomes `x = (7 + 3) - 5` + junk code
  - Control flow becomes spaghetti
  - Dead code, fake branches added
- **Strength:** 6/10
- **Performance:** 10-20% slower
- **Use for:**
  - Non-critical functions
  - When you need better performance
  - Helper functions

---

### 4. **VMProtectBeginUltra(name)** ⭐⭐ MAXIMUM PROTECTION
```cpp
VMProtectBeginUltra("TopSecret");
// Code here
VMProtectEnd();
```
- **Protection:** Virtualization + Mutation combined
- **What it does:**
  - First mutates the code (obfuscates)
  - Then virtualizes the mutated code
  - Double-layer protection
- **Strength:** 10/10 - MAXIMUM
- **Performance:** 30-50% slower
- **Use for:**
  - License key generation
  - Anti-cheat detection
  - Proprietary algorithms
  - Anything you REALLY don't want reversed

---

### 5. **VMProtectBeginVirtualizationLockByKey(name)**
```cpp
VMProtectBeginVirtualizationLockByKey("PaidFeature");
// Code here - only runs if valid license
VMProtectEnd();
```
- **Protection:** Virtualization + License check
- **What it does:**
  - Code only runs if valid serial number activated
  - Without license, code won't execute
- **Strength:** 9/10
- **Use for:** Paid features, premium modules

---

### 6. **VMProtectBeginUltraLockByKey(name)**
```cpp
VMProtectBeginUltraLockByKey("ProVersion");
// Code here - only runs if valid license
VMProtectEnd();
```
- **Protection:** Ultra + License check
- **Strength:** 10/10 - MAXIMUM + License
- **Use for:** Highest-tier paid features

---

### 7. **VMProtectEnd()**
```cpp
VMProtectEnd();
```
- **REQUIRED:** Marks the end of EVERY protected block
- ⚠️ **CRITICAL:** Always pair with a Begin marker!

---

## 🚀 SDK Integration Steps

### Step 1: Copy SDK Files to Your Project

Create this folder structure:
```
DMATool/
├── vendor/
│   └── VMProtectSDK/
│       ├── include/
│       │   └── VMProtectSDK.h
│       └── lib/
│           ├── VMProtectSDK32.lib
│           ├── VMProtectSDK64.lib
│           ├── VMProtectSDK32.dll
│           └── VMProtectSDK64.dll
```

**Copy from:**
- Header: `C:\Program Files\VMProtect Ultimate\Include\C\VMProtectSDK.h`
- Libraries: `C:\Program Files\VMProtect Ultimate\Lib\Windows\`

---

### Step 2: Update Visual Studio Project

**Add to `DMATool.vcxproj`:**

```xml
<PropertyGroup>
  <!-- Add SDK include path -->
  <IncludePath>$(ProjectDir)vendor\VMProtectSDK\include;$(IncludePath)</IncludePath>
  
  <!-- Add SDK lib path -->
  <LibraryPath>$(ProjectDir)vendor\VMProtectSDK\lib;$(LibraryPath)</LibraryPath>
</PropertyGroup>

<!-- OR the SDK header auto-links the lib, so you may not need AdditionalDependencies -->
```

---

### Step 3: Add Markers to Your Code

**Example for your DMATool:**

```cpp
// src/main.cpp
#include "Application.h"
#include <memory>
#include <Windows.h>

// Add this include
#include "VMProtectSDK.h"

#ifdef NDEBUG  // Release build only
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    // Protect the entry point
    VMProtectBeginUltra("EntryPoint");
    
    auto app = std::make_unique<DMATool::Application>("DMATool", 1400, 900);
    
    if (!app->Initialize())
    {
        VMProtectEnd();
        return -1;
    }
    
    VMProtectEnd();
    
    app->Run();
    app->Shutdown();
    
    return 0;
}
#endif
```

---

### Step 4: Compile Your Project

Build as normal:
```powershell
MSBuild DMATool.vcxproj /p:Configuration=Release /p:Platform=x64
```

**What happens:**
- SDK markers are compiled in (as no-op functions)
- Your exe now has marker "signatures" VMProtect will find
- App runs normally (markers do nothing in debug/development)

---

### Step 5: Protect with VMProtect

**Option A: Use GUI**
```powershell
& "C:\Program Files\VMProtect Ultimate\VMProtect.exe" "bin\Release-x64\DMATool.exe"
```
- VMProtect automatically detects markers
- Shows them in "Functions" tab with your custom names
- Click "Compile" - markers get transformed and removed

**Option B: Use CLI (if you have a .vmp project)**
```powershell
& "C:\Program Files\VMProtect Ultimate\VMProtect_Con.exe" "DMATool.vmp"
```

---

## 💡 Real-World Examples

### Example 1: Protect License Validation
```cpp
#include "VMProtectSDK.h"

bool ValidateLicense(const std::string& key) {
    VMProtectBeginUltra("LicenseCheck");
    
    // This code is now IMPOSSIBLE to reverse
    bool valid = false;
    
    if (key.length() == 20) {
        int checksum = 0;
        for (char c : key) {
            checksum += (c * 7) ^ 0xAB;
        }
        valid = (checksum == 0x1234ABCD);
    }
    
    VMProtectEnd();
    return valid;
}
```

**Result:** Reverse engineers see VM bytecode garbage, not your algorithm.

---

### Example 2: Protect Critical Backend Function
```cpp
// src/Backend/OpenOCDInterface.cpp
#include "VMProtectSDK.h"

bool OpenOCDInterface::FlashFirmware(const std::string& file) {
    VMProtectBeginVirtualization("FlashFirmware");
    
    // Your proprietary flashing algorithm
    // This code is now virtualized
    
    VMProtectEnd();
    return true;
}
```

---

### Example 3: Protect Entire Class
```cpp
// src/Backend/LeechCoreWrapper.cpp
#include "VMProtectSDK.h"

class LeechCoreWrapper {
public:
    void Initialize() {
        VMProtectBeginUltra("LeechCoreInit");
        
        // Protected initialization
        
        VMProtectEnd();
    }
    
    void ReadMemory() {
        VMProtectBeginVirtualization("LeechCoreRead");
        
        // Protected memory read
        
        VMProtectEnd();
    }
    
    void WriteMemory() {
        VMProtectBeginVirtualization("LeechCoreWrite");
        
        // Protected memory write
        
        VMProtectEnd();
    }
};
```

---

### Example 4: Mixed Protection (Performance Optimized)
```cpp
void ProcessData() {
    // Fast code - no protection
    std::vector<int> data = LoadData();
    
    // Critical algorithm - ULTRA protection
    VMProtectBeginUltra("Algorithm");
    int key = GenerateKey(data);
    VMProtectEnd();
    
    // More fast code
    SaveResult(key);
}
```

---

## ✅ Best Practices

### 1. **Name Your Markers Descriptively**
```cpp
// ❌ BAD
VMProtectBeginUltra("1");

// ✅ GOOD
VMProtectBeginUltra("LicenseValidation_CheckKey");
```
**Why:** Makes debugging easier, GUI shows meaningful names

---

### 2. **Always Pair Begin with End**
```cpp
// ❌ BAD - Missing End!
VMProtectBeginUltra("Test");
DoSomething();
// Forgot VMProtectEnd();

// ✅ GOOD
VMProtectBeginUltra("Test");
DoSomething();
VMProtectEnd();
```

---

### 3. **Don't Protect Large Code Blocks**
```cpp
// ❌ BAD - Too much protected = slow app
VMProtectBeginUltra("EverythingProtected");
// 1000 lines of code
VMProtectEnd();

// ✅ GOOD - Only protect critical parts
void MyFunction() {
    SetupVariables();  // Fast
    
    VMProtectBeginUltra("OnlyCriticalPart");
    int key = CriticalAlgorithm();
    VMProtectEnd();
    
    CleanupVariables();  // Fast
}
```

---

### 4. **Use Appropriate Protection Levels**
```cpp
// Critical: Ultra (maximum)
VMProtectBeginUltra("LicenseKey");

// Important: Virtualization (strong)
VMProtectBeginVirtualization("CoreLogic");

// Nice-to-have: Mutation (fast)
VMProtectBeginMutation("HelperFunction");
```

---

### 5. **Avoid Protecting Loops/Hot Paths**
```cpp
// ❌ BAD - Kills performance
for (int i = 0; i < 1000000; i++) {
    VMProtectBeginUltra("Loop");
    Process(i);
    VMProtectEnd();
}

// ✅ GOOD - Protect once outside loop
VMProtectBeginUltra("BatchProcess");
for (int i = 0; i < 1000000; i++) {
    Process(i);
}
VMProtectEnd();
```

---

### 6. **Don't Jump Into/Out of Protected Regions**
```cpp
// ❌ BAD - Undefined behavior
VMProtectBeginUltra("BadExample");
if (condition) goto outside_label;
VMProtectEnd();

outside_label:
    DoSomething();

// ✅ GOOD - Complete blocks only
VMProtectBeginUltra("GoodExample");
if (condition) {
    HandleError();
}
VMProtectEnd();
```

---

## ❌ Common Mistakes

### Mistake 1: Forgetting to Include SDK
```cpp
// ❌ ERROR: VMProtectBeginUltra not defined
VMProtectBeginUltra("Test");

// ✅ FIX: Add include
#include "VMProtectSDK.h"
VMProtectBeginUltra("Test");
```

---

### Mistake 2: Using Markers in Debug Build
```cpp
// ❌ BAD: Always protected, even in debug
VMProtectBeginUltra("Test");

// ✅ GOOD: Only protect in Release
#ifdef NDEBUG
    VMProtectBeginUltra("Test");
#endif
```

---

### Mistake 3: Protecting UI Rendering
```cpp
// ❌ BAD: Makes UI laggy
void RenderFrame() {
    VMProtectBeginUltra("Render");
    ImGui::Begin("Window");
    // UI code
    ImGui::End();
    VMProtectEnd();
}

// ✅ GOOD: Don't protect UI
void RenderFrame() {
    ImGui::Begin("Window");
    // UI code stays fast
    ImGui::End();
}
```

---

### Mistake 4: Nesting Markers
```cpp
// ❌ BAD: Don't nest!
VMProtectBeginUltra("Outer");
    VMProtectBeginVirtualization("Inner");  // Wrong!
    VMProtectEnd();
VMProtectEnd();

// ✅ GOOD: Sequential blocks
VMProtectBeginUltra("First");
DoSomething();
VMProtectEnd();

VMProtectBeginVirtualization("Second");
DoSomethingElse();
VMProtectEnd();
```

---

## ⚡ Performance Considerations

### Protection Impact on Speed:

| Protection Type | Slowdown | When to Use |
|----------------|----------|-------------|
| **None** | 0% | Development, non-critical |
| **Mutation** | 10-20% | Helper functions, utilities |
| **Virtualization** | 20-40% | Important algorithms |
| **Ultra** | 30-50% | Only most critical code |

### Real Example:
```cpp
// Original: 10ms execution time
void FastFunction() {
    Calculate();
}

// With Ultra: 15-20ms execution time
void SlowerFunction() {
    VMProtectBeginUltra("Protected");
    Calculate();
    VMProtectEnd();
}
```

### Optimization Strategy:
```cpp
void OptimizedFunction() {
    // Fast: Load data (no protection)
    auto data = LoadDataFromDisk();  // 0ms overhead
    
    // Slow: Process (Ultra protected)
    VMProtectBeginUltra("CriticalAlgorithm");
    int result = ProcessData(data);  // +50% overhead
    VMProtectEnd();
    
    // Fast: Save result (no protection)
    SaveResult(result);  // 0ms overhead
}
```

**Result:** Only 10% of function is protected, but critical part is secure.

---

## 📊 Protection Effectiveness vs IDA Pro

### Without SDK Markers (GUI only):
- ❌ IDA Pro: Can see most functions
- ❌ Hex-Rays: Produces readable pseudocode
- ❌ Protection: 5/10

### With SDK Markers (Ultra):
- ✅ IDA Pro: Shows VM bytecode garbage
- ✅ Hex-Rays: Output is meaningless
- ✅ Protection: 10/10

**Example of what IDA shows with Ultra protection:**
```assembly
; Instead of clear code:
mov eax, [password]
cmp eax, 0x12345678
je valid_license

; They see garbage like:
lea rax, [rip+0x1A3C]
call vm_handler_0x42
db 0x8A, 0x3C, 0x5D, 0x2E  ; VM bytecode
vm_enter_context_0x89
; ... hundreds of VM opcodes ...
```

---

## 📝 Summary Checklist

Before releasing your protected app:

- [ ] SDK files copied to project
- [ ] `VMProtectSDK.h` included in protected files
- [ ] Markers added to critical functions
- [ ] Each `VMProtectBegin` has matching `VMProtectEnd`
- [ ] Protected code doesn't include UI/rendering
- [ ] No markers in tight loops
- [ ] Tested protected exe works correctly
- [ ] Performance is acceptable
- [ ] Markers only active in Release build

---

## 🎯 Quick Reference Card

```cpp
#include "VMProtectSDK.h"

// Entry point protection
VMProtectBeginUltra("EntryPoint");
VMProtectEnd();

// License validation (maximum security)
VMProtectBeginUltra("License");
VMProtectEnd();

// Important algorithms (strong security)
VMProtectBeginVirtualization("Algorithm");
VMProtectEnd();

// Helper functions (good security, fast)
VMProtectBeginMutation("Helper");
VMProtectEnd();

// Paid features (requires license)
VMProtectBeginUltraLockByKey("ProFeature");
VMProtectEnd();
```

---

## 🔗 Additional Resources

- Official Docs: https://vmpsoft.com/vmprotect/user-manual/
- SDK Functions: https://vdown.cn/vmpsoft/en/support/user-manual/
- Your project file: `DMATool_Ultra.vmp`
- SDK location: `C:\Program Files\VMProtect Ultimate\Include\C\`

---

**Next Step:** I can help you add SDK markers to your DMATool code!
