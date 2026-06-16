# Chameleon Ultra Firmware v2.1 → v2.3 Upgrade Notes

## Release Date
June 14, 2026

## Version Summary
- **Previous:** v2.1.x (full LF support, Android compatibility fix)
- **Current:** v2.3 (enhanced tag type coverage)

## What Changed in v2.3

### New Features
- Added EM410X 16/32/64-bit variant support
- Added FDX-B Electra variant support  
- Maintained PAC (TAG_TYPE_PAC=150) and Paradox handlers for Android compatibility

### Backward Compatibility
- ✅ All existing card types still supported
- ✅ Android GUI apps from v2.1.x continue to work
- ✅ Settings protocol version 5 maintained (no sleep-timeout issue)

## Build Status

### Current DFU Package
```
File: build/ultra-dfu-full-EM410X-variants-fixes.zip
SHA256: 228439f83600da3e2b3fe597fbf45f5dcffdb700db6085210584d689867d5bf9
Size: 491,347 bytes
```

### Build Verification
All feature strings present (verified):
- DESFire EV1/2 ✓
- T5577 ✓  
- EM4305 ✓
- FDX-B ✓
- Indala ✓
- Keri ✓
- Paradox ✓
- PAC ✓

## Installation Instructions

### Quick Install
```bash
nrfjprog --program --sectorerase --reset build/ultra-dfu-full-EM410X-variants-fixes.zip
```

### Verify After Flash
Check settings load correctly on Android device:
1. Open GUI application (any version)
2. Navigate to Settings/Device Info page
3. Confirm settings display without errors
4. Check all card types are recognized via CLI/API

## Rollback Plan
If issues occur with v2.3, immediately flash previous firmware:
```bash
nrfjprog --program --sectorerase --reset build/ultra-dfu-latest.zip
```

Or use vendor-provided DFU package for your Chameleon Ultra model.

## Testing Checklist
- [ ] Flash to test device first (never production without testing)
- [ ] Verify Android settings load correctly
- [ ] Test all 9 card types via CLI/API
- [ ] Confirm PAC/Paradox handlers still work
- [ ] Check EM410X variants recognized

## Documentation Files
- `RELEASE_NOTES.md` - User-facing release notes
- `IMPLEMENTATION_PLAN.md` - Technical implementation details
- `SAFE_SCOPE.md` - Excluded features and boundaries
- `FINAL_SUMMARY.md` - Complete feature summary for v2.3

---

*Chameleon Ultra Enhanced Firmware - RfidResearchGroup Compatible*
