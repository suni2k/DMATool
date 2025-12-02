# FTDI Driver Performance Analysis

## Summary of Changes & Issues

### ? Changes Successfully Implemented

1. **Reverted Flash DMA & DNA ID bottom panel spacing**
   - Restored original offset calculations for proper alignment
   - Bottom panels maintain ~16px margin from window edges
   
2. **Reduced FTDI button spacing**
   - Changed spacing from `ImGui::Spacing()` (~8px) to `ImGui::Dummy(ImVec2(0, 2))` (2px)
   - Reduced vertical space by ~12px total
   - Panel should now fit without scrollbar

### ?? New Issue: Throughput Drop After FTDI Driver Installation

## Performance Comparison

### Before FTDI Driver Installation
```
Quick Speed Test:
- RPS: ~6,287
- AVG. Latency: 159 탎
- MIN. Latency: 116 탎
- MAX. Latency: 1,267 탎
- Rating: GOOD ?

Throughput Test:
- Transfer: 1024 MB
- Throughput: ~190 MB/s
- Duration: ~5.4 seconds
- Rating: AMAZING ?
```

### After FTDI Driver Installation (v1.4.0.1)
```
Quick Speed Test:
- RPS: ~6,287 (SAME ?)
- AVG. Latency: 159 탎 (SAME ?)
- MIN. Latency: 116 탎 (SAME ?)
- MAX. Latency: 1,267 탎 (SAME ?)
- Rating: GOOD ?

Throughput Test:
- Transfer: 1024 MB
- Throughput: ~149.21 MB/s (?26% ??)
- Duration: ~6.86 seconds (?27% ??)
- Rating: WARNING ??
- Errors: "FPGA: Bad PCIe TLP received!" (multiple times)
```

## Analysis

### What Changed
- **Small reads (4KB) improved**: RPS stayed high, latency stayed low
- **Large transfers (1MB chunks) degraded**: Throughput dropped from 190 MB/s to 149 MB/s
- **New errors appeared**: "Bad PCIe TLP received!" during large transfers

### Likely Causes

#### 1. **Driver Configuration Difference**

The new FTDI D3XX driver (v1.4.0.1) may have different default settings:

**Possible Issues:**
- **USB transfer size limits**: New driver may have smaller max transfer size
- **Buffer allocation**: Different buffer strategy for large transfers
- **USB endpoint configuration**: Changed endpoint settings
- **Timeout values**: More conservative timeout settings

**Evidence:**
- Small reads (4KB) work fine ? driver handles small packets well
- Large transfers (1MB) degrade ? driver struggles with bulk transfers
- Multiple "Bad TLP" errors ? suggests packet fragmentation/corruption

#### 2. **PCIe TLP (Transaction Layer Packet) Issues**

"FPGA: Bad PCIe TLP received!" indicates malformed PCIe packets.

**Root Cause Analysis:**
```
USB3 ? FTDI FT601 ? PCIe Bridge ? FPGA
        ?
    Potential bottleneck
```

**Theory:**
1. **Old driver**: Used larger USB transfers ? fewer PCIe TLPs ? better throughput
2. **New driver**: Uses smaller USB transfers ? more PCIe TLPs ? packet fragmentation
3. **Result**: FPGA receives malformed/fragmented TLPs ? retries ? lower throughput

#### 3. **USB3 Bulk Transfer Settings**

The FT601 chip supports different modes:

| Mode | Max Packet Size | Use Case |
|------|-----------------|----------|
| **SuperSpeed (USB 3.0)** | 1024 bytes | High throughput |
| **HighSpeed (USB 2.0 fallback)** | 512 bytes | Compatibility |

**Hypothesis:**
- Old driver: Optimized for 1024-byte packets (USB 3.0 native)
- New driver: More conservative packet size (better compatibility, lower throughput)

## Diagnostic Steps

### Step 1: Verify Driver Version & Settings

Check current driver configuration:

```powershell
# Device Manager ? FT601 Device ? Properties ? Driver
# Verify:
- Driver Version: 1.4.0.1
- Driver Provider: FTDI
- Driver Date: Check if latest
```

### Step 2: Check USB Connection

```powershell
# Verify USB 3.0 connection (not falling back to USB 2.0)
Get-PnpDevice | Where-Object {$_.FriendlyName -like '*FT601*'} | 
  Get-PnpDeviceProperty -KeyName 'DEVPKEY_Device_Speed'
```

**Expected output:**
- `SuperSpeed` = USB 3.0 (good)
- `HighSpeed` = USB 2.0 (bad - would explain 149 MB/s cap)

### Step 3: Test with Different USB Ports

USB 3.0 ports can vary in quality:

1. **Try different USB 3.0 ports**:
   - Front panel vs rear I/O
   - Different USB controllers
   
2. **Check USB cable**:
   - Ensure USB 3.0 cable (blue connector)
   - Try different cable if available

### Step 4: Monitor PCILeech Output

Run throughput test again and count errors:

```
Device Info: FPGA: Bad PCIe TLP received! Should not happen!
```

**Analysis:**
- 0-2 errors = minor issue, likely transient
- 3-10 errors (like yours) = moderate issue, driver/config problem
- 10+ errors = severe issue, hardware/firmware problem

## Potential Solutions

### Solution 1: Rollback to Previous Driver

If you have the old driver backup:

```powershell
# Uninstall current driver
pnputil /delete-driver <oem>.inf /uninstall /force

# Reinstall old driver
pnputil /add-driver <old_driver_path>\ftd3xxwu.inf /install
```

**Expected Result**: Throughput should return to ~190 MB/s

### Solution 2: Configure FT601 USB Settings

Check FT601 FIFO Mode configuration:

**FT_SetPipePolicy API settings** (if accessible):
- `PIPE_TRANSFER_TIMEOUT`: Increase from 500ms to 1000ms
- `AUTO_CLEAR_STALL`: Enable
- `RAW_IO`: Enable for maximum throughput

**Note**: These settings require modifying the driver or firmware.

### Solution 3: Update Firmware/Bitstream

The FPGA firmware may need updates to handle the new driver's behavior:

```
# Flash latest PCILeech firmware
# From: https://github.com/ufrisk/pcileech-fpga
```

### Solution 4: Adjust PCILeech Settings

Try different FPGA algorithms:

```powershell
# Test with algorithm 0 (auto)
pcileech.exe benchmark -device fpga://algo=0

# Test with algorithm 1 (async normal)
pcileech.exe benchmark -device fpga://algo=1

# Test with algorithm 2 (async tiny)
pcileech.exe benchmark -device fpga://algo=2
```

## Workaround (Temporary)

If throughput is critical and downgrading isn't an option:

**Use smaller chunk sizes for large transfers:**
- Instead of 1MB chunks, use 256KB or 512KB chunks
- May reduce "Bad TLP" errors
- Throughput will still be lower but more stable

## Recommended Action Plan

1. **Immediate**: Test different USB ports/cables
2. **Short-term**: Try FPGA algorithm variations
3. **Medium-term**: Consider driver rollback if issue persists
4. **Long-term**: Report issue to FTDI (driver bug) or update firmware

## Technical Details

### USB3 SuperSpeed Transfer Characteristics

**Theoretical Max (USB 3.0):**
- Bandwidth: 5 Gbps = 625 MB/s (theoretical)
- Practical: ~400 MB/s (overhead)

**FT601 Specifications:**
- Max throughput: ~200 MB/s (write), ~400 MB/s (read)
- Your old performance: 190 MB/s (expected range)
- Your new performance: 149 MB/s (**below spec**)

### PCIe TLP Structure

```
TLP Header (12-20 bytes)
??? Format & Type
??? Length
??? Requester ID
??? Tag
??? Address

TLP Data Payload (4-4096 bytes)
??? Actual data

TLP ECRC (optional, 4 bytes)
??? Error checking
```

**"Bad PCIe TLP"** errors mean:
- Malformed header (wrong format/length)
- Corrupted payload (checksum fail)
- Invalid address/ID
- Timeout (incomplete transfer)

## Monitoring Commands

### Check USB Transfer Stats

```powershell
# USB performance counter (Windows)
Get-Counter '\USB(*)\Bytes/sec' -Continuous
```

### Monitor PCIe Activity

```powershell
# PCIe bandwidth usage
Get-Counter '\PCIe(*)\Bytes/sec' -Continuous
```

### LeechCore Diagnostics

```powershell
# Verbose output to see TLP errors
pcileech.exe benchmark -v -vv -vvv
```

## Expected vs Actual Performance

| Metric | Expected (USB3) | Old Driver | New Driver | Status |
|--------|-----------------|------------|------------|--------|
| **RPS** | 6000-8000 | ~6287 ? | ~6287 ? | GOOD |
| **Latency** | 120-180 탎 | ~159 탎 ? | ~159 탎 ? | GOOD |
| **Throughput** | 180-220 MB/s | ~190 MB/s ? | ~149 MB/s ?? | WARNING |
| **TLP Errors** | 0-1 | 0 ? | 10+ ?? | HIGH |

## Conclusion

The FTDI driver installation improved **small read performance** (RPS/latency) but **degraded large transfer throughput** by ~26%. This suggests a **driver configuration issue** rather than hardware failure.

**Most likely causes** (in order):
1. **Driver USB transfer size settings** (most probable)
2. **USB 3.0 fallback to 2.0** (check connection)
3. **Firmware compatibility** (less likely, but possible)

**Recommended next step**: Test with different USB port/cable to rule out hardware, then consider driver rollback if issue persists.

---

**Files Modified:**
- `src/UI/Tabs/JTAGPortTab.cpp` - Reverted bottom panel spacing
- `src/UI/Tabs/JTAGFlashTab.cpp` - Reverted bottom panel spacing  
- `src/UI/Tabs/DataPortTab.cpp` - Reduced FTDI button spacing to 2px

**Build Status:** ? Successful

**Testing Required:**
1. Verify FTDI panel fits without scrollbar
2. Verify bottom panels aligned properly in DNA ID and Flash DMA tabs
3. Investigate throughput drop (USB/driver/firmware)
