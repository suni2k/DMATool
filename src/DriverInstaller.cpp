#include "DriverInstaller.h"
#include <windows.h>
#include <newdev.h>
#include <setupapi.h>
#include <string>

#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "setupapi.lib")

bool InstallDriverForDevice(const std::wstring& infPath, const std::wstring& hardwareId)
{
    BOOL rebootRequired = FALSE;
    
    // Use UpdateDriverForPlugAndPlayDevices exactly like Zadig does
    // INSTALLFLAG_FORCE = 0x00000001 - Force installation even if current driver is newer
    BOOL result = UpdateDriverForPlugAndPlayDevicesW(
        NULL,                       // No parent window
        hardwareId.c_str(),         // Hardware ID to match
        infPath.c_str(),            // Path to INF file
        INSTALLFLAG_FORCE,          // Force installation
        &rebootRequired             // Reboot required flag
    );

    return result != FALSE;
}
