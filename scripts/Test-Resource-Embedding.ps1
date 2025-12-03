# Test if resources are embedded in the executable
Write-Host "=== Testing Resource Embedding in DMATool.exe ===" -ForegroundColor Cyan
Write-Host ""

$exe = "C:\Users\suni\source\repos\DMATool\x64\Debug\DMATool.exe"

if (!(Test-Path $exe)) {
    Write-Host "[ERROR] DMATool.exe not found at: $exe" -ForegroundColor Red
    Write-Host "Please build the project first" -ForegroundColor Yellow
    exit
}

$exeSize = (Get-Item $exe).Length
$exeSizeMB = [math]::Round($exeSize / 1MB, 2)

Write-Host "Executable: $exe" -ForegroundColor White
Write-Host "Size: $exeSizeMB MB" -ForegroundColor White
Write-Host ""

# Expected minimum size if resources are embedded
# openocd.exe ~10MB + dlls ~1MB + bscans ~2MB + other resources ~1MB = ~14MB minimum
$minExpectedSize = 14 * 1MB

if ($exeSize -gt $minExpectedSize) {
    Write-Host "[OK] Executable size suggests resources are embedded ($exeSizeMB MB)" -ForegroundColor Green
}
else {
    Write-Host "[WARNING] Executable seems too small ($exeSizeMB MB)" -ForegroundColor Yellow
    Write-Host "Expected at least 14 MB if all resources are embedded" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "POSSIBLE ISSUE: Resources may not be compiled into the executable" -ForegroundColor Red
    Write-Host ""
    Write-Host "SOLUTIONS:" -ForegroundColor Cyan
    Write-Host "1. Rebuild the solution (Clean + Rebuild)" -ForegroundColor White
    Write-Host "2. Check that DMATool.rc is included in the project" -ForegroundColor White
    Write-Host "3. Verify all resource file paths in DMATool.rc are correct" -ForegroundColor White
}

Write-Host ""
Write-Host "Press any key to check if DMATool.rc is in the project..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

# Check if .rc file is in project
$projFile = "C:\Users\suni\source\repos\DMATool\DMATool.vcxproj"
$projContent = Get-Content $projFile -Raw

if ($projContent -like "*DMATool.rc*") {
    Write-Host "[OK] DMATool.rc is referenced in the project file" -ForegroundColor Green
}
else {
    Write-Host "[ERROR] DMATool.rc is NOT in the project file!" -ForegroundColor Red
    Write-Host ""
    Write-Host "To fix:" -ForegroundColor Yellow
    Write-Host "1. Right-click DMATool project in Visual Studio" -ForegroundColor White
    Write-Host "2. Add > Existing Item..." -ForegroundColor White
    Write-Host "3. Select DMATool.rc" -ForegroundColor White
}
