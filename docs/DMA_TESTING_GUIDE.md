# DMA Manual Testing Guide - XC7A100T

## ?? Quick Setup & Testing

### Step 1: Download PCILeech
1. Go to: https://github.com/ufrisk/pcileech/releases/latest
2. Download: `pcileech_files_and_binaries_<version>.zip`
3. Extract to: `C:\Tools\PCILeech\`

**OR use PowerShell:**
```powershell
# Create directory
New-Item -Path "C:\Tools\PCILeech" -ItemType Directory -Force

# Download latest release (check GitHub for actual URL)
# Example URL (update version number):
$url = "https://github.com/ufrisk/pcileech/releases/download/v4.19/pcileech_files_and_binaries_v4.19.zip"
$output = "$env:TEMP\pcileech.zip"
Invoke-WebRequest -Uri $url -OutFile $output

# Extract
Expand-Archive -Path $output -DestinationPath "C:\Tools\PCILeech" -Force
Remove-Item $output
```

---

## ?? Testing Your XC7A100T DMA

### Prerequisites
- ? DMA card installed in PCIe slot
- ? Target PC powered on
- ? USB cable connected (if using USB-based DMA)
- ? FTDI drivers installed (check Device Manager)

---

### Test 1: Probe for DMA Device
**Command:**
```powershell
cd C:\Tools\PCILeech
.\pcileech.exe probe
```

**Expected Output:**
```
DEVICE: FPGA: Artix-7 XC7A100T
CONNECTION: USB -> FTDI FT601 -> PCIe
STATUS: Connected
VERSION: <firmware version>
```

**If it fails:**
- Check USB connection
- Verify FTDI drivers (VID: 0403, PID: 6011)
- Run as Administrator
- Check Device Manager for "USB Serial Converter"

---

### Test 2: Memory Display (Read Test)
**Command:**
```powershell
# Display 256 bytes from physical address 0x1000
.\pcileech.exe display -min 0x1000 -max 0x1100
```

**Expected Output:**
```
Memory at 0x0000000000001000:
0000000000001000  4d 5a 90 00 03 00 00 00  04 00 00 00 ff ff 00 00  MZ..........ÿÿ..
0000000000001010  b8 00 00 00 00 00 00 00  40 00 00 00 00 00 00 00  ¸.......@.......
0000000000001020  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  ................
...
```

**If you see:**
- ? **Valid data (like above):** DMA read working!
- ? **All zeros:** DMA not reading, check connections
- ? **Random garbage:** Memory protection issue

---

### Test 3: Memory Read Benchmark
**Command:**
```powershell
# Run performance benchmark
.\pcileech.exe benchmark
```

**Expected Output:**
```
PCIe DMA Performance Benchmark
==============================
Device: FPGA Artix-7 XC7A100T
Interface: USB3 -> FT601

Read Performance:
  1MB  :  150.2 MB/s
  10MB :  145.8 MB/s
  100MB:  142.3 MB/s

Write Performance:
  1MB  :  120.5 MB/s
  10MB :  118.2 MB/s
  100MB:  115.7 MB/s

Average Read:  146.1 MB/s ?
Average Write: 118.1 MB/s ?
```

**Performance Targets:**
- **USB3 (FT601):** 140-150 MB/s read, 110-120 MB/s write
- **USB2 (FT2232H):** 20-30 MB/s read/write

---

### Test 4: Memory Read Stability Test
**Command:**
```powershell
# Test read stability (100 iterations)
.\pcileech.exe testmemread -min 0x1000
```

**Expected Output:**
```
Memory Test Read: starting, reading 100 times from address: 0x00001000
Memory Test Read: SUCCESS!
```

**If it fails:**
```
Memory Test Read: Failed. DMA failed / data changed by target computer / memory corruption.
Read: 1. Run: 45. Offset: 0x123
```
**Causes:**
- Target memory is volatile (stack/heap)
- Memory protection active
- DMA card not stable
- **Solution:** Try different address (e.g., 0x10000, 0x100000)

---

### Test 5: Memory Read+Write Test
**Command:**
```powershell
# Test both read and write (100 iterations each)
.\pcileech.exe testmemreadwrite -min 0x1000
```

**Expected Output:**
```
Memory Test Read: starting, reading 100 times from address: 0x00001000
Memory Test Read: SUCCESS!
Memory Test Write: starting, reading/writing 100 times from address: 0x00001000
Memory Test Write: Success!
```

**?? WARNING:**
- This **modifies target memory!**
- Use safe addresses (avoid system areas)
- Original data is restored after test

---

### Test 6: Dump Memory to File
**Command:**
```powershell
# Dump 1MB from physical memory to file
.\pcileech.exe dump -min 0x1000 -max 0x101000 -out memdump.bin
```

**Expected Output:**
```
Memory Dump
  Address Range: 0x0000000000001000 - 0x0000000000101000
  Size: 1048576 bytes (1.00 MB)
  Speed: 145.3 MB/s
  File: memdump.bin

Dump completed successfully!
```

**Verify dump:**
```powershell
# Check file size
(Get-Item memdump.bin).Length  # Should be 1048576
```

---

## ?? Troubleshooting

### Issue: "Failed to connect to device"
**Solutions:**
1. Check USB connection
2. Install FTDI D2XX drivers: https://ftdichip.com/drivers/d2xx-drivers/
3. Run as Administrator
4. Try different USB port (USB 3.0 preferred)

### Issue: "Error: No FPGA device found"
**Solutions:**
1. Check DMA card is powered (LED should be on)
2. Verify PCIe connection
3. Check BIOS settings (IOMMU/VT-d should be disabled)
4. Re-flash FPGA firmware if needed

### Issue: "Read speed very slow (<10 MB/s)"
**Solutions:**
1. Use USB 3.0 port (not USB 2.0)
2. Close other USB devices
3. Update USB controller drivers
4. Check USB cable quality

### Issue: "Memory test fails immediately"
**Solutions:**
1. Try different memory address:
   ```powershell
   .\pcileech.exe testmemread -min 0x10000
   .\pcileech.exe testmemread -min 0x100000
   .\pcileech.exe testmemread -min 0x1000000
   ```
2. Use lower iteration count:
   ```powershell
   # Edit pcileech source to reduce from 100 to 10 iterations
   ```
3. Check target PC memory protection settings

---

## ?? Expected Results Summary

| Test | Expected Result | Pass Criteria |
|------|----------------|---------------|
| **Probe** | Device detected | FPGA: XC7A100T shown |
| **Display** | Valid memory data | Readable hex output |
| **Benchmark** | Performance metrics | Read: >140 MB/s, Write: >110 MB/s |
| **TestMemRead** | 100 reads successful | "SUCCESS!" message |
| **TestMemReadWrite** | 100 r/w successful | Both tests pass |
| **Dump** | 1MB file created | File size matches range |

---

## ?? Next Steps After Testing

Once all tests pass:

1. **Document your results:**
   - Take screenshots of successful tests
   - Note your read/write speeds
   - Record any issues encountered

2. **Share on Discord:**
   - Post results to: https://discord.gg/MfH9UHxkdP
   - Help others troubleshoot

3. **Integrate into DMATool:**
   - We'll build the Benchmark DMA tab
   - Automate these tests in the GUI
   - Add real-time performance monitoring

---

## ?? Support

**Discord:** https://discord.gg/MfH9UHxkdP  
**GitHub:** https://github.com/ufrisk/pcileech/issues  
**Docs:** https://github.com/ufrisk/pcileech/wiki

---

**Happy Testing!** ??
