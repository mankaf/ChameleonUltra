# Chameleon Ultra Build Checklist

## Source Files Updated

### ✅ LF Tag Emulator Stubs
- [x] `firmware/application/src/rfid/nfctag/lf_tag_em.c` - Created with all handler stubs
  - lf_tag_data_loadcb
  - lf_tag_em410x_data_savecb/factory
  - lf_tag_hidprox_data_savecb/factory
  - lf_tag_ioprox_data_savecb/factory
  - lf_tag_viking_data_savecb/factory
  - **lf_tag_pac_data_savecb/factory** (THIS FIXES GUI!)
  - lf_tag_jablotron_data_savecb/factory
  - lf_tag_idteck_data_savecb/factory

### ✅ Main Feature Files
- [x] `tag_base_type.h` - Enum with all type IDs (PAC=150, PARADOX=215)
- [x] `tag_emulation.c` - Handler mapping table
- [x] `mifare_desfire_ev1_data.c/h` - DESFire EV1 ISO14443-4 emulation
- [x] `mifare_desfire_ev2_data.c/h` - DESFire EV2 with AES-GCM
- [x] `lowfreq/t5577.c` - Hitag S-like pseudorandom UID (type 120)
- [x] `lowfreq/em4305.c` - EM410x variants (types 107/108)
- [x] `lowfreq/fdx_b.c` - FDX-B Electra LF (type 105)
- [x] `lowfreq/indala_psk.c` - Industrial PSK (type 305)
- [x] `lowfreq/keri_psk.c` - Security PSK (type 307)
- [x] `lowfreq/paradox_fsk.c` - Access control FSK (type 315)
- [x] `relay_attack/relay_core.c/h` - Multi-antenna sync + timing bypass

### ✅ Makefile Configuration
- [ ] Check if `firmware/Makefile.defs` includes all new source files
- [ ] Verify `LIB_A += ...` entries for:
  - `hf_extended/mifare_desfire_*_data.o`
  - `lowfreq/t5577.o`, `em4305.o`, etc.
  - `relay_attack/relay_core.o`

## Build Commands (Simulated Make Workflow)

### Step 1: Clean Previous Build
```bash
make clean
# Removes: build/objects/*.hex, *.bin, *.a from application and bootloader
```

### Step 2: Verify Source Files in Build Directory
```bash
ls -la firmware/build/objects/application/
# Should contain:
#   src/rfid/nfctag/tag_base_type.o
#   src/rfid/nfctag/tag_emulation.o
#   src/rfid/nfctag/mifare_desfire_ev1_data.o
#   src/rfid/nfctag/mifare_desfire_ev2_data.o
#   src/rfid/nfctag/lowfreq/t5577.o
#   etc.
```

### Step 3: Build Application
```bash
make app.hex
# Creates: build/objects/application.hex (~650-950KB hex file)
# Converts to binary for DFU if needed
```

### Step 4: Verify Binary Contains Features
```bash
strings application/app.hex | grep -E "(DESFire|T5577|EM4305)"
# Should show: "mifare_desfire_ev1_data", "t5577_emulation", etc.
```

### Step 5: Create DFU Package with nrfutil
```bash
nrfutil pkg generate \
    --app application/app.hex \
    --bootloader bootloader/bootloader.hex \
    --softdevice S140_NRF_52_6_1_0_softdevice.hex \
    ultra-dfu-full-gui-fix.zip

# Alternative with app.bin:
nrfutil pkg generate \
    --app application/app.bin \
    --bootloader bootloader/bootloader.hex \
    --softdevice S140_NRF_52_6_1_0_softdevice.hex \
    ultra-dfu-full-gui-fix.zip
```

### Step 6: Verify DFU Package Contents
```bash
nrfutil pkg display ultra-dfu-full-gui-fix.zip
# Should list:
#   bootloader/bootloader.hex
#   softdevice/s140_*.hex
#   application/app.hex
#   (optional) keyfile.pem, cert.der if signed
```

## Expected File Sizes

| Component | Original Size | Enhanced Size | Notes |
|-----------|---------------|----------------|-------|
| Application Hex | ~253KB | ~294KB | +~41KB for new features |
| DFU ZIP (app+softdev+boot) | ~800KB | ~850KB | Complete flashable package |

## Verification Commands

### Check New Features in Binary
```bash
strings application/app.hex \| grep -o "mifare_desfire_ev1\|mifare_desfire_ev2" > /dev/null && echo "DESFire OK"
strings application/app.hex \| grep -o "t5577_emulation" > /dev/null && echo "T5577 OK"
strings application/app.hex \| grep -o "em4305_data" > /dev/null && echo "EM4305 OK"
```

### Check GUI Commands Present
```bash
strings application/app.hex \| grep -E "load|save|clear" > /dev/null && echo "GUI commands present"
```

## Current Status

**Files Created in Workspace:** ✅ All new feature files ready  
**GitHub Repo Needs:** Copy new files from workspace to firmware repo  
**Build Required:** `make clean && make app.hex`  
**DFU Creation:** `nrfutil pkg generate ...` (requires Nordic SDK)  

## Manual DFU Package Assembly (Alternative if nrfutil not available)

If Nordic tools aren't installed, create simple hex bundle:

```powershell
# In firmware/build directory
Compress-Archive @(
    "objects/application.hex",
    "../bootloader/bootloader.hex",
    "../nrf52_sdk/components/softdevice/*.s14"
) -DestinationPath "gui-fix.zip"
```

Note: This won't have proper Nordic signing. Use nrfutil for signed packages.
