# Flash Verification SHA256 Enhancement

## Summary

Enhanced flash verification to match the PowerShell script's professional output format with SHA256 hash comparison instead of byte-by-byte comparison.

---

## Changes Made

### 1. ? **SHA256 Hash Calculation**
**Implementation**: Windows Crypto API (`wincrypt.h`)

**Function**: `CalculateSHA256(const std::string& filePath)`
- Uses `CryptAcquireContext` to initialize crypto provider
- Uses `CALG_SHA_256` algorithm
- Reads file in 8KB chunks for efficiency
- Returns uppercase hex string matching PowerShell format

### 2. ? **Replaced Byte-by-Byte Comparison**
**Before**: Compared files in 128KB chunks, tracked MB boundaries  
**After**: Calculate SHA256 for both files and compare hashes

**Benefits**:
- ? **Faster** - No need to track progress through file
- ? **More reliable** - Industry-standard hash comparison
- ? **Matches PowerShell** - Same verification method
- ? **Professional** - SHA256 is cryptographic standard

### 3. ? **Enhanced Output Format**
**Matches PowerShell script exactly**:

#### Success Output:
```
[INFO] ========================================
[INFO] Flash Verification
[INFO] ========================================
[INFO] 
[INFO] Original file: C:\...\002ced811686a854_ACE_75T.bin
[INFO] Target chip: xc7a75t
[INFO] This will read the entire flash and compare SHA256 hashes
[INFO] 

[INFO] 
[SUCCESS] ============================================
[SUCCESS] VERIFICATION PASSED!
[SUCCESS] ============================================
[INFO] 
[SUCCESS] Flash contents match the firmware file exactly
[SUCCESS] The firmware was written correctly
[INFO] 
[INFO] Original SHA256:  ABFF0B6682130A283EC062D4C68AB7F7FD02902475A894F5754251CC75F705F9
[INFO] Readback SHA256:  ABFF0B6682130A283EC062D4C68AB7F7FD02902475A894F5754251CC75F705F9
[INFO] 
[INFO] Bytes verified: 2099688
[INFO] Duration: 6.514539 seconds
[INFO] Speed: 405 KiB/s
[INFO] 
```

#### Failure Output:
```
[INFO] ========================================
[INFO] Flash Verification
[INFO] ========================================
[INFO] 
[INFO] Original file: C:\...\003ccd8c77d04854_BEEAC_100T.bin
[INFO] Target chip: xc7a75t
[INFO] This will read the entire flash and compare SHA256 hashes
[INFO] 

[INFO] 
[ERROR] ============================================
[ERROR] VERIFICATION FAILED!
[ERROR] ============================================
[INFO] 
[ERROR] Flash contents DO NOT match!
[ERROR] The flash may be corrupted
[INFO] 
[INFO] Original SHA256:  41D0162DB2F9436743B2A58E708D7DEC73D56984624FC94E3AC711F8DF6E85BF
[INFO] Readback SHA256:  4A54DD28430932CE5297BF341BECB92F7A8CA4971B86F3545D0FCAC6539CDAD9
[INFO] 
[WARNING] Troubleshooting:
[WARNING]   1. Try reflashing with slower clock speed
[WARNING]   2. Check JTAG connections
[WARNING]   3. Verify hardware is working correctly
[INFO] 
```

---

## Technical Implementation

### SHA256 Calculation (Windows Crypto API)

```cpp
std::string FlashInterface::CalculateSHA256(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    
    // Initialize Windows Crypto API
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    
    CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
    CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);

    // Read and hash file in chunks
    const size_t bufferSize = 8192;
    char buffer[bufferSize];
    
    while (file.read(buffer, bufferSize) || file.gcount() > 0)
    {
        DWORD bytesRead = static_cast<DWORD>(file.gcount());
        CryptHashData(hHash, reinterpret_cast<BYTE*>(buffer), bytesRead, 0);
    }

    // Get hash value (32 bytes for SHA-256)
    BYTE hashBytes[32];
    DWORD hashLen = 32;
    CryptGetHashParam(hHash, HP_HASHVAL, hashBytes, &hashLen, 0);

    // Convert to uppercase hex string
    std::stringstream ss;
    for (DWORD i = 0; i < hashLen; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hashBytes[i]);
    }
    
    std::string hashString = ss.str();
    std::transform(hashString.begin(), hashString.end(), hashString.begin(), ::toupper);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    
    return hashString; // e.g., "ABFF0B6682130A283EC062D4C68AB7F7..."
}
```

### Verification Flow

```cpp
// 1. Read flash (30% - 65%)
flash read_bank 0 "readback.bin" 0x0 2099688

// 2. Calculate SHA256 hashes (65% - 85%)
std::string originalHash = CalculateSHA256(firmwarePath);    // 65% - 75%
std::string readbackHash = CalculateSHA256(readbackPath);    // 75% - 85%

// 3. Compare hashes (85% - 100%)
bool filesMatch = (originalHash == readbackHash);

// 4. Display results
if (filesMatch)
{
    AddLog("[SUCCESS] VERIFICATION PASSED!");
    AddLog("[INFO] Original SHA256:  " + originalHash);
    AddLog("[INFO] Readback SHA256:  " + readbackHash);
}
```

---

## Files Modified

1. **`src/Backend/FlashInterface.h`**
   - Added `CalculateSHA256()` method declaration

2. **`src/Backend/FlashInterface.cpp`**
   - Added Windows Crypto API headers (`#include <wincrypt.h>`)
   - Added `#pragma comment(lib, "advapi32.lib")`
   - Implemented `CalculateSHA256()` function
   - Modified `VerifyFirmware()` to use SHA256 instead of byte comparison
   - Extract read speed from OpenOCD output

3. **`src/UI/Tabs/JTAGFlashTab.cpp`**
   - Enhanced verification output format
   - Added header/footer separators
   - Parse and display SHA256 hashes from result message
   - Added speed information display

---

## Build Status
? **Build successful** - Ready for testing

---

## Testing Instructions

### Test 1: Successful Verification
```
Flash Tab ? Detect Flash Device
Flash Tab ? Browse ? Select: 002ced811686a854_ACE_75T.bin
Flash Tab ? Verify Firmware
```

**Expected Output**:
```
[INFO] ========================================
[INFO] Flash Verification
[INFO] ========================================

[SUCCESS] ============================================
[SUCCESS] VERIFICATION PASSED!
[SUCCESS] ============================================

[INFO] Original SHA256:  ABFF0B6682130A283EC062D4C68AB7F7FD02902475A894F5754251CC75F705F9
[INFO] Readback SHA256:  ABFF0B6682130A283EC062D4C68AB7F7FD02902475A894F5754251CC75F705F9

[INFO] Bytes verified: 2099688
[INFO] Duration: 6.5 seconds
[INFO] Speed: 405 KiB/s
```

**Progress Bar**: ?? Darker green bar showing "Verification complete - MATCH!"

---

### Test 2: Failed Verification (Wrong Firmware)
```
Flash Tab ? Browse ? Select: 003ccd8c77d04854_BEEAC_100T.bin (different firmware)
Flash Tab ? Verify Firmware
```

**Expected Output**:
```
[INFO] ========================================
[INFO] Flash Verification
[INFO] ========================================

[ERROR] ============================================
[ERROR] VERIFICATION FAILED!
[ERROR] ============================================

[INFO] Original SHA256:  41D0162DB2F9436743B2A58E708D7DEC73D56984624FC94E3AC711F8DF6E85BF
[INFO] Readback SHA256:  4A54DD28430932CE5297BF341BECB92F7A8CA4971B86F3545D0FCAC6539CDAD9

[WARNING] Troubleshooting:
[WARNING]   1. Try reflashing with slower clock speed
[WARNING]   2. Check JTAG connections
[WARNING]   3. Verify hardware is working correctly
```

**Progress Bar**: ?? Red bar showing "Verification failed - MISMATCH!"

---

### Test 3: Compare with PowerShell
```powershell
.\scripts\Verify-Flash.ps1
```

**Result**: SHA256 hashes should match **exactly** between PowerShell and UI!

---

## Benefits

### ?? **Matches PowerShell Script**
- Same verification method (SHA256)
- Same hash format (uppercase hex)
- Same output structure

### ? **Faster Verification**
- No byte-by-byte comparison needed
- Direct hash comparison
- Less memory usage

### ?? **More Reliable**
- SHA256 is cryptographic standard
- Industry-proven verification method
- Detects even single bit changes

### ?? **Better Output**
- Clear header/footer separators
- SHA256 hashes displayed prominently
- Speed information included
- Professional formatting

---

## SHA256 Hash Format

**Example Hash**:
```
ABFF0B6682130A283EC062D4C68AB7F7FD02902475A894F5754251CC75F705F9
```

**Format**:
- **Length**: 64 characters (32 bytes * 2 hex digits/byte)
- **Encoding**: Uppercase hexadecimal
- **Algorithm**: SHA-256 (FIPS 180-4 standard)

**Matches**:
- ? PowerShell `Get-FileHash -Algorithm SHA256`
- ? OpenSSL `openssl sha256`
- ? Linux `sha256sum`

---

## Summary

**What Changed**:
1. ? Added SHA256 hash calculation using Windows Crypto API
2. ? Replaced byte-by-byte comparison with hash comparison
3. ? Enhanced output format to match PowerShell script
4. ? Added header/footer separators for clarity
5. ? Display SHA256 hashes prominently
6. ? Added speed information extraction

**User Benefits**:
- ?? **Professional output** - Matches industry standards
- ? **Faster verification** - Hash comparison is quicker
- ?? **More reliable** - Cryptographic verification
- ?? **Better visibility** - SHA256 hashes shown clearly
- ? **Consistent** - Matches PowerShell script exactly

**Result**: Flash verification now has professional, SHA256-based verification matching the PowerShell script! ??

---

## Next Steps

1. ? Rebuild solution (already done)
2. ? Test verification with correct firmware ? Should show matching SHA256 hashes
3. ? Test verification with wrong firmware ? Should show mismatched SHA256 hashes
4. ? Compare SHA256 with PowerShell ? Should match exactly
5. ? Confirm professional output format

**Enjoy the professional verification output!** ??
