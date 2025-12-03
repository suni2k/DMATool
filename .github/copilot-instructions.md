# GitHub Copilot Instructions for DMATool

## General Guidelines
- This is a C++ Windows desktop application using ImGui and DirectX 11
- Target platform: Windows 10/11, x64 only
- Language standard: C++17
- Build system: Visual Studio MSBuild (.vcxproj)

## PowerShell Commands
When you need to run PowerShell commands:

- **Prefer creating .ps1 script files** instead of executing long scripts directly in terminal
- **Keep terminal commands SHORT** - max 3-4 lines, otherwise create a .ps1 file
- Avoid leaving incomplete constructs (like open try/finally blocks or here-strings) that wait for input
- Use short, idempotent commands in the terminal
- For multi-line scripts or anything over 100 characters, generate a .ps1 file in the `scripts/` folder
- If a command might hang or wait for input, explicitly warn and do not auto-execute
- PowerShell multi-line commands often don't output everything when run through terminal integration
- **Remember**: The terminal has limitations - use scripts for complex operations!

## Project Structure
- `src/` - Source code (C++ headers and implementation files)
- `src/Backend/` - Backend logic (OpenOCD, Flash, Drivers, LeechCore)
- `src/UI/` - UI components (ImGui-based tabs and panels)
- `src/Util/` - Utility classes
- `vendor/` - Third-party libraries (ImGui, LeechCore)
- `dmafiles/` - External resources (OpenOCD binaries, driver files, bitstreams)
- `tools/` - Driver files and utilities
- `scripts/` - PowerShell scripts for automation and maintenance
- `docs/` - Documentation and guides

## Resource Files
- `DMATool.rc` - Windows resource file (embedded files, dialogs, icons)
- `src/resource.h` - Resource ID definitions
- Resources are embedded in the .exe and extracted to temp at runtime

## Code Style
- Use consistent formatting (follow existing code style)
- Comment complex algorithms but don't over-comment obvious code
- Use RAII for resource management
- Prefer smart pointers for dynamic allocations
- Use `std::filesystem` for file operations
- Console logging: Use `std::cout` with prefixes like `[INFO]`, `[DEBUG]`, `[ERROR]`, `[SUCCESS]`

## Build Configuration
- Debug builds: Use for development, includes full debugging symbols
- Release builds: Use for distribution, optimized, smaller binary
- Administrator privileges required: Use `app.manifest` to configure UAC

## Common Tasks
- **Adding new embedded resources**: Update `DMATool.rc` and `src/resource.h`
- **Modifying UI**: Edit files in `src/UI/Tabs/`
- **Backend changes**: Edit files in `src/Backend/`
- **Driver management**: Files in `src/Backend/*DriverInterface.cpp`

## Testing
- Always test on both Debug and Release configurations
- Test with and without administrator privileges where applicable
- Verify resource extraction works correctly
- Check for memory leaks in Debug builds

## Documentation
- Update relevant `.md` files in `docs/` when making significant changes
- Keep `README.md` current with setup instructions
- Document breaking changes in commit messages
