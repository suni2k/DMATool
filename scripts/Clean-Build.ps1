# Clean-Build.ps1
# Removes all build artifacts to force a clean rebuild

Write-Host "Cleaning build artifacts..." -ForegroundColor Cyan

# Define directories to clean
$dirs = @(
    "bin-int",
    "bin",
    "x64",
    ".vs"
)

foreach ($dir in $dirs) {
    if (Test-Path $dir) {
        Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "[DELETED] $dir" -ForegroundColor Yellow
    }
}

Write-Host "[SUCCESS] Build artifacts cleaned!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Close Visual Studio completely" -ForegroundColor White
Write-Host "2. Reopen Visual Studio" -ForegroundColor White
Write-Host "3. Build -> Rebuild Solution" -ForegroundColor White
