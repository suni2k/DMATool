# FT601 Driver Popup Notification & Uninstall Fix

## Issues Fixed ?

### 1. **Popup Notification Added**
- ? Added floating notification overlay during driver operations (Check/Install/Uninstall)
- ? Same implementation as CH357 driver in JTAG tab
- ? Dark overlay with centered popup
- ? Animated "Please wait..." text with dots
- ? Shows specific operation status (Checking/Installing/Uninstalling)

### 2. **Driver Panel Renamed**
- ? Changed "FTDI FT601 Driver" ? "FTDI Driver"
- ? Cleaner, more concise naming

### 3. **Uninstall Fixed**
- ? Fixed OEM INF detection by parsing `pnputil /enum-drivers` output line-by-line
- ? Correctly finds `oem49.inf` (or any oemXX.inf) that corresponds to `ftd3xxwu.inf`
- ? Properly uninstalls using `pnputil /delete-driver <oem>.inf /uninstall /force`

## What Changed

### src/UI/Tabs/DataPortTab.cpp

#### 1. Panel Header Renamed
```cpp
ImGui::Text("FTDI Driver");  // Was: "FTDI FT601 Driver"
```

#### 2. Popup Notification Overlay Added
```cpp
// Floating progress notification (toast-style) for driver operations
if (s_IsCheckingFT601Driver || s_IsInstallingFT601Driver || s_IsUninstallingFT601Driver)
{
    // Get the MAIN VIEWPORT (entire window) position and size
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Center notification 420x140px
    // Dark overlay with 4 rectangles around popup
    // Blue border, animated dots
    // Shows "Checking Driver" / "Installing Driver" / "Uninstalling Driver"
}
```

**Features**:
- **Dark overlay**: `IM_COL32(0, 0, 0, 160)` - dims everything except popup
- **Centered popup**: 420x140px in center of viewport
- **Blue border**: `ImVec4(0.53f, 0.65f, 0.86f, 0.8f)` - matches theme
- **Animated dots**: "Please wait..." ? "Please wait." ? "Please wait.." ? "Please wait..."
- **Operation-specific title**: Changes based on current operation

### src/Backend/FT601DriverInterface.cpp

#### 1. Uninstall Function Rewritten
```cpp
bool FT601DriverInterface::UninstallDriver()
{
    // NEW: Line-by-line parsing of pnputil output
    std::istringstream iss(output);
    std::string line;
    std::string oemInf;
    std::string publishedName;
    
    while (std::getline(iss, line))
    {
        // Look for "Published Name:" line
        if (line.find("Published Name") != std::string::npos)
        {
            // Extract oem49.inf (or any oemXX.inf)
        }
        // Look for "Original Name:" line with ftd3xxwu.inf
        else if (line.find("Original Name") != std::string::npos)
        {
            // Check if this is ftd3xxwu.inf (case-insensitive)
            if (lowerLine.find("ftd3xxwu.inf") != std::string::npos)
            {
                oemInf = publishedName;
                foundDriver = true;
                break;
            }
        }
    }
}
```

**Why This Works**:
- Parses `pnputil /enum-drivers` output line-by-line
- Tracks "Published Name" (e.g., `oem49.inf`)
- Matches it with "Original Name" (e.g., `ftd3xxwu.inf`)
- Case-insensitive search for robustness
- Handles whitespace variations

#### 2. Added Algorithm Header
```cpp
#include <algorithm>  // For std::transform (case-insensitive search)
```

## Expected User Experience

### Before Fix

#### Uninstall:
```
[INFO] Uninstalling FT601 driver...
[INFO] Searching for FT601 driver package...
[WARNING] FT601 driver package not found in driver store
[DEBUG] Searched for 'ftd3xxwu.inf' (case-insensitive)
[INFO] The driver may not be installed, or was already removed
[ERROR] Failed to uninstall FT601 driver
```
? **Regex didn't match** ? Uninstall failed

#### No Popup:
- No visual feedback during driver operations
- User could click other UI elements while processing

---

### After Fix ?

#### Check Driver:
```
User clicks "Check Driver Status"
? Popup appears: "Checking Driver" with animated dots
? Screen dims (dark overlay)
? After 1-2 frames: check completes
? Popup disappears
? Console shows results
```

#### Install Driver:
```
User clicks "Install FTDI Driver"
? Popup appears: "Installing Driver"
? Screen dims
? UAC prompt may appear (installer)
? After installation: popup disappears
? Console shows success/failure
```

#### Uninstall Driver:
```
User clicks "Uninstall FTDI Driver"
? Popup appears: "Uninstalling Driver"
? Screen dims
? Console shows:
  [INFO] Searching for FT601 driver package...
  [INFO] Found driver package: oem49.inf
  [DEBUG] Original name: ftd3xxwu.inf
  [INFO] Uninstalling driver package: oem49.inf
  [SUCCESS] FT601 driver package deleted: oem49.inf
  [INFO] Re-enumerating FT601 devices...
? Popup disappears
? Driver successfully removed
```

## Testing Checklist

- [x] Build successful
- [ ] **Check Driver Status**:
  - [ ] Popup shows "Checking Driver"
  - [ ] Screen dims during operation
  - [ ] Console shows driver info
- [ ] **Install Driver**:
  - [ ] Popup shows "Installing Driver"
  - [ ] UAC prompt appears
  - [ ] Console shows installation progress
- [ ] **Uninstall Driver**:
  - [ ] Popup shows "Uninstalling Driver"
  - [ ] Console shows "Found driver package: oem49.inf"
  - [ ] Console shows "[SUCCESS] FT601 driver package deleted"
  - [ ] Device shows with yellow triangle after uninstall (expected)

## Technical Details

### Popup Implementation
- **Viewport-based positioning**: Uses `ImGui::GetMainViewport()` for full-screen overlay
- **4-rectangle overlay**: Top, Bottom, Left, Right rectangles exclude popup
- **Centered calculation**: `(viewportSize.x - toastWidth) * 0.5f`
- **Style matching**: Same colors/fonts as JTAG tab popup

### Uninstall Parsing
- **Line-by-line**: Avoids regex complexity
- **State machine**: Tracks "Published Name" then matches "Original Name"
- **Case-insensitive**: Uses `std::transform` to lowercase
- **Whitespace handling**: Trims leading/trailing whitespace

### Why Regex Failed
The original regex tried to match across multiple lines:
```cpp
R"(Published\s+Name\s*:\s*(oem\d+\.inf).*?Original\s+Name\s*:\s*ftd3xxwu\.inf)"
```

**Problem**: 
- `pnputil` output has variable whitespace/formatting
- `.*?` doesn't match newlines by default in `std::regex`
- Need `std::regex::multiline` flag, but easier to just parse line-by-line

## Files Modified

1. `src/UI/Tabs/DataPortTab.cpp` - Added popup notification, renamed panel
2. `src/Backend/FT601DriverInterface.cpp` - Fixed uninstall, added algorithm header

## Next Steps

1. **Test uninstall** with actual FT601 device
2. **Verify OEM INF detection** works with different OEM numbers (oem49, oem50, etc.)
3. **Test popup** on different screen resolutions
4. **Verify UAC prompt** doesn't interfere with popup

## Known Behavior

### After Uninstall:
- Device will show with **yellow triangle** (??) in Device Manager
- This is **EXPECTED** - device has no driver
- Re-installing driver will fix it

### Popup Timing:
- Popup appears immediately on button click
- Operations start after **2 frames** (prevents UI lag)
- Popup disappears when operation completes
