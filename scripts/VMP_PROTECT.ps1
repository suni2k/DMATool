# VMProtect - GUI Method (Easiest and Most Reliable)
# This opens VMProtect GUI with pre-filled paths

Write-Host "`n=== VMProtect Protection (GUI Method) ===" -ForegroundColor Cyan
Write-Host "This will open VMProtect GUI with your project loaded`n" -ForegroundColor Gray

$vmprotectGUI = "C:\Program Files\VMProtect Ultimate\VMProtect.exe"
$inputExe = Resolve-Path ".\bin\Release-x64\DMATool.exe"
$outputExe = "C:\Users\suni\source\repos\DMATool\bin\Release-x64\DMATool.vmp.exe"

# Verify input exists
if (-not (Test-Path $inputExe)) {
    Write-Host "? ERROR: Input exe not found" -ForegroundColor Red
    Write-Host "Please build Release configuration first!" -ForegroundColor Yellow
    exit 1
}

Write-Host "? Input file ready: $inputExe" -ForegroundColor Green
Write-Host "  Size: $([math]::Round((Get-Item $inputExe).Length/1MB,2)) MB`n" -ForegroundColor Gray

Write-Host "=== Instructions ===" -ForegroundColor Cyan
Write-Host "VMProtect GUI will open. Follow these steps:`n" -ForegroundColor Yellow

Write-Host "Step 1: Click 'File ? New'" -ForegroundColor White
Write-Host "Step 2: Input File - Click Browse and select:" -ForegroundColor White
Write-Host "        $inputExe" -ForegroundColor Gray
Write-Host "Step 3: Output File - Enter:" -ForegroundColor White  
Write-Host "        $outputExe" -ForegroundColor Gray
Write-Host "Step 4: Click the big 'Compile' button (? icon)" -ForegroundColor White
Write-Host "Step 5: Wait for 'Compilation completed' message" -ForegroundColor White
Write-Host "Step 6: Close VMProtect`n" -ForegroundColor White

Write-Host "Opening VMProtect GUI in 3 seconds..." -ForegroundColor Yellow
Start-Sleep -Seconds 3

# Open VMProtect GUI
Start-Process $vmprotectGUI

Write-Host "`n? VMProtect GUI opened" -ForegroundColor Green
Write-Host "`nWaiting for you to complete the protection..." -ForegroundColor Yellow
Write-Host "Press ENTER when VMProtect finishes (after you see 'Compilation completed')" -ForegroundColor Cyan

Read-Host

# Check if output was created
if (Test-Path $outputExe) {
    $originalSize = (Get-Item $inputExe).Length
    $protectedSize = (Get-Item $outputExe).Length
    $increase = $protectedSize - $originalSize
    $percentIncrease = [math]::Round(($increase / $originalSize) * 100, 1)
    
    Write-Host "`n=== ? SUCCESS! Protected File Created ===" -ForegroundColor Green
    Write-Host "Location: $outputExe" -ForegroundColor White
    Write-Host "Original: $([math]::Round($originalSize/1MB,2)) MB" -ForegroundColor Gray
    Write-Host "Protected: $([math]::Round($protectedSize/1MB,2)) MB" -ForegroundColor Gray
    Write-Host "Increase: +$([math]::Round($increase/1KB,0)) KB (+$percentIncrease%)`n" -ForegroundColor Gray
    
    Write-Host "=== Next Steps ===" -ForegroundColor Cyan
    Write-Host "1. Test the protected exe:" -ForegroundColor Yellow
    Write-Host "   $outputExe`n" -ForegroundColor White
    
    Write-Host "2. If all features work, distribute this file" -ForegroundColor Yellow
    Write-Host "3. Rename to DMATool.exe for users`n" -ForegroundColor Yellow
    
    Write-Host "? Your code is now protected from IDA Pro/Hex-Rays!" -ForegroundColor Green
    
} else {
    Write-Host "`n? Protected file not found" -ForegroundColor Yellow
    Write-Host "Did you complete all steps in VMProtect GUI?" -ForegroundColor Gray
    Write-Host "`nExpected location: $outputExe" -ForegroundColor Gray
}
