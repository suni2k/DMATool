# DMA Benchmarking Documentation - Summary

## ? What We've Created

We've built comprehensive documentation for DMA performance testing and benchmarking based on:
1. **PCILeech** by Ulf Frisk
2. Your existing DMA testing tool features
3. Industry best practices

---

## ? Documents Created

### 1. **DMA_BENCHMARKING_GUIDE.md** ?? **Main Guide**
**Purpose:** Complete guide for customers to test their DMA cards

**Contents:**
- 5 test types (Quick Speed, Custom, Throughput, Mixed, Stress)
- Performance benchmarks and classifications
- Troubleshooting guide
- PCILeech command reference
- Memory enumeration guide
- Expected results for XC7A100T, XC7A75T, XC7A35T

**Use:** Customer-facing documentation

---

### 2. **BENCHMARK_TAB_SPEC.md** ?? **Technical Spec**
**Purpose:** Implementation specification for developers

**Contents:**
- UI design mockup
- Class architecture
- PCILeech integration code
- Output parsing logic
- File export formats (TXT, CSV, JSON)
- Implementation checklist with phases

**Use:** Development reference

---

### 3. **DMA_TESTING_GUIDE.md** ? **Quick Start**
**Purpose:** Step-by-step manual testing guide

**Contents:**
- PCILeech download and setup
- 6 essential tests
- Troubleshooting checklist
- Quick reference commands

**Use:** First-time users, troubleshooting

---

## ? Test Types Overview

### Quick Speed Test ?
- **Duration:** 10 seconds
- **Measures:** RPS (Reads Per Second)
- **Expected:** 600-700 RPS for XC7A100T
- **Use:** Fast validation

### Throughput Test ??
- **Duration:** ~7 seconds (1GB transfer)
- **Measures:** MB/s
- **Expected:** 140-160 MB/s (USB3/FT601)
- **Use:** USB/adapter validation

### Mixed Test ??
- **Duration:** ~20 seconds (Speed + Throughput)
- **Measures:** Both RPS and MB/s
- **Expected:** Combined metrics
- **Use:** Comprehensive validation

### Stress Test ??
- **Duration:** 60-3600 seconds
- **Measures:** Stability over time
- **Expected:** 0% error rate
- **Use:** Long-term reliability testing

### Custom Test ??
- **Duration:** User-defined
- **Measures:** User-configured
- **Expected:** Varies
- **Use:** Advanced troubleshooting

---

## ?? Performance Benchmarks

### XC7A100T (Your Card)
| Metric | Elite | Amazing | Good | Warning | Low |
|--------|-------|---------|------|---------|-----|
| **RPS** | >7,500 | 6,500-7,500 | 5,200-6,500 | 4,000-5,200 | <4,000 |
| **Throughput** | >220 MB/s | 200-220 | 150-200 | 125-150 | <125 |
| **Latency** | <130 탎 | 130-150 | 150-190 | 190-250 | >250 |

### Classification
- **ELITE:** Top 1% performance - competition-grade hardware ?
- **AMAZING:** Top 5% performance - excellent for professional use ??
- **GOOD:** Above average - suitable for most applications ?
- **WARNING:** Below expected - check connections/drivers ??
- **LOW:** Issue present - troubleshooting required ?

---

## ?? PCILeech Commands Reference

### Essential Commands:
```powershell
# 1. Probe device
pcileech.exe probe

# 2. Quick benchmark
pcileech.exe benchmark

# 3. Memory display (visual test)
pcileech.exe display -min 0x1000 -max 0x2000

# 4. Stability test (100 iterations)
pcileech.exe testmemread -min 0x1000

# 5. Throughput test (dump 1MB)
pcileech.exe dump -min 0x1000 -max 0x101000 -out test.bin
```

### Advanced:
```powershell
# Algorithm selection
pcileech.exe benchmark -device fpga://algo=1  # Async Normal
pcileech.exe benchmark -device fpga://algo=2  # Async Tiny

# Verbose output
pcileech.exe benchmark -v -vv -vvv
```

---

## ?? Troubleshooting Quick Reference

### Low RPS (<400)
```
1. Check USB cable (USB 3.0 blue port)
2. Try different FPGA algorithm
3. Delete mmap.txt, regenerate
4. Verify BIOS settings (disable IOMMU/VT-d)
```

### Low Throughput (<80 MB/s)
```
1. Verify USB 3.0 connection
2. Update FTDI drivers
3. Try different PCIe slot
4. Check cable quality
```

### Intermittent Failures
```
1. Check power connections
2. Re-flash FPGA firmware
3. Disable memory protection (BIOS)
4. Test with different memory ranges
```

---

## ?? Next Steps for DMATool

### Phase 1: Setup PCILeech Integration
```cpp
// Create BenchmarkInterface class
namespace DMATool::Backend
{
    class BenchmarkInterface
    {
        bool StartTest(BenchmarkConfig config);
        BenchmarkResults GetResults();
        void StopTest();
    };
}
```

### Phase 2: Build UI
```
? Test Controls Panel (left)
? Results Display Panel (right)
? Console Log (bottom)
? Real-time graphing (ImPlot)
```

### Phase 3: Implement Tests
```
1. Quick Speed Test
2. Throughput Test
3. Mixed Test
4. Stress Test
5. Custom Test
```

### Phase 4: Export Features
```
? Text report (.txt)
? CSV export (.csv)
? JSON export (.json)
? PDF generation (optional)
```

---

## ?? File Locations

All documentation is in `docs/`:

```
docs/
?? DMA_BENCHMARKING_GUIDE.md    ? Main customer guide
?? BENCHMARK_TAB_SPEC.md        ? Technical specification
?? DMA_TESTING_GUIDE.md         ? Quick start manual
?? DNA_ID_TAB_COMPLETE.md       ? DNA ID tab docs
?? ARCHITECTURE.md              ? Project structure
?? BUILD_INSTRUCTIONS.md        ? Build guide
?? TROUBLESHOOTING.md           ? Common issues
```

Helper scripts in `scripts/`:
```
scripts/
?? Setup-PCILeech.ps1           ? Auto-download PCILeech
```

---

## ? PCILeech Setup

We've created an auto-setup script:

```powershell
# Run this to download PCILeech
.\scripts\Setup-PCILeech.ps1
```

**Installs to:** `C:\Tools\PCILeech\`

**Includes:**
- pcileech.exe
- All required DLLs
- Configuration files

---

## ?? Ready to Use

### For Customers:
1. Read `DMA_BENCHMARKING_GUIDE.md`
2. Run `Setup-PCILeech.ps1`
3. Follow `DMA_TESTING_GUIDE.md` steps
4. Validate DMA card performance

### For Developers:
1. Read `BENCHMARK_TAB_SPEC.md`
2. Review class architecture
3. Implement Phase 1 (PCILeech integration)
4. Build UI according to design mockup
5. Add export functionality

---

## ?? Testing Your XC7A100T

### Expected Results:
```
Quick Speed Test: 6,000-7,000 RPS (AMAZING)
Throughput Test: 200-220 MB/s (AMAZING)
Mixed Test: Both metrics AMAZING
Stress Test: 0% error rate
```

### Performance Targets by Rating:

**Elite (Top 1%):**
- RPS: >7,500
- Throughput: >220 MB/s
- Latency: <130 탎

**Amazing (Top 5%):**
- RPS: 6,500-7,500
- Throughput: 200-220 MB/s
- Latency: 130-150 탎

**Good (Above Average):**
- RPS: 5,200-6,500
- Throughput: 150-200 MB/s
- Latency: 150-190 탎

**Warning (Below Expected):**
- RPS: 4,000-5,200
- Throughput: 125-150 MB/s
- Latency: 190-250 탎

**Low (Needs Fixing):**
- RPS: <4,000
- Throughput: <125 MB/s
- Latency: >250 탎

### If Results Are Poor:
1. Check [Troubleshooting Guide](docs/DMA_BENCHMARKING_GUIDE.md#troubleshooting-guide)
2. Verify USB 3.0 connection
3. Update FTDI drivers
4. Test different PCIe slot
5. Ask on Discord: https://discord.gg/MfH9UHxkdP

---

## ?? Summary

? **Documentation Complete:**
- Comprehensive benchmarking guide
- Technical implementation spec
- Quick start manual
- Auto-setup script

? **Ready for:**
- Customer testing
- Development
- Integration into DMATool

? **Tools Provided:**
- PCILeech setup script
- Command reference
- Troubleshooting guide

**Next:** Start implementing Benchmark DMA tab following `BENCHMARK_TAB_SPEC.md`!

---

**Created:** January 2025  
**For:** DMATool v1.0+  
**Based on:** PCILeech v4.19 + Industry Best Practices
