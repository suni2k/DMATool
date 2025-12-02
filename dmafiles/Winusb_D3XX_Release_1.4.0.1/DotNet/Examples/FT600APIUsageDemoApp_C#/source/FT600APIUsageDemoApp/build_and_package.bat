@echo off
setlocal

echo ================================
echo Locating signtool...

:: Adjust path based on VS installation (example shown for VS2022)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq tokens=*" %%i IN (`%VSWHERE% -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) DO SET VSINSTALL=%%i

IF EXIST "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" (
    echo Running VS Dev Environment...
    CALL "%VSINSTALL%\Common7\Tools\VsDevCmd.bat"
) ELSE (
    echo Could not locate Visual Studio Developer Command tools!
    exit /b 1
)

echo ================================
echo Building x86 version...
dotnet publish -c Release -r win-x86 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -p:EnableCompressionInSingleFile=true
IF %ERRORLEVEL% NEQ 0 (
    echo Error building x86 version!
    exit /b %ERRORLEVEL%
)

copy /Y ".\bin\Release\net8.0-windows\win-x86\publish\FT600APIUsageDemoApp.exe" ".\FT600APIUsageDemoApp.exe"

signtool sign /debug /v /s MY /a /n "Future Technology Devices International Ltd" /fd sha256 /tr http://timestamp.digicert.com /td sha256 /as FT600APIUsageDemoApp.exe


echo All builds and signing completed successfully!

pause
