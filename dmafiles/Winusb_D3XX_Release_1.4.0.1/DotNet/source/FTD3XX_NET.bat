@echo off
REM 1) Find the latest VS install (requires VS 2017+)
for /f "usebackq delims=" %%i in (`
  "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" ^
    -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
`) do set VSINSTALLDIR=%%i

REM 2) Call the dev-cmd to get signtool on PATH
call "%VSINSTALLDIR%\Common7\Tools\VsDevCmd.bat" -no_logo -arch=%PROCESSOR_ARCHITECTURE%

REM 3) Sign the DLL passed in as %1
signtool sign /debug /n "Future Technology Devices International Ltd" ^
  /tr http://timestamp.digicert.com /td sha256 /fd sha256 /a ^
  "%~1"
