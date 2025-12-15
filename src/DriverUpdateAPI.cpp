#include "DriverUpdateAPI.h"
#include <windows.h>
#include <setupapi.h>
#include <newdev.h>
#include <string>
#include <iostream>

#pragma comment(lib, "newdev.lib")
#pragma comment(lib, "setupapi.lib")

namespace DriverUpdateAPI {

    bool UpdateDriverForDevice(const std::wstring& hardwareId, const std::wstring& infPath) {
        BOOL rebootRequired = FALSE;
        
        std::wcout << L"[DEBUG] Calling UpdateDriverForPlugAndPlayDevicesW" << std::endl;
        std::wcout << L"[DEBUG] Hardware ID: " << hardwareId << std::endl;
        std::wcout << L"[DEBUG] INF Path: " << infPath << std::endl;

        // Call the Windows API that Zadig uses
        BOOL result = UpdateDriverForPlugAndPlayDevicesW(
            NULL,                    // No parent window
            hardwareId.c_str(),      // Hardware ID (e.g., USB\VID_0403&PID_6011&MI_00)
            infPath.c_str(),         // Full path to INF file
            INSTALLFLAG_FORCE,       // Force installation even if current driver is newer
            &rebootRequired          // Whether reboot is needed
        );

        if (result) {
            std::wcout << L"[SUCCESS] Driver updated successfully" << std::endl;
            if (rebootRequired) {
                std::wcout << L"[INFO] Reboot required for changes to take effect" << std::endl;
            }
            return true;
        } else {
            DWORD error = GetLastError();
            std::wcerr << L"[ERROR] UpdateDriverForPlugAndPlayDevicesW failed with error: " << error << std::endl;
            
            // Common error codes
            switch (error) {
                case ERROR_FILE_NOT_FOUND:
                    std::wcerr << L"[ERROR] INF file not found" << std::endl;
                    break;
                case ERROR_NO_SUCH_DEVINST:
                    std::wcerr << L"[ERROR] Device not found" << std::endl;
                    break;
                case ERROR_NO_MORE_ITEMS:
                    std::wcerr << L"[ERROR] No compatible driver found in INF" << std::endl;
                    break;
                default:
                    std::wcerr << L"[ERROR] Unknown error" << std::endl;
                    break;
            }
            return false;
        }
    }

}
