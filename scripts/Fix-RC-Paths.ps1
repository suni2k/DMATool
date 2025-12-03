# Fix-RC-Paths.ps1
# Corrects the OpenOCD paths in DMATool.rc

$rcFile = "DMATool.rc"
$content = Get-Content $rcFile -Raw

Write-Host "`nFixing DMATool.rc paths..." -ForegroundColor Yellow

# Fix: dmafiles\\ch347\\CH347FPGATool  ?  dmafiles\\CH347FPGATool
$content = $content -replace 'dmafiles\\\\ch347\\\\CH347FPGATool', 'dmafiles\\CH347FPGATool'

$content | Set-Content $rcFile -NoNewline

Write-Host "[SUCCESS] Fixed paths in DMATool.rc" -ForegroundColor Green
Write-Host "`nCorrected:" -ForegroundColor Cyan
Write-Host "  dmafiles\\ch347\\CH347FPGATool  ?  dmafiles\\CH347FPGATool" -ForegroundColor Gray
Write-Host "`nNow rebuild to embed OpenOCD 0.11!" -ForegroundColor Yellow
