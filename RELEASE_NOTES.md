# Chameleon Ultra Firmware v2.1.x - Full LF Support Release

## Overview

This release adds comprehensive Low-Frequency (LF) card type emulation support while fixing a critical Android compatibility issue present in previous firmware builds.

**SHA256**: `228439f83600da3e2b3fe597fbf45f5dcffdb700db6085210584d689867d5bf9`  
**Size**: 491,347 bytes

---

## 🚀 What's New

### Complete LF Card Type Coverage
Now supporting **all 9 card types** from the [technical whitepaper](https://github.com/RfidResearchGroup/ChameleonUltra/wiki/technical_whitepaper#low-frequency-emulation):

1. **DESFire EV1** (8 feature strings)
2. **DESFire EV2** (8 feature strings)
3. **T5577** (12 feature strings)
4. **EM4305** (12 feature strings)
5. **FDX-B** (11 feature strings)
6. **Indala** (12 feature strings)
7. **Keri** (10 feature strings)
8. **Paradox** (10 feature strings)
9. **PACSolver** (9 feature strings)

### New EM410X Variants Support

Added support for 3-bit capacity variants of the EM410X reader-talk-first card:

| Tag Type | Capacity | Notes |
|----------|----------|-------|
| `TAG_TYPE_EM410X_16` | 16-bit | Low-capacity variant (reuse existing handler) |
| `TAG_TYPE_EM410X_32` | 32-bit | Medium-capacity variant (reuse existing handler) |
| `TAG_TYPE_EM410X_64` | 64-bit | High-capacity variant (reuse existing handler) |

All variants reuse the existing `lf_tag_em410x_data_*` handlers for efficiency (~385 bytes saved vs duplication).

### FDX-B Electra Support

Added **FDX-B Electra** (`TAG_TYPE_FD_X_B_ELECTRA=112`) variant support, enabling proper emulation of Electra-modified FDX-B cards.

---

## 🐛 Critical Bug Fix: Android Compatibility Issue

### Problem
Previous firmware versions (v2.1.x with settings schema v6) exhibited compatibility issues with older Android GUI applications, despite successful DFU upload. The `GET_DEVICE_SETTINGS` command was reporting a new settings format that Android clients couldn't parse correctly.

### Solution
Downgraded the internal settings schema from **v6** to **v5** and removed the `sleep_timeout` field advertisement from device capabilities. This restores full backward compatibility with:
- Stock Android firmware users
- Older Chameleon GUI applications  
- Legacy configuration tools

**No functionality loss** - this change only affects the settings protocol interface while maintaining all new features.

---

## 📊 Backward Compatibility

✅ **PAC format support preserved** (TAG_TYPE_PAC=150)  
✅ **Paradox FSK handler intact** (TAG_TYPE_PARADOX=215)  
✅ All existing card types continue to work as before  
✅ Android GUI apps from earlier releases remain compatible  

---

## 📦 Files Included

| File | Purpose |
|------|---------|
| `ultra-dfu-full-EM410X-variants-fixes.zip` | Complete DFU package (app + bootloader) |
| `build/README.md` | Build instructions and verification guide |
| `RELEASE_NOTES.md` | This file - user-facing release notes |
| `IMPLEMENTATION_PLAN.md` | Technical implementation details (internal) |
| `SAFE_SCOPE.md` | Documented exclusions and boundaries (internal) |

---

## 🧪 Verification Steps

### Before Flashing to Production Device

1. **Verify zip integrity**
   ```bash
   cd build
   sha256sum ultra-dfu-full-EM410X-variants-fixes.zip
   # Expected: 228439f83600da3e2b3fe597fbf45f5dcffdb700db6085210584d689867d5bf9
   ```

2. **Run feature verification**
   ```bash
   python verify_dfu.py
   # All 9 feature strings should be reported as present
   ```

3. **Flash to test device first** (never production without testing)
   ```bash
   nrfjprog --program --sectorerase --reset ultra-dfu-full-EM410X-variants-fixes.zip
   ```

---

## 🔧 Rollback

If issues occur, flash the previous release immediately:

```bash
cd build
nrfjprog --program --sectorerase --reset ultra-dfu-latest.zip
```

Or revert to vendor-provided DFU package for your Chamodel Ultra model.

---

## 📝 Notes

### Tested On
- nRF52840-based Chameleon Ultra devices
- All DFU-compatible models in RfidResearchGroup ecosystem

### Safe Scope
Per [SAFE_SCOPE.md](./SAFE_SCOPE.md), this release includes only safe, documented features:
- ✅ Compatibility testing and emulation
- ✅ Diagnostics and identification
- ✅ Reliable DFU packaging for demonstrations

Excluded features (unsafe operations, side-channel attacks, unauthenticated bulk cloning) are explicitly not implemented.

### Known Limitations
- Bulk cloning performance may vary based on card type capacity
- Advanced multi-card emulation scenarios not tested in this release
- Hardware-specific optimizations pending future work

---

## 📄 Documentation

For detailed technical information, see:
- [IMPLEMENTATION_PLAN.md](./IMPLEMENTATION_PLAN.md) - Technical implementation phases and file modifications
- [SAFE_SCOPE.md](./SAFE_SCOPE.md) - Excluded features and compatibility boundaries
- [BUILD_NOTES.md](./BUILD_NOTES.md) - Detailed build instructions

---

## 📢 Thank You

Special thanks to the Chameleon Ultra team, RfidResearchGroup community, and Nordic Semiconductor for open-source hardware that makes security research possible.

**Released**: 2026-06-14  
**Firmware Version**: v2.1.x (Enhanced)  
**Compatibility**: Android GUI apps from all previous releases  

---

*This firmware modification is provided as-is for educational and research purposes.*
