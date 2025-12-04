#pragma once

// VMProtect SDK Integration for DMATool
// 
// This header provides conditional compilation of VMProtect SDK markers:
// - In RELEASE builds: Full protection via VMProtect SDK
// - In DEBUG builds: Markers disabled for fast iteration
//
// Protection Strategy:
// - Ultra: Entry points, license validation, critical algorithms
// - Virtualization: DMA operations, flash programming, JTAG commands
// - Mutation: Helper functions, utilities
//
// Performance Impact:
// - Ultra: 30-50% slowdown (use sparingly)
// - Virtualization: 20-40% slowdown (important code)
// - Mutation: 10-20% slowdown (less critical code)

// Only enable VMProtect in RELEASE builds with x64 architecture
#if defined(NDEBUG) && defined(_M_X64)
    #define VMPROTECT_ENABLED
    #include "../vendor/VMProtectSDK/VMProtectSDK.h"
#else
    // Debug builds: Define empty macros for zero overhead
    #define VMProtectBeginUltra(x)
    #define VMProtectBeginVirtualization(x)
    #define VMProtectBeginMutation(x)
    #define VMProtectBeginUltraLockByKey(x)
    #define VMProtectEnd()
    #define VMProtectIsDebuggerPresent() (false)
    #define VMProtectIsVirtualMachinePresent() (false)
    #define VMProtectIsValidImageCRC() (true)
#endif

// Helper macros for common protection patterns

// Protect an entire function with Ultra (maximum security)
// Usage: VMPROTECT_ULTRA_FUNCTION("FunctionName")
#define VMPROTECT_ULTRA_FUNCTION(name) VMProtectBeginUltra(name)
#define VMPROTECT_END_FUNCTION() VMProtectEnd()

// Protect a code block with Virtualization (strong security)
// Usage: { VMPROTECT_VIRTUALIZE_BLOCK("BlockName") ... VMPROTECT_END_BLOCK() }
#define VMPROTECT_VIRTUALIZE_BLOCK(name) VMProtectBeginVirtualization(name)
#define VMPROTECT_END_BLOCK() VMProtectEnd()

// Protect a code block with Mutation (good security, faster)
// Usage: { VMPROTECT_MUTATE_BLOCK("BlockName") ... VMPROTECT_END_BLOCK() }
#define VMPROTECT_MUTATE_BLOCK(name) VMProtectBeginMutation(name)

// Anti-debugging checks (returns true if debugger detected)
#define VMPROTECT_CHECK_DEBUGGER() VMProtectIsDebuggerPresent()

// Anti-VM checks (returns true if running in VM)
#define VMPROTECT_CHECK_VM() VMProtectIsVirtualMachinePresent()

// Image integrity check (returns true if exe is unmodified)
#define VMPROTECT_CHECK_INTEGRITY() VMProtectIsValidImageCRC()

// Protection Guidelines:
//
// DO PROTECT:
// - Entry points (WinMain, Initialize)
// - License validation logic
// - Critical algorithms (DMA operations, flash programming)
// - JTAG command processing
// - Resource extraction logic
// - LeechCore initialization
//
// DO NOT PROTECT:
// - UI rendering code (causes lag)
// - Tight loops (massive slowdown)
// - Frequently called functions (>1000 calls/sec)
// - Third-party library code (ImGui, DirectX)
// - Logging/debugging code
//
// MARKER PLACEMENT:
// ? GOOD: One marker per logical function/operation
// ? BAD:  Nested markers (causes crashes)
// ? BAD:  Markers inside loops
// ? BAD:  Markers in constructors/destructors (can cause issues)
//
// EXAMPLE USAGE:
//
// bool InitializeCriticalSystem() {
//     VMPROTECT_ULTRA_FUNCTION("InitCritical");
//     
//     // Critical initialization code here
//     
//     VMPROTECT_END_FUNCTION();
//     return true;
// }
//
// void ProcessDMAOperation() {
//     VMPROTECT_VIRTUALIZE_BLOCK("DMAOp");
//     
//     // DMA processing code
//     
//     VMPROTECT_END_BLOCK();
// }
//
// void HelperFunction() {
//     VMPROTECT_MUTATE_BLOCK("Helper");
//     
//     // Less critical helper code
//     
//     VMPROTECT_END_BLOCK();
// }
