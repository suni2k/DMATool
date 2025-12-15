#pragma once
#include <string>

// Windows Driver Installation API wrapper
// Uses UpdateDriverForPlugAndPlayDevices like Zadig does
bool InstallDriverForDevice(const std::wstring& infPath, const std::wstring& hardwareId);
