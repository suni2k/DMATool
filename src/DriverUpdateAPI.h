#pragma once
#include <string>

namespace DriverUpdateAPI {
    // Updates the driver for a specific device using Windows UpdateDriverForPlugAndPlayDevices API
    // This is the same method Zadig uses to force a driver onto a device
    // 
    // Parameters:
    //   hardwareId - The hardware ID of the device (e.g., "USB\\VID_0403&PID_6011&MI_00")
    //   infPath    - Full path to the INF file (e.g., "C:\\path\\to\\driver.inf")
    //
    // Returns:
    //   true if driver was successfully updated, false otherwise
    bool UpdateDriverForDevice(const std::wstring& hardwareId, const std::wstring& infPath);
}
