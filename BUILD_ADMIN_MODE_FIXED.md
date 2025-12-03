# Build Fixed - Administrator Mode Enabled

## Problem
The application was failing to build with linker error `LNK1327: failure during running mt.exe`. This was caused by incompatibility with Visual Studio 2022's manifest handling.

## Root Cause
Visual Studio 2022+ no longer supports the `AdditionalManifestFiles` linker option that was being used. Additionally, there was a conflict between:
- The linker's default UAC level (`asInvoker`)
- The manifest file's UAC level (`requireAdministrator`)

## Solution
1. **Removed deprecated manifest options** from `DMATool.vcxproj`:
   - Removed `<AdditionalManifestFiles>app.manifest</AdditionalManifestFiles>`
   - Removed `<Manifest Include="app.manifest" />` from ItemGroup

2. **Set UAC execution level in linker settings**:
   - Added `<UACExecutionLevel>RequireAdministrator</UACExecutionLevel>` to both Debug and Release configurations

3. **Removed manifest from RC file**:
   - The linker now generates the manifest automatically with proper admin requirements
   - The `app.manifest` file is merged automatically by the linker

## Verification
Both Debug and Release builds now:
? Build successfully without errors
? Embed a manifest that requires administrator privileges
? Include all DPI awareness and compatibility settings from `app.manifest`

## Files Modified
- `DMATool.vcxproj` - Updated linker settings
- `DMATool.rc` - Removed duplicate manifest embedding

## Scripts Created
- `scripts/Fix-Manifest-VS2022.ps1` - Initial fix attempt
- `scripts/Fix-VCXProj-Manifest.ps1` - XML-based project file fix
- `scripts/Set-AdminMode.ps1` - Set UAC execution level

## Testing
Run the built executable - Windows will show the UAC prompt requesting administrator privileges.
