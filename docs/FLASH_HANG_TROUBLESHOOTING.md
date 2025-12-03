# Flash Operation Hanging at "Programming flash..." - Troubleshooting Guide

## Symptoms

The flash operation shows:
```
[PROGRESS] Preparing to flash firmware...
[INFO] Starting flash programming...
[INFO] Firmware: C:\Users\...\firmware.bin
[INFO] Target chip: xc7a75t
[PROGRESS] Preparing to flash firmware...
[PROGRESS] Programming flash...
```

And then **hangs at 10%** with no further progress.

## Immediate Diagnosis

### Step 1: Check if OpenOCD is Actually Running

Run the diagnostic script:
```powershell
.\scripts\Check-OpenOCD-Status.ps1
```

**Expected behaviors:**

1. **OpenOCD process found, CPU time increasing**
   - ? Normal - OpenOCD is actively flashing
   - Wait longer (flash can take 2-5 minutes for large firmware)

2. **OpenOCD process found, CPU time NOT increasing**
   - ? Stuck - OpenOCD has deadlocked
   - Kill it and retry

3. **No OpenOCD process found**
   - ? Failed to launch - Check configuration

### Step 2: Check Application Console Output

With the fix applied, you should now see real-time OpenOCD output in the console like:
```
[DEBUG] OpenOCD command: "C:\Users\...\openocd.exe" -f "..."
[DEBUG] OpenOCD process launched successfully
[OPENOCD] Open On-Chip Debugger 0.11.0+dev...
[OPENOCD] Info : clock speed 10000 kHz
[OPENOCD] Info : JTAG tap: xc7.tap tap/device found...
[OPENOCD] jtagspi_program
[OPENOCD] Info : sector 0 took 123 ms
[OPENOCD] Info : sector 1 took 125 ms
...
```

**If you DON'T see this output**, the fix didn't compile correctly.

### Step 3: Check for CH347 Communication Errors

Look for errors like:
```
Error: CH347_Read read data failure.
Error: CH347 clear Buffer Error.
```

This indicates a hardware communication problem.

## Common Causes & Solutions

### 1. **OpenOCD Hangs Due to Missing Real-Time Output**

**Cause**: Old version of `FlashInterface.cpp` didn't read stdout/stderr in real-time

**Fix Applied**: Modified `ExecuteOpenOCDCommand()` to use `PeekNamedPipe` and non-blocking reads

**How to Verify**: Rebuild and check for `[OPENOCD]` prefixed messages in console

---

### 2. **CH347 Adapter Communication Failure**

**Symptoms**:
- Flash hangs at 10%
- Errors in OpenOCD output about CH347

**Solutions**:

a) **Reset the adapter**:
```powershell
.\scripts\Reset-CH347-Adapter.ps1
```

b) **Replug the USB cable**
- Disconnect CH347 from PC
- Wait 5 seconds
- Reconnect

c) **Check JTAG connection**:
- Ensure JTAG cable is firmly connected to FPGA
- Verify FPGA has power
- Check for loose connections

---

### 3. **Firmware File Path Issues**

**Symptoms**:
- Flash operation starts but immediately fails
- OpenOCD can't find firmware file

**Solutions**:

a) **Check path has no special characters**:
```
? Good: C:\Users\suni\source\repos\DMATool\dmafiles\firmware.bin
? Bad:  C:\Users\suni\My Documents\firmware.bin  (space in path)
```

b) **Verify file exists**:
```powershell
Test-Path "C:\Users\suni\source\repos\DMATool\dmafiles\CH347FPGATool\002ced811686a854_ACE_75T.bin"
```

---

### 4. **BSCAN Bitstream Not Found**

**Symptoms**:
- Error about missing BSCAN bitstream
- Flash operation fails immediately

**Solution**:

Verify BSCAN files are extracted:
```powershell
Get-ChildItem "C:\Users\suni\AppData\Local\Temp\DMATool\bscan\"
```

Should show:
```
bscan_spi_xc7a35t.bit
bscan_spi_xc7a50t.bit
bscan_spi_xc7a75t.bit
bscan_spi_xc7a100t.bit
bscan_spi_xc7a200t.bit
```

If missing, restart DMATool.

---

### 5. **OpenOCD Stuck in Previous Operation**

**Symptoms**:
- Flash hangs immediately
- Multiple OpenOCD processes running

**Solution**:

Kill all OpenOCD processes:
```powershell
.\scripts\Kill-OpenOCD-Admin.ps1
```

Then retry the flash operation.

---

## Expected Flash Timeline

For a typical **4-8 MB firmware file** on **XC7A75T**:

| Stage | Time | Progress % | What's Happening |
|-------|------|------------|------------------|
| **Preparing** | 0-2s | 0-10% | Config generation, BSCAN load |
| **Sector Erase** | 10-30s | 10-40% | Erasing flash sectors |
| **Programming** | 60-120s | 40-90% | Writing firmware data |
| **Verify** | 30-60s | 90-100% | Reading back and comparing |

**Total**: ~2-4 minutes for typical firmware

---

## Debugging Steps

### Enable Detailed Logging

1. Watch the console output for `[OPENOCD]` messages
2. Look for sector erase progress:
   ```
   [OPENOCD] Info : sector 0 took 123 ms
   [OPENOCD] Info : sector 1 took 125 ms
   ```

3. Check for errors:
   ```
   [OPENOCD-ERR] Error: CH347_Read read data failure
   ```

### Test OpenOCD Manually

Try running OpenOCD directly to isolate the issue:

```powershell
cd "C:\Users\suni\AppData\Local\Temp\DMATool"

.\openocd.exe `
  -f "cpld\xilinx-dna-347.cfg" `
  -f "cpld\xilinx-dna.cfg" `
  -c "init" `
  -c "shutdown"
```

Should show:
```
Open On-Chip Debugger 0.11.0+dev...
Info : clock speed 10000 kHz
Info : JTAG tap: xc7.tap tap/device found: 0x13632093
shutdown command invoked
```

If this fails, OpenOCD/CH347 setup is broken.

---

## If Still Hanging After Fix

### Check Build Version

Verify the fix was compiled:

```powershell
# In DMATool project folder
git status

# Should show modified:
#   modified:   src/Backend/FlashInterface.cpp
```

Rebuild:
```powershell
# In Visual Studio
Build -> Rebuild Solution
```

### Monitor Process in Real-Time

Open two terminals:

**Terminal 1**: Run DMATool and start flash operation

**Terminal 2**: Monitor OpenOCD:
```powershell
while ($true) {
    .\scripts\Check-OpenOCD-Status.ps1
    Start-Sleep -Seconds 2
}
```

Watch the **CPU time** - it should be increasing if OpenOCD is working.

---

## Emergency Recovery

If the flash operation has corrupted the FPGA firmware:

1. **Power cycle the FPGA**
   - Unplug power
   - Wait 10 seconds
   - Plug back in

2. **Reflash with known-good firmware**
   - Use the original firmware file
   - Follow the flash procedure again

3. **Verify FPGA is detected**
   ```
   JTAG Port tab -> Detect FPGA
   ```

---

## Related Documentation

- `docs/FLASH_INTERFACE_LOOP_FIX.md` - Loop prevention fix
- `docs/CH347_COMMUNICATION_ERROR_GUIDE.md` - CH347 troubleshooting
- `docs/OPENOCD_PROCESS_LEAK_FIX.md` - Process management
- `scripts/Kill-OpenOCD-Admin.ps1` - Process cleanup
- `scripts/Check-OpenOCD-Status.ps1` - Diagnostic tool

---

## Success Indicators

After the fix, you should see:

1. ? Real-time `[OPENOCD]` messages in console
2. ? Progress updates showing sector numbers
3. ? Smooth progression from 10% ? 100%
4. ? Only ONE OpenOCD process during flash
5. ? Clean process termination after completion

## Contact / Issues

If the problem persists after trying all solutions:

1. Run `.\scripts\Check-OpenOCD-Status.ps1` while stuck
2. Capture the full console output
3. Note the exact point where it hangs
4. Check Windows Event Viewer for crash logs
