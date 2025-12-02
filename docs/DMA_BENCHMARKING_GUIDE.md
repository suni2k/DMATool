# DMA Benchmarking & Testing - Complete Guide

## ?? Overview

This guide covers comprehensive DMA performance testing and benchmarking for validating DMA card functionality after installation or firmware flashing. These tests help customers verify their DMA card is working properly before deployment.

---

## ?? Test Types

### 1. **Quick Speed Test** ?
**Purpose:** Fast validation of DMA read performance  
**Duration:** 10 seconds (default)  
**What it tests:** Reads per second (RPS) and latency

**Use when:**
- First-time card validation
- Quick health check
- Troubleshooting performance issues

**Expected Results:**
- **Elite:** 7,500+ RPS ?
- **Amazing:** 6,500-7,500 RPS ??
- **Good:** 5,200-6,500 RPS ?
- **Warning:** 4,000-5,200 RPS ??
- **Low:** <4,000 RPS ?

**Example Output:**
```
[i] Running Quick Speed Test (10 seconds)...

[01/10s]: 6,160 RPS
[02/10s]: 6,130 RPS
[03/10s]: 6,320 RPS
...
[10/10s]: 6,400 RPS

Results:
- Total Reads: 64,940
- Slowest Read: 1,173 탎
- Fastest Read: 104 탎
- AVG. Latency: 154 탎
- Reads Per Second (RPS): 6,494 ? AMAZING
```

---

### 2. **Custom Speed Test** ???
**Purpose:** Configurable speed testing with custom parameters  
**Duration:** User-defined (default: 10s)  
**Read Size:** User-defined (default: 1 byte)

**Use when:**
- Testing specific read patterns
- Comparing different configurations
- Advanced troubleshooting

**Configuration Options:**
```
Test duration: 5-300 seconds
Read size: 1-4096 bytes
Memory range: Specific or auto-enumerate
```

**Example Output:**
```
Custom Speed Test Settings:
[?] Test duration (seconds) [default: 10]: 30
[?] Read size (bytes) [default: 1]: 8

[i] Running Custom Speed Test...

Results:
- Total Reads: 19,359
- AVG. Latency: 1,550 탎
- Reads Per Second (RPS): 645
```

---

### 3. **Throughput Test** ??
**Purpose:** Measure maximum data transfer rate  
**Data Size:** 1 GB (default)  
**What it tests:** MB/s sustained transfer speed

**Use when:**
- Validating USB3 connection (FT601)
- Comparing adapter performance
- Testing large memory dumps

**Expected Results:**

| Adapter Type | Elite | Amazing | Good | Warning | Low |
|--------------|-------|---------|------|---------|-----|
| **USB3 (FT601)** | >220 MB/s | 200-220 | 150-200 | 125-150 | <125 |
| **USB2 (FT2232H)** | >30 MB/s | 25-30 | 20-25 | 15-20 | <15 |

**Example Output:**
```
[i] Running 1 GB Throughput Test...

Progress: [????????????????????] 100%
Time Elapsed: 4.5 seconds

Results:
- Throughput (MB/s): 222.2 ? ELITE
- Data Transferred: 1,000 MB
```

---

### 4. **Mixed Test (Speed + Throughput)** ??
**Purpose:** Combined validation of both RPS and throughput  
**Duration:** ~20 seconds total

**Use when:**
- Comprehensive card validation
- Pre-deployment testing
- Customer acceptance testing

**Example Output:**
```
[i] Running Mixed Test...

=== Phase 1: Speed Test (10s) ===
- RPS: 625
- Latency: 1,599 탎

=== Phase 2: Throughput Test (1 GB) ===
- Throughput: 145.3 MB/s

Overall Status: ? PASS
```

---

### 5. **Stress Test** ??
**Purpose:** Extended testing under load to detect instability  
**Duration:** 60-3600 seconds (configurable)  
**Read Size:** 4096 bytes (default)

**Use when:**
- Long-term stability testing
- Detecting intermittent issues
- Quality assurance / burn-in testing

**Configuration Options:**
```
Test duration: 60-3600 seconds
Read size: 1-8192 bytes
Error tolerance: 0-1%
```

**Example Output:**
```
Stress Test Settings:
[?] Test duration (seconds) [default: 60]: 300
[?] Read size (bytes) [default: 4096]: 4096

[i] Running 5-minute Stress Test...

[001/300s]: 656 RPS ?
[002/300s]: 644 RPS ?
[003/300s]: 672 RPS ?
...
[300/300s]: 651 RPS ?

Results:
- Total Reads: 195,600
- Failed Reads: 0
- Error Rate: 0.00% ? EXCELLENT
- Uptime: 100%
```

---

## ?? Advanced Options

### FPGA Algorithm Selection

Different algorithms optimize for different scenarios:

| Algorithm | Use Case | Performance Characteristics |
|-----------|----------|----------------------------|
| **Auto** | General use | Best for most scenarios |
| **Async Normal** | High throughput | Optimized for large transfers |
| **Async Tiny** | Low latency | Optimized for small reads |
| **Old Normal** | Compatibility | Legacy algorithm |
| **Old Tiny** | Compatibility | Legacy tiny reads |

**Configuration:**
```
Please choose the desired FPGA Algorithm:
[1] Auto (Recommended)
[2] Async Normal
[3] Async Tiny
[4] Old Normal
[5] Old Tiny

[?] FPGA Algorithm [default: 1]: 1
```

### Logging Modes

Control verbosity of test output:

| Mode | Description | When to Use |
|------|-------------|-------------|
| **None** | Minimal output | Production testing |
| **Verbose** | Detailed logging | Troubleshooting |
| **Very Verbose** | Full debug output | Advanced debugging |

---

## ?? Performance Benchmarks

### Expected Performance by Hardware

#### **XC7A100T (Your Card)**
| Metric | Elite | Amazing | Good | Warning | Low |
|--------|-------|---------|------|---------|-----|
| **RPS** | >7,500 | 6,500-7,500 | 5,200-6,500 | 4,000-5,200 | <4,000 |
| **Throughput** | >220 MB/s | 200-220 | 150-200 | 125-150 | <125 |
| **Latency** | <130 탎 | 130-150 | 150-190 | 190-250 | >250 |

#### **XC7A75T**
| Metric | Elite | Amazing | Good | Warning | Low |
|--------|-------|---------|------|---------|-----|
| **RPS** | >7,000 | 6,000-7,000 | 4,800-6,000 | 3,500-4,800 | <3,500 |
| **Throughput** | >210 MB/s | 190-210 | 140-190 | 115-140 | <115 |
| **Latency** | <140 탎 | 140-160 | 160-200 | 200-270 | >270 |

#### **XC7A35T**
| Metric | Elite | Amazing | Good | Warning | Low |
|--------|-------|---------|------|---------|-----|
| **RPS** | >6,500 | 5,500-6,500 | 4,200-5,500 | 3,000-4,200 | <3,000 |
| **Throughput** | >200 MB/s | 180-200 | 130-180 | 105-130 | <105 |
| **Latency** | <150 탎 | 150-180 | 180-230 | 230-330 | >330 |

### Performance Classifications

- **ELITE:** Top 1% performance (competition-grade)
- **AMAZING:** Top 5% performance (excellent for gaming)
- **GOOD:** Above average (suitable for most applications)
- **WARNING:** Below expected (check connections/drivers)
- **LOW:** Issue present, troubleshooting required

---

## ??? Troubleshooting Guide

### Issue: Low RPS (<4,000)

**Causes & Solutions:**

1. **USB Connection Issue**
   ```
   ? Check USB cable quality
   ? Try different USB 3.0 port
   ? Ensure blue USB port (USB 3.0)
   ? Update USB controller drivers
   ```

2. **FPGA Algorithm Mismatch**
   ```
   ? Try "Auto" algorithm first
   ? Test "Async Normal" for throughput
   ? Test "Async Tiny" for latency
   ```

3. **Memory Enumeration Issue**
   ```
   ? Delete mmap.txt and regenerate
   ? Check memory ranges are valid
   ? Verify BIOS settings (IOMMU/VT-d)
   ```

### Issue: Low Throughput (<125 MB/s on USB3)

**Causes & Solutions:**

1. **USB 2.0 Connection**
   ```
   ? Verify USB 3.0 port (blue connector)
   ? Check Device Manager: "USB 3.0" in name
   ? Test different USB 3.0 port
   ```

2. **FTDI Driver Issue**
   ```
   ? Download latest FTDI D3XX drivers
   ? URL: https://ftdichip.com/drivers/d3xx-drivers/
   ? Reinstall FTD3XX.dll
   ```

3. **PCIe Slot Issue**
   ```
   ? Try different PCIe slot (x1, x4, x16)
   ? Prefer slot closest to CPU
   ? Check PCIe Gen setting in BIOS
   ```

### Issue: Intermittent Failures

**Causes & Solutions:**

1. **Unstable Power**
   ```
   ? Check DMA card LED is solid (not flickering)
   ? Verify PCIe power connections
   ? Test with different PSU
   ```

2. **Firmware Issue**
   ```
   ? Re-flash FPGA firmware
   ? Verify bitstream integrity
   ? Check firmware version compatibility
   ```

3. **Memory Protection**
   ```
   ? Disable Virtualization in BIOS
   ? Disable VT-d (Intel) / SVM (AMD)
   ? Disable IOMMU
   ```

---

## ?? Recommended Testing Workflow

### For New Installations:

```
1. Quick Speed Test (10s)
   ?? PASS ? Continue
   ?? FAIL ? Troubleshoot connections

2. Throughput Test (1 GB)
   ?? PASS ? Continue
   ?? FAIL ? Check USB/drivers

3. Mixed Test
   ?? PASS ? Continue
   ?? FAIL ? Advanced troubleshooting

4. Stress Test (5-10 minutes)
   ?? PASS ? Ready for deployment ?
   ?? FAIL ? Extended diagnostics
```

### For Post-Firmware Flash:

```
1. Quick Speed Test
   ?? Verify RPS same as before

2. Throughput Test
   ?? Verify throughput maintained

3. Stress Test (5 minutes)
   ?? Ensure stability after flash
```

### For Customer Acceptance:

```
1. Mixed Test (Speed + Throughput)
   ?? Document results

2. Stress Test (10-30 minutes)
   ?? Prove long-term stability

3. Generate Test Report
   ?? Export results to PDF/CSV
```

---

## ?? PCILeech Command Reference

### Basic Commands

```powershell
# Probe for device
pcileech.exe probe

# Quick benchmark
pcileech.exe benchmark

# Memory display (visual test)
pcileech.exe display -min 0x1000 -max 0x2000

# Memory dump (throughput test)
pcileech.exe dump -min 0x1000 -max 0x101000 -out test.bin

# Read stability test
pcileech.exe testmemread -min 0x1000

# Read+Write stability test
pcileech.exe testmemreadwrite -min 0x1000
```

### Advanced Commands

```powershell
# Custom device selection
pcileech.exe probe -device fpga
pcileech.exe probe -device fpga://algo=0

# Verbose output
pcileech.exe benchmark -v -vv -vvv

# Custom memory range
pcileech.exe dump -min 0x100000 -max 0x200000

# Algorithm selection
pcileech.exe benchmark -device fpga://algo=1  # Async Normal
pcileech.exe benchmark -device fpga://algo=2  # Async Tiny
```

---

## ?? Memory Enumeration

### Understanding Memory Ranges

Valid memory ranges from typical test output:

```
[+] Adding memory range: 1000 - 5E000        (384 KB)
[+] Adding memory range: 5F000 - A0000       (260 KB)
[+] Adding memory range: 100000 - 30B93000   (779 MB)
[+] Adding memory range: 30B94000 - 31276000 (6.9 MB)
[+] Adding memory range: 35FFF000 - 36000000 (4 KB)
[+] Adding memory range: 100000000 - 10BFC00000 (43 GB)
```

**Safe Test Ranges:**
- **Low Memory:** `0x1000 - 0x100000` (safe, stable)
- **Mid Memory:** `0x100000 - 0x1000000` (generally safe)
- **High Memory:** `0x100000000+` (varies by system)

**Avoid:**
- `0x0 - 0x1000` (reserved/BIOS)
- MMIO regions (hardware mapped)
- Kernel memory (may change frequently)

---

## ?? Test Result Interpretation

### Reading Test Output

```
Results:
- Total Reads: 6,494          ? Total operations completed
- Slowest Read: 11,736 탎     ? Worst-case latency (outlier)
- Fastest Read: 1,049 탎      ? Best-case latency
- AVG. Latency: 1,541 탎      ? Average latency (key metric)
- Reads Per Second (RPS): 649 ? Operations per second (key metric)
```

**Key Metrics:**

1. **RPS (Reads Per Second)**
   - Primary performance indicator
   - Higher = better
   - Affected by: USB speed, FPGA algorithm, system load

2. **Average Latency**
   - Time per operation
   - Lower = better
   - Consistent values = stable

3. **Slowest Read**
   - Watch for extreme outliers (>20ms)
   - May indicate USB congestion or system interference

---

## ?? Quick Start Checklist

### Pre-Test Setup:
- [ ] DMA card installed in PCIe slot
- [ ] USB cable connected (DATA port, not UPDATE)
- [ ] FTDI drivers installed
- [ ] Device detected in Device Manager
- [ ] OpenOCD/PCILeech installed

### Running First Test:
```powershell
# 1. Navigate to PCILeech
cd C:\Tools\PCILeech

# 2. Probe device
.\pcileech.exe probe

# 3. Run quick benchmark
.\pcileech.exe benchmark

# 4. If successful, run stability test
.\pcileech.exe testmemread -min 0x1000
```

### Success Criteria:
- ? Device detected in probe
- ? Benchmark shows >400 RPS
- ? Throughput >80 MB/s (USB3)
- ? Stability test passes 100 iterations

---

## ?? Support Resources

**Documentation:**
- PCILeech Wiki: https://github.com/ufrisk/pcileech/wiki
- LeechCore Docs: https://github.com/ufrisk/LeechCore

**Community:**
- Discord: https://discord.gg/MfH9UHxkdP
- GitHub Issues: https://github.com/ufrisk/pcileech/issues

**Drivers:**
- FTDI D3XX: https://ftdichip.com/drivers/d3xx-drivers/
- CH347: https://github.com/WCHSoftGroup/ch347

---

**Last Updated:** January 2025  
**Version:** 1.0  
**For:** DMATool v1.0+
