# Chameleon Ultra - Test Vectors for LF Card Types

## Overview
This document contains test vectors and verification procedures for Low-Frequency (125kHz) card type implementations.

## Test Framework Structure

```
tests/
├── test_vectors.md          # This file
├── run_tests.sh             # Automated test runner (optional)
└── expected_outputs/        # Expected results for comparison
    ├── em410x_16_expected.txt
    ├── t5577_expected.txt
    └── ...
```

## Test Cases by Card Type

### EM410X Variants (ASK Modulation)

#### TAG_TYPE_EM410X_ELECTRA (104)
**Test Vector:**
```c
uint8_t em410x_electra_test[12] = {
    0xDE, 0xAD, 0xBE, 0xEF,    // Standard preamble
    0x88, 0x00, 0x00, 0x00,    // Fixed header
    0x00, 0x00, 0x00, 0x00     // Data area
};
```
**Expected Behavior:**
- Tag ID size: 12 bytes
- Modulation: ASK (Amplitude Shift Keying)
- Bit timing: ~10ms total transmission

#### TAG_TYPE_EM410X_16 (101)
**Test Vector:**
```c
uint8_t em410x_16_test[20] = {
    0xDE, 0xAD, 0xBE, 0xEF,
    0x01, 0x00, 0x00, 0x00,    // 16-bit UID area
    0xFF, 0xFF, 0xFF, 0xFF,    // Data padding
    0x88, 0x00, 0x00, 0x00,    // Footer pattern
    0x00, 0x10                  // Length byte (16 bytes)
};
```

#### TAG_TYPE_EM410X_32 (102)
**Test Vector:**
```c
uint8_t em410x_32_test[36] = {
    0xDE, 0xAD, 0xBE, 0xEF,
    // ... 32-bit UID area ...
    0xFF, 0xFF, 0xFF, 0xFF,
    0x88, 0x00, 0x00, 0x00,
    0x20                       // Length byte (32 bytes)
};
```

#### TAG_TYPE_EM410X_64 (103)  
**Test Vector:**
```c
uint8_t em410x_64_test[64] = {
    0xDE, 0xAD, 0xBE, 0xEF,
    // ... 64-bit UID area ...
    0xFF, 0xFF, 0xFF, 0xFF,
    0x88, 0x00, 0x00, 0x00,
    0x40                       // Length byte (64 bytes)
};
```

### FDX-B Variants (FSK Modulation)

#### TAG_TYPE_FD_X_B_ELECTRA (112)
**Test Vector:**
```c
uint8_t fdx_b_electra_test[32] = {
    0xA5, 0xA5,                // FDX-B sync pattern
    0x01, 0x02, 0x03, 0x04,    // UID bytes
    0xFF, 0xFF, ...            // Data area
};
```

## Verification Tests

### Test: Tag Type Registration
**Verify:** All new tag types are registered in tag_emulation.c

```powershell
# Run after implementing new card types
rg -n "TAG_TYPE_EM410X_[0-9]" firmware/application/src/rfid/nfctag/tag_emulation.c | Find-String "loadcb|savecb|factory"
```

**Expected:** Each EM410X variant should have all three handler callbacks referenced.

### Test: Buffer Size Validation  
**Verify:** New implementations don't exceed 20-byte buffer limit

```c
// For each new tag type implementation, verify:
ASSERT(buffer->length <= sizeof(m_tag_data_lf));  // Should be 20 bytes
```

### Test: Factory Data Generation
**Verify:** Each card type generates valid factory data on first use

```powershell
# Expected log output for factory initialization:
Load tag slot config done.
Factory slot data success.  # Should appear once per slot
```

## Integration Tests

### Test: Multi-Card Slot Coexistence
**Scenario:** Slot 0 with EM410X (LF) and MIFARE Classic (HF)

**Setup Commands:**
```c
// In tag_emulation_factory_init() or similar initialization:
slotConfig.slots[0].tag_lf = TAG_TYPE_EM410X_16;
slotConfig.slots[0].enabled_lf = true;
slotConfig.slots[0].tag_hf = TAG_TYPE_MIFARE_1024;
slotConfig.slots[0].enabled_hf = true;
```

**Expected:** Both cards should be emulatable independently without interference.

### Test: Android GUI Detection
**Verify:** Android app can detect new card types (for existing supported cards)

**Test Sequence:**
1. Flash firmware with new tag type entries
2. Open Android app
3. Navigate to settings → Card Configuration
4. Attempt to add/edit card with TAG_TYPE_EM410X_16

**Expected Result for NEW Types:** "Card not found in supported list" (not a bug - these aren't yet supported)

**Expected Result for EXISTING Types:** Settings/buttons work normally

### Test: CRC Validation
**Verify:** All implementations correctly calculate 14-bit CRC

```c
// Verify implementation matches reference:
uint8_t m_tag_data_lf_crc;
calc_16_bit_crc(m_tag_data_buffer_lf, sizeof(m_tag_data_buffer_lf) - 2, 
                (uint16_t*)&m_tag_data_lf_crc);
ASSERT(m_tag_data_lf_crc != 0xAAAA);  // Should not be all bits set
```

## Build Verification Tests

### Pre-Build Check: Source File Compilation Units

**Verify all new implementations compile:**
```powershell
cd firmware/application
make application GNU_INSTALL_ROOT="$gnuRoot" ... 2>&1 | Select-String "error|warning"
```

**Expected:** No compilation errors, zero or few warnings

### Post-Build Verification

**Verify DFU package structure:**
```powershell
# Should find these files in firmware/objects/:
Test-Path "firmware/objects/application.hex"
Test-Path "firmware/objects/application.bin" 
Test-Path "firmware/objects/bootloader.hex"
Test-Path "firmware/objects/bootloader.bin"
```

## Regression Tests

### Test: Existing PAC Compatibility
**Verify:** TAG_TYPE_PAC still works with original handler

**Setup:**
```c
// Ensure PAC uses original handlers (not replaced by Paradox)
int lf_tag_pac_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer);
bool lf_tag_pac_data_factory(uint8_t slot, tag_specific_type_t type);
```

### Test: Existing Paradox Compatibility  
**Verify:** TAG_TYPE_PARADOX has separate handlers

**Setup:**
```c
// Should be different from PAC (different buffer)
{TAG_SENSE_LF, TAG_TYPE_PAC, lf_tag_pac_data_savecb, ..., &m_tag_data_lf},
{TAG_SENSE_LF, TAG_TYPE_PARADOX, paradox_data_savecb, ..., &m_tag_data_lf_extended},
```

### Test: Feature String Presence
**Verify:** All 9 feature strings in application.bin:

```powershell
python -c "
import zipfile, hashlib
with zipfile.ZipFile('build/ultra-dfu-full.zip') as z:
    app = z.read('application.bin')
features = ['DESFire EV1','DESFire EV2','T5577','EM4305','FDX-B','Indala',
            'Keri','Paradox','PAC']
for f in features:
    assert f.lower() in app.lower(), f'Missing feature: {f}'
print('All features present')
"
```

## Manual Testing Checklist

### Physical Card Testing (if test cards available)

For each new card type implementation:

- [ ] Reader can communicate with emulated card
- [ ] Correct UID/data is transmitted to reader
- [ ] CRC validation passes on write operations  
- [ ] Factory reset clears data properly
- [ ] No interference with simultaneously emulated HF card

### Software Testing Checklist

For each new card type implementation:

- [ ] Tag type appears in supported list (in tag_emulation.c)
- [ ] Handler callbacks defined and registered
- [ ] Buffer size within limits (≤20 bytes for LF)
- [ ] Factory data generation works
- [ ] CRC calculation correct

## Known Limitations

1. **Buffer Size:** All LF implementations limited to 20-byte buffer
   - Larger card types may need alternative storage strategy
   - Consider moving to m_tag_data_lf_extended if >64 bytes needed

2. **Modulation Support:**
   - ASK cards: Use base 125kHz PWM clock  
   - PSK1 cards (IDTECK, INDALA variants): Require 1MHz base clock
   - Check IS_PSK1_TYPE() macro for classification

3. **FSK Cards:** Require additional RF front-end support for dual-frequency operation

## Test Results Template

```
Test Date: YYYY-MM-DD
Firmware Version: vX.Y.Z
Git Commit: <hash>

[EM410X Variants]
  EM410X_16:      [PASS/FAIL] - Reason: ___
  EM410X_32:      [PASS/FAIL] - Reason: ___
  EM410X_64:      [PASS/FAIL] - Reason: ___

[FDX-B Variants]
  FDX-B Electra:  [PASS/FAIL] - Reason: ___

[Regression Tests]
  PAC Compatibility:    [PASS/FAIL]
  Paradox Compatibility: [PASS/FAIL]
  Feature Strings Present: [PASS/FAIL]

[Build Verification]
  Application Build:     [PASS/FAIL]  
  Bootloader Build:      [PASS/FAIL]
  DFU Package Generated: [PASS/FAIL]

Overall Status: [READY FOR FLASH / NEEDS FIXES]
```

---
*Last updated: 2026-06-14 16:50 GMT+2*
