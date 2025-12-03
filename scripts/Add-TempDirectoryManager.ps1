# Add TempDirectoryManager to DMATool.vcxproj

$projFile = "C:\Users\suni\source\repos\DMATool\DMATool.vcxproj"

# Read the project file
$content = Get-Content $projFile -Raw

# Check if TempDirectoryManager is already in the project
if ($content -notlike "*TempDirectoryManager.cpp*") {
    Write-Host "Adding TempDirectoryManager.cpp to project..." -ForegroundColor Yellow
    
    # Find the last ClCompile entry and add after it
    $content = $content -replace '(\s*<ClCompile Include="src\\Backend\\FlashInterface.cpp" />)', 
        '$1' + "`r`n    <ClCompile Include=`"src\Util\TempDirectoryManager.cpp`" />"
    
    Write-Host "Added TempDirectoryManager.cpp" -ForegroundColor Green
}

if ($content -notlike "*TempDirectoryManager.h*") {
    Write-Host "Adding TempDirectoryManager.h to project..." -ForegroundColor Yellow
    
    # Find the last ClInclude entry and add after it
    $content = $content -replace '(\s*<ClInclude Include="src\\Backend\\FlashInterface.h" />)', 
        '$1' + "`r`n    <ClInclude Include=`"src\Util\TempDirectoryManager.h`" />"
    
    Write-Host "Added TempDirectoryManager.h" -ForegroundColor Green
}

# Save the project file
$content | Set-Content $projFile -NoNewline

Write-Host ""
Write-Host "Project file updated successfully!" -ForegroundColor Green
Write-Host "Please rebuild the solution." -ForegroundColor Cyan
