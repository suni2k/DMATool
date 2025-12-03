# Quick FPGA Detection Test
Write-Host "=== Testing FPGA Detection ===" -ForegroundColor Cyan
Write-Host ""

$exe = "C:\Users\suni\source\repos\DMATool\bin\Debug-x64\DMATool.exe"

if (!(Test-Path $exe)) {
    Write-Host "[ERROR] DMATool.exe not found" -ForegroundColor Red
    exit
}

$exeSize = (Get-Item $exe).Length / 1MB
Write-Host "Found DMATool.exe: $([math]::Round($exeSize, 2)) MB" -ForegroundColor Green
Write-Host ""

# Check temp directory to see if resources are being extracted
$tempDir = "$env:TEMP\DMATool\"

Write-Host "Checking temp directory: $tempDir" -ForegroundColor White

if (Test-Path $tempDir) {
    Write-Host "[INFO] Temp directory exists" -ForegroundColor Green
    Write-Host ""
    Write-Host "Contents:" -ForegroundColor Cyan
    Get-ChildItem $tempDir -Recurse | ForEach-Object {
        $relativePath = $_.FullName.Replace($tempDir, "")
        if ($_.PSIsContainer) {
            Write-Host "  [DIR]  $relativePath" -ForegroundColor Yellow
        }
        else {
            $size = [math]::Round($_.Length / 1KB, 2)
            Write-Host "  [FILE] $relativePath ($size KB)" -ForegroundColor White
        }
    }
}
else {
    Write-Host "[INFO] Temp directory doesn't exist yet (will be created on first run)" -ForegroundColor Gray
}

Write-Host ""
Write-Host "To test FPGA detection:" -ForegroundColor Cyan
Write-Host "1. Make sure CH347 adapter is connected to FPGA" -ForegroundColor White
Write-Host "2. Run DMATool.exe" -ForegroundColor White
Write-Host "3. Click on 'JTAG Port' tab" -ForegroundColor White  
Write-Host "4. Click 'Detect FPGA & Read DNA'" -ForegroundColor White
Write-Host ""
Write-Host "Watch the console output for:" -ForegroundColor Cyan
Write-Host "- [DEBUG] Attempting to extract resources to: ..." -ForegroundColor White
Write-Host "- [DEBUG] Extracted openocd.exe successfully" -ForegroundColor White
Write-Host "- [INFO] OpenOCD found at: ..." -ForegroundColor White
