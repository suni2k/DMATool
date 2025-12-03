# Manual OpenOCD Test Script
# Tests OpenOCD commands step-by-step to diagnose the CH347 communication error

Write-Host "=== Manual OpenOCD Testing ===" -ForegroundColor Cyan
Write-Host ""

$tempDir = "$env:TEMP\DMATool"
$openocdExe = "$tempDir\openocd.exe"

# Check if files exist
if (!(Test-Path $openocdExe)) {
    Write-Host "[ERROR] OpenOCD not found at: $openocdExe" -ForegroundColor Red
    Write-Host "[INFO] Run DMATool first to extract resources" -ForegroundColor Yellow
    exit 1
}

Write-Host "[OK] Found OpenOCD at: $openocdExe" -ForegroundColor Green
Write-Host "[OK] Temp directory: $tempDir" -ForegroundColor Green
Write-Host ""

# Set environment variable
$env:OPENOCD_SCRIPTS = $tempDir
Write-Host "[INFO] Set OPENOCD_SCRIPTS=$tempDir" -ForegroundColor Cyan
Write-Host ""

# Test 1: Just load the config without init
Write-Host "=== TEST 1: Load Config Only (No Init) ===" -ForegroundColor Yellow
Write-Host "Command: openocd.exe -c `"source [find cpld/xilinx-dna-347.cfg]`" -c `"exit`"" -ForegroundColor Gray
Write-Host ""

& $openocdExe -c "source [find cpld/xilinx-dna-347.cfg]" -c "exit"
$test1Exit = $LASTEXITCODE

Write-Host ""
Write-Host "Test 1 Exit Code: $test1Exit" -ForegroundColor $(if ($test1Exit -eq 0) { "Green" } else { "Red" })
Write-Host ""
Write-Host "Press any key for Test 2..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
Write-Host ""

# Test 2: Load config and init (where it fails)
Write-Host "=== TEST 2: Load Config + Init ===" -ForegroundColor Yellow
Write-Host "Command: openocd.exe -c `"source [find cpld/xilinx-dna-347.cfg]`" -c `"init`" -c `"shutdown`"" -ForegroundColor Gray
Write-Host ""

& $openocdExe -c "source [find cpld/xilinx-dna-347.cfg]" -c "init" -c "shutdown"
$test2Exit = $LASTEXITCODE

Write-Host ""
Write-Host "Test 2 Exit Code: $test2Exit" -ForegroundColor $(if ($test2Exit -eq 0) { "Green" } else { "Red" })
Write-Host ""

if ($test2Exit -ne 0) {
    Write-Host "[ERROR] Init failed! This suggests a hardware/driver issue:" -ForegroundColor Red
    Write-Host "  - Check JTAG cable is connected to FPGA" -ForegroundColor Yellow
    Write-Host "  - Verify FPGA is powered on" -ForegroundColor Yellow
    Write-Host "  - Check CH347 driver is correct (should be HighSpeed-JTAG)" -ForegroundColor Yellow
    Write-Host "  - Try unplugging/replugging CH347 adapter" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Press any key to continue with Test 3 (manual adapter config)..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    Write-Host ""
}

# Test 3: Manual adapter config (bypass the config file)
Write-Host "=== TEST 3: Manual Adapter Config ===" -ForegroundColor Yellow
Write-Host "Command: openocd.exe -c `"adapter driver ch347`" -c `"ch347 vid_pid 0x1a86 0x55dd`" -c `"adapter speed 1000`" -c `"init`" -c `"shutdown`"" -ForegroundColor Gray
Write-Host ""

& $openocdExe -c "adapter driver ch347" -c "ch347 vid_pid 0x1a86 0x55dd" -c "adapter speed 1000" -c "init" -c "shutdown"
$test3Exit = $LASTEXITCODE

Write-Host ""
Write-Host "Test 3 Exit Code: $test3Exit" -ForegroundColor $(if ($test3Exit -eq 0) { "Green" } else { "Red" })
Write-Host ""

# Test 4: Lower speed (sometimes helps with communication issues)
Write-Host "=== TEST 4: Lower Clock Speed (100 kHz) ===" -ForegroundColor Yellow
Write-Host "Command: openocd.exe -c `"adapter driver ch347`" -c `"ch347 vid_pid 0x1a86 0x55dd`" -c `"adapter speed 100`" -c `"init`" -c `"shutdown`"" -ForegroundColor Gray
Write-Host ""

& $openocdExe -c "adapter driver ch347" -c "ch347 vid_pid 0x1a86 0x55dd" -c "adapter speed 100" -c "init" -c "shutdown"
$test4Exit = $LASTEXITCODE

Write-Host ""
Write-Host "Test 4 Exit Code: $test4Exit" -ForegroundColor $(if ($test4Exit -eq 0) { "Green" } else { "Red" })
Write-Host ""

# Summary
Write-Host "=== Test Summary ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Test 1 (Config Load):       " -NoNewline
Write-Host $(if ($test1Exit -eq 0) { "PASS" } else { "FAIL" }) -ForegroundColor $(if ($test1Exit -eq 0) { "Green" } else { "Red" })

Write-Host "Test 2 (Config + Init):     " -NoNewline
Write-Host $(if ($test2Exit -eq 0) { "PASS" } else { "FAIL" }) -ForegroundColor $(if ($test2Exit -eq 0) { "Green" } else { "Red" })

Write-Host "Test 3 (Manual 1MHz):       " -NoNewline
Write-Host $(if ($test3Exit -eq 0) { "PASS" } else { "FAIL" }) -ForegroundColor $(if ($test3Exit -eq 0) { "Green" } else { "Red" })

Write-Host "Test 4 (Manual 100kHz):     " -NoNewline
Write-Host $(if ($test4Exit -eq 0) { "PASS" } else { "FAIL" }) -ForegroundColor $(if ($test4Exit -eq 0) { "Green" } else { "Red" })

Write-Host ""

if ($test1Exit -eq 0 -and $test2Exit -ne 0) {
    Write-Host "[DIAGNOSIS] Config loads OK but init fails = Hardware/Connection Issue" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Possible causes:" -ForegroundColor Cyan
    Write-Host "1. JTAG cable not connected or loose" -ForegroundColor White
    Write-Host "2. FPGA not powered" -ForegroundColor White
    Write-Host "3. Wrong JTAG pins (TDI/TDO/TCK/TMS/GND)" -ForegroundColor White
    Write-Host "4. CH347 adapter needs to be unplugged and replugged" -ForegroundColor White
    Write-Host "5. Another application is using the CH347" -ForegroundColor White
}
elseif ($test1Exit -ne 0) {
    Write-Host "[DIAGNOSIS] Config load failed = Missing Files or Wrong OPENOCD_SCRIPTS" -ForegroundColor Yellow
}
elseif ($test2Exit -eq 0) {
    Write-Host "[SUCCESS] OpenOCD can communicate with hardware!" -ForegroundColor Green
    Write-Host "[INFO] The issue may be with the DNA extraction commands" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "Check Device Manager to verify CH347 driver:" -ForegroundColor Cyan
Write-Host "  Expected: 'USB HighSpeed-JTAG/I2C... CH347T'" -ForegroundColor White
Write-Host ""
