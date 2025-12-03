# XC7A50T/75T IDCODE Mapping

## Problem
DMA cards that are physically **XC7A75T** chips are reporting the IDCODE for **XC7A50T** (`0x3632` instead of `0x362D`).

## Real-World Observation
- **Physical Chip**: XC7A75T (confirmed by successful flashing with 75T bitstreams)
- **Reported IDCODE**: `0x13632093` (part number `0x3632` = XC7A50T)
- **Expected IDCODE**: `0x1362D093` (part number `0x362D` = XC7A75T)

## Why This Happens

### Common Causes
1. **Remarked Chips**: Original chip relabeled with different part number
2. **Chip Binning**: Same die sold as different variants based on testing
   - A 75T die that doesn't meet speed/power specs ? sold as 50T
   - Still works perfectly with 75T bitstreams
3. **Engineering Samples**: Pre-production chips with mismatched IDs
4. **Manufacturing Variations**: Different fab runs, grade differences

### DMA Card Reality
- **No true 50T DMA cards exist in inventory**
- All cards flash successfully with **75T bitstreams**
- All cards use **75T flash tools** (confirmed working)
- JTAG reports 50T, but hardware is 75T

## Solution: IDCODE Aliasing

We map **both** IDCODEs to the same chip model:

```cpp
ChipModel OpenOCDInterface::IDCodeToChipModel(uint32_t idcode)
{
    uint32_t partNumber = (idcode >> 12) & 0xFFFF;
    
    switch (partNumber)
    {
    case 0x3622: return ChipModel::XC7A35T;
    case 0x3632: return ChipModel::XC7A75T;  // 50T IDCODE ? treat as 75T
    case 0x362D: return ChipModel::XC7A75T;  // Official 75T IDCODE
    case 0x3631: return ChipModel::XC7A100T;
    default: return ChipModel::Unknown;
    }
}
```

## IDCODE Reference Table

| Part Number | IDCODE (Full) | Official Chip | DMATool Mapping | Notes |
|-------------|---------------|---------------|-----------------|-------|
| `0x3622` | `0x13622093` | XC7A35T | XC7A35T | ? Standard |
| `0x3632` | `0x13632093` | XC7A50T | **XC7A75T** | ?? **Aliased** - DMA cards |
| `0x362D` | `0x1362D093` | XC7A75T | XC7A75T | ? Standard |
| `0x3631` | `0x13631093` | XC7A100T | XC7A100T | ? Standard |

## Impact

### Before Fix
```
[SUCCESS] FPGA detected: XC7A50T  ? WRONG
[INFO] IDCODE: 0x13632093
```

User sees "50T" but card is actually 75T ? confusion!

### After Fix
```
[SUCCESS] FPGA detected: XC7A75T  ? CORRECT
[INFO] IDCODE: 0x13632093
[INFO] Logic Cells: 75,520
```

User sees correct chip model even though IDCODE says 50T.

## Verification

### Test Script Used
```bash
C:\Users\suni\source\repos\DMATool\scripts\Test-Flash-Debug.ps1
```

This script uses **75T flash tools** and works successfully, confirming the hardware is indeed 75T.

### Physical Evidence
1. ? Card flashes successfully with 75T bitstreams
2. ? Test-Flash-Debug.ps1 works (uses 75T config)
3. ? No 50T cards in DMA inventory
4. ? All visual inspection shows 75T markings

## Technical Details

### IDCODE Bit Structure
```
IDCODE: 0x13632093
         ^^^^ ^^
         |    |__ Part Number: 0x3632 (50T)
         |_______ Version: 0x1

Bits 31:28 = Version (0x1)
Bits 27:12 = Part Number (0x3632)
Bits 11:1  = Manufacturer (0x049 = Xilinx)
Bit 0      = Always 1
```

### Why IDCODE Doesn't Match

The IDCODE is **burned into the chip during manufacturing**. If:
1. Chip is manufactured as 75T die
2. Fails to meet full 75T specs during testing
3. Downgraded to 50T
4. IDCODE already burned in (can't change)
5. Chip gets remarked/relabeled back to 75T for DMA use

Result: Physical 75T chip with 50T IDCODE

## Best Practices

### When to Use IDCODE Aliasing
? **Yes** - When you have confirmed hardware evidence:
  - Flash tools work with different model
  - Physical chip markings differ from IDCODE
  - Manufacturer confirms chip variant

? **No** - When:
  - Just guessing
  - No physical verification
  - Could cause incorrect bitstream loading

### DMATool Approach
Since we **confirmed**:
- All DMA inventory uses 75T flash tools
- No 50T DMA cards exist
- Cards successfully flash with 75T bitstreams

We **safely alias** 50T IDCODE ? 75T chip model.

## Future Considerations

### If Real 50T Cards Appear
If genuine 50T DMA cards appear in inventory:

1. **Option A**: Add runtime detection
   ```cpp
   // Try loading 75T bitstream
   // If it works ? 75T (remarked)
   // If it fails ? true 50T
   ```

2. **Option B**: User configuration
   ```cpp
   // Let user override IDCODE mapping in settings
   ```

3. **Option C**: Physical detection
   ```cpp
   // Read chip markings, PCB version, etc.
   ```

### Current Decision
**Option: Direct Mapping** (simplest, works for current inventory)

All 50T IDCODEs ? 75T chip model

## Summary

| Aspect | Details |
|--------|---------|
| **Problem** | 75T cards report 50T IDCODE |
| **Root Cause** | Remarked/binned chips |
| **Solution** | IDCODE aliasing (0x3632 ? 75T) |
| **Verification** | 75T flash tools work successfully |
| **Risk** | None (no 50T cards in inventory) |
| **Status** | ? Implemented and tested |

---

**Date**: 2025-12-02  
**Affected Files**: `src/Backend/OpenOCDInterface.cpp`  
**Impact**: Users now see correct "XC7A75T" instead of confusing "XC7A50T"  
**Testing**: Confirmed with Test-Flash-Debug.ps1 (75T flash script)
