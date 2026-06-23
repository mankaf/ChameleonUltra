# Chameleon Ultra - LF Card Types Implementation Plan

## Overview
This document outlines the implementation plan to add full support for all LF card types mentioned in the official Chameleon Ultra Technical Whitepaper, specifically focusing on Low-Frequency (125kHz) emulation capabilities.

## Current State Analysis

### Already Implemented LF Cards (tag_emulation.c)
The current codebase already has handlers for most standard LF card types:
- TAG_TYPE_EM410X (100) - EM4100/EM-Micro and variants
- TAG_TYPE_FD_X_B (105) - FDX-B
- TAG_TYPE_EM4305 (107) - EM4305
- TAG_TYPE_EM4305_64 (108) - EM4305 64-bit
- TAG_TYPE_T5577 (120) - HID Prox / T55xx series
- TAG_TYPE_PAC (150) - SecuKey/PAC cards
- TAG_TYPE_VIKING (170) - Viking cards
- TAG_TYPE_JABLOTRON (180) - Jablotron cards
- TAG_TYPE_HID_PROX (200) - HID proximity cards
- TAG_TYPE_IOPROX (203) - Ioprox cards
- TAG_TYPE_PARADOX (215) - Paradox cards
- TAG_TYPE_INDALA (305) - Indala cards
- TAG_TYPE_KERI (307) - Keri cards
- TAG_TYPE_IDTECK (310) - IDTECK cards
- TAG_TYPE_INDALA_20 (308) - Indala 20 series
- TAG_TYPE_KERI_V2 (309) - Keri V2

### Missing LF Card Implementations

Based on the Chameleon Ultra whitepaper and analysis of existing implementations, we need to add support for:

#### HIGH PRIORITY (Core Security Demo Cards)
1. **EM410X Extended Variants**
   - TAG_TYPE_EM410X_16 (101) - EM410 16-bit capacity
   - TAG_TYPE_EM410X_32 (102) - EM410 32-bit capacity  
   - TAG_TYPE_EM410X_64 (103) - EM410 64-bit capacity

2. **FDX-B Extended Variants**
   - TAG_TYPE_FD_X_B_ELECTRA (112) - FDX-B Electra variant

#### MEDIUM PRIORITY (Industry Standard Cards)
3. **Hitag Series** (Reader-Talk-First, 400+)
   - Hitag S/2/4 cards need implementation
   
4. **Indala V16/V32 variants**
   - TAG_TYPE_INDALA_16 variant for older Indala cards

#### LOW PRIORITY (Legacy/Niche)
5. **Other proprietary formats** that may be needed for specific demo scenarios

## Implementation Strategy

### Phase 1: Foundation Files Review & Extension
- [x] Review existing tag_emulation.c structure
- [ ] Add missing EM410X variant entries to tag_base_map[]
- [ ] Update TAG_SPECIFIC_TYPE_LF_VALUES define
- [ ] Verify data buffers support new card types

### Phase 2: Missing Protocol Implementations
- [ ] Create protocol implementations for any missing card types
- [ ] Use existing code patterns as templates
- [ ] Add to tag_emulation.c handlers

### Phase 3: Testing & Documentation
- [ ] Create test vectors for each new card type
- [ ] Update SAFE_SCOPE.md with documented limits
- [ ] Generate BUILD_NOTES.md with verification steps

### Phase 4: DFU Package Generation
- [ ] Build application and bootloader
- [ ] Create signed DFU ZIP package
- [ ] Verify all feature strings present
- [ ] Run build verification tests

## Key Design Decisions

### Backward Compatibility
- Maintain existing TAG_TYPE_PAC = 150 with original lf_tag_pac handlers
- Keep TAG_TYPE_PARADOX = 215 separate from PAC
- Do not replace PAC with Paradox handlers (breaks Android GUI compatibility)

### Data Storage Strategy
- Use m_tag_data_lf for LF cards that fit in small buffers (<64 bytes)
- Use m_tag_data_lf_extended for larger card data (up to 64 bytes)
- All implementations follow existing CRC validation patterns

### Code Structure Alignment
All new implementations must follow the pattern:
```c
int <card>_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer);
int <card>_data_savecb(tag_specific_type_t type, tag_data_buffer_t *buffer);
bool <card>_data_factory(uint8_t slot, tag_specific_type_t type);
```

## Files to Modify

### Primary Modifications
1. **firmware/application/src/rfid/nfctag/tag_emulation.c**
   - Add entries to tag_base_map[] array
   - Update TAG_SPECIFIC_TYPE_LF_VALUES define
   
2. **firmware/application/src/rfid/nfctag/tag_base_type.h**
   - Add new TAG_TYPE_ enum values if needed

### Test Files (New)
3. **tests/test_lf_emulation.c**
   - Create unit tests for each new card type
   - Verify data load/save operations

4. **firmware/application/src/rfid/nfctag/lf/*_data.c** (as needed)
   - Implement missing protocol handlers

## Build Verification Requirements

Before considering firmware flash-ready:
- [ ] Successful make application without errors
- [ ] Successful make bootloader without errors  
- [ ] Generated ultra-dfu-full.zip contains all 3 images
- [ ] SHA256 hash matches expected range (~491KB ZIP, ~294KB app)
- [ ] All feature strings present (DESFire EV1/2, T5577, EM4305, FDX-B, Indala, Keri, Paradox, PAC)
- [ ] Android GUI settings/buttons work with existing card types

## Risk Assessment

### Low Risk
- Adding new tag type entries to existing buffer handlers
- Extending TAG_SPECIFIC_TYPE_LF_VALUES array
  
### Medium Risk  
- New protocol implementations that require different data formats
- Buffer overflow if large card types exceed 64-byte limit

### Mitigation
- Always validate card data length before saving
- Use extended buffer for larger cards
- Comprehensive CRC validation on load/save

## Rollback Strategy

If new implementations cause issues:
1. Revert tag_emulation.c to last known good state
2. Rebuild firmware with original commit hash
3. DFU package includes version tracking via GIT_VERSION parameter

## Deliverables Checklist

- [x] IMPLEMENTATION_PLAN.md (this document)
- [ ] SAFE_SCOPE.md - documenting excluded unsafe features
- [ ] Firmware patches in small, incremental commits
- [ ] Tests/test vectors for each new card type
- [ ] BUILD_NOTES.md with exact build commands
- [ ] Generated DFU package: build/ultra-dfu-full.zip
- [ ] SHA256 hashes of generated artifacts
- [ ] PR-style summary with changed files, tests run, rollback notes

## Next Steps

1. Review existing protocol implementations to identify patterns
2. Check whitepaper for exact specifications on missing card types
3. Implement highest priority cards (EM410X variants) first
4. Test each implementation individually before batch building
5. Generate final DFU package with full verification

---
*Last updated: 2026-06-14 16:46 GMT+2*
