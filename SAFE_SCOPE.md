# Chameleon Ultra - Safe Firmware Scope

## Document Purpose

This document defines the **excluded and limited features** of this firmware fork to ensure safe, authorized use only for physical security demonstrations on owned/test devices with synthetic demo data.

## Approved Use Cases

✅ **Authorized Uses:**
- Physical security demonstration setups using owned/test hardware
- Training exercises with controlled environments
- Educational demonstrations of RFID emulation principles
- Testing authentication systems in isolation
- Research and development with permission

❌ **Prohibited Uses:**
- Unauthorized access attempts to secured facilities
- Production environment attacks without explicit authorization
- Commercial resale or distribution
- Use on third-party devices without written permission
- Any activity that compromises legitimate security controls


### LIMITED-BYTE IMPLEMENTATIONS (Size Constraints)

#### 1. Limited Data Buffer Sizes
- **Small buffers:** 20 bytes for basic LF cards (EM410X, T5577, PAC, etc.)
- **Extended buffers:** 64 bytes for larger card types
- **Large card types:** May not be fully supported

**Rationale:** Maintains compatibility with existing DFU packaging constraints and prevents buffer overflow vulnerabilities.

#### 2. No Multi-Card Active Emulation (LF)
Current implementation: One active LF card at a time per slot  
**Excluded:** Concurrent emulation of multiple cards simultaneously

**Reason:** Adds complexity and potential for race conditions; focus on single-card demonstration use cases.

### COMPATIBILITY EXCLUSIONS

#### 1. Android GUI Settings Compatibility
**Maintained:** Existing PAC (TAG_TYPE_PAC = 150) and Paradox (TAG_TYPE_PARADOX = 215) handlers  
**Not Modified:** No changes to original handler functions to ensure Android app compatibility

**Verified Features in Existing Build:**
- ZIP size: ~491347 bytes
- Application size: ~294160 bytes
- Feature strings: DESFire EV1/2, T5577, EM4305, FDX-B, Indala, Keri, Paradox, PAC

#### 2. No Protocol Command Exposure
**Excluded:** CLI/GUI commands that expose raw protocol details  
**Reason:** Prevents unintended information disclosure about implementation specifics

### DEMO DATA EXCLUSIONS

All implementations must:
- Use synthetic/demo data only (no real credential data)
- Support factory reset functionality for each card type
- Include proper CRC validation to prevent data corruption
- Allow explicit factory initialization per card slot

## Implementation Guidelines

### For Adding New Card Types

1. **Size Validation:** Always validate that card data fits within buffer limits before saving
2. **CRC Protection:** Implement proper CRC calculation and verification for all operations
3. **Factory Support:** Provide factory reset functionality for each implementation
4. **Data Isolation:** Each card type uses independent storage slots
5. **Documentation:** Add comments explaining protocol specifics for future maintainers

### Testing Requirements

Each new card type implementation must pass:
- [ ] Factory data load verification
- [ ] Data save and persistence validation
- [ ] CRC integrity checks
- [ ] Android GUI detection (if applicable)
- [ ] No regression in existing card types

## Build Verification Checklist

Before considering firmware release:

- [x] Original PAC handler intact (TAG_TYPE_PAC = 150)
- [x] Paradox handler separate (TAG_TYPE_PARADOX = 215)  
- [x] All original feature strings present in DFU package
- [x] Android app settings work with existing card types
- [ ] New card type handlers don't exceed buffer limits
- [ ] No buffer overflow vulnerabilities introduced
- [ ] SHA256 hash verification passes

## Emergency Rollback Procedure

If any issues arise:

1. **Immediate Action:** Rebuild firmware with previous commit
   ```powershell
   git revert <commit-hash>~1
   make application bootloader ultra-dfu-full.zip
   ```

2. **Verification Commands:**
   ```powershell
   nrfutil nrf5sdk-tools pkg display build\ultra-dfu-full.zip
   Get-FileHash -Algorithm SHA256 build\ultra-dfu-full.zip
   ```

3. **Rollback Criteria:**
   - DFU package size differs by >10% from expected ~491KB
   - Application section size differs by >5% from expected ~294KB  
   - Missing feature strings in application.bin
   - Android app fails to detect cards it previously supported

## Code Review Sign-Off

All implementations must be reviewed against this scope. Changes outside SAFE_SCOPE require:
- Written authorization from project maintainers
- Security impact assessment
- Updated SAFE_SCOPE.md document

---
*Last updated: 2026-06-14 16:48 GMT+2*  
*This is a safety-by-design document. All exclusions are intentional.*
