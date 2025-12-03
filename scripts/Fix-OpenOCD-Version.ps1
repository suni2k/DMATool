# Fix-OpenOCD-Version.ps1
# Updates DMATool.rc to use OpenOCD 0.11 from CH347FPGATool instead of 0.12

$rcFile = "DMATool.rc"

Write-Host "`n================================" -ForegroundColor Cyan
Write-Host " Fixing OpenOCD Version in RC" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan

# Read current content
$content = Get-Content $rcFile -Raw

Write-Host "[1/2] Current OpenOCD paths:" -ForegroundColor Yellow
$content -split "`n" | Where-Object { $_ -match "IDR_OPENOCD" -or $_ -match "IDR_XILINX" -or $_ -match "IDR_JTAGSPI" } | ForEach-Object {
    Write-Host "  $_" -ForegroundColor Gray
}

Write-Host "`n[2/2] Updating to OpenOCD 0.11 (CH347FPGATool)..." -ForegroundColor Yellow

# Replace old paths with new ones
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\openocd\.exe', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\openocd.exe'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\ch347\.cfg', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\ch347.cfg'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\libusb-1\.0\.dll', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\libusb-1.0.dll'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\libhidapi-0\.dll', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\libhidapi-0.dll'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\xilinx-dna-347\.cfg', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-dna-347.cfg'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\xilinx-xc7\.cfg', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-xc7.cfg'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\jtagspi\.cfg', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\jtagspi.cfg'
$content = $content -replace 'dmafiles\\ch347\\CH347FPGATool\\OpenOCD_CH347\\bin\\xilinx-dna\.cfg', 'dmafiles\CH347FPGATool\OpenOCD_CH347\bin\xilinx-dna.cfg'

# Write back
$content | Set-Content $rcFile -NoNewline

Write-Host "[SUCCESS] Updated DMATool.rc to use OpenOCD 0.11" -ForegroundColor Green
Write-Host ""
Write-Host "New paths:" -ForegroundColor Cyan
Write-Host "  OpenOCD: dmafiles\CH347FPGATool\OpenOCD_CH347\bin\openocd.exe" -ForegroundColor Gray
Write-Host "  Config files: dmafiles\CH347FPGATool\OpenOCD_CH347\bin\*.cfg" -ForegroundColor Gray
Write-Host ""

Write-Host "================================" -ForegroundColor Cyan
Write-Host " Next: Rebuild DMATool" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan
