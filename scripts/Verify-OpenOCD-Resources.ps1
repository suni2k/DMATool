# Verify OpenOCD Resources
Write-Host "=== Verifying OpenOCD Resource Files ===" -ForegroundColor Cyan
Write-Host ""

$projectRoot = "C:\Users\suni\source\repos\DMATool"
$errors = 0

# Check all resource files from DMATool.rc
$resources = @(
    @{ Name = "openocd.exe"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\openocd.exe" },
    @{ Name = "ch347.cfg"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\ch347.cfg" },
    @{ Name = "libusb-1.0.dll"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\libusb-1.0.dll" },
    @{ Name = "libhidapi-0.dll"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\libhidapi-0.dll" },
    @{ Name = "xilinx-dna-347.cfg"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-dna-347.cfg" },
    @{ Name = "xilinx-xc7.cfg"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-xc7.cfg" },
    @{ Name = "jtagspi.cfg"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\jtagspi.cfg" },
    @{ Name = "xilinx-dna.cfg"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-dna.cfg" },
    @{ Name = "bscan_spi_xc7a35t.bit"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a35t.bit" },
    @{ Name = "bscan_spi_xc7a50t.bit"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a50t.bit" },
    @{ Name = "bscan_spi_xc7a75t.bit"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a75t.bit" },
    @{ Name = "bscan_spi_xc7a100t.bit"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a100t.bit" },
    @{ Name = "bscan_spi_xc7a200t.bit"; Path = "dmafiles\CH347FPGATool\OpenOCD_CH347\share\openocd\scripts\cpld\xilinx\bscan_spi_xc7a200t.bit" }
)

foreach ($resource in $resources) {
    $fullPath = Join-Path $projectRoot $resource.Path
    
    if (Test-Path $fullPath) {
        $size = (Get-Item $fullPath).Length
        $sizeKB = [math]::Round($size / 1KB, 2)
        Write-Host "[OK] " -ForegroundColor Green -NoNewline
        Write-Host "$($resource.Name) " -NoNewline
        Write-Host "($sizeKB KB)" -ForegroundColor Gray
    }
    else {
        Write-Host "[MISSING] " -ForegroundColor Red -NoNewline
        Write-Host "$($resource.Name)"
        Write-Host "  Expected at: $fullPath" -ForegroundColor Yellow
        $errors++
    }
}

Write-Host ""
if ($errors -eq 0) {
    Write-Host "=== All resource files found! ===" -ForegroundColor Green
}
else {
    Write-Host "=== $errors file(s) missing! ===" -ForegroundColor Red
    Write-Host ""
    Write-Host "SOLUTION: Copy missing files from the working CH347FPGATool directory" -ForegroundColor Yellow
}
