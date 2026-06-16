# Chameleon Ultra - Build Notes & Verification Guide

## Repository Location
```powershell
C:\Users\mirot\.openclaw\workspace\ChameleonUltra
```

⚠️ **DO NOT** use `C:\github\ChameleonUltra`

## Prerequisites

### Tools Located On System
```powershell
# Verify tools are available
where.exe make
where.exe arm-none-eabi-gcc
where.exe nrfutil
where.exe git
```

### Known Working Paths
- **Make:** `C:\ProgramData\chocolatey\bin\make.exe`
- **GCC ARM:** `C:\ProgramData\chocolatey\lib\gcc-arm-embedded\tools\gcc-arm-none-eabi-10.3-2021.10\bin\arm-none-eabi-gcc.exe`
- **nrfutil:** `C:\Users\mirot\.nrfutil\bin\nrfutil.exe`
- **Git Binary:** `C:\Program Files\Git\usr\bin`

### Nordic SDK Location
```powershell
C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\nrf52_sdk
```

## Environment Setup

Open PowerShell and run:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

# Set environment variables
$env:Path = 'C:\Program Files\Git\usr\bin;C:\ProgramData\chocolatey\lib\gcc-arm-embedded\tools\gcc-arm-none-eabi-10.3-2021.10\bin;' + $env:Path

$gnuRoot = 'C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/gcc-arm-none-eabi-10.3-2021.10/bin/'
```

## Build Targets

### Application Binary

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\application

make application `
  GNU_INSTALL_ROOT="$gnuRoot" `
  GNU_PREFIX=arm-none-eabi `
  APP_FW_VER_MAJOR=2 `
  APP_FW_VER_MINOR=2 `
  GIT_VERSION=<VERSION_STRING>
```

**Expected Success Output:**
```text
Linking target: ../objects/application.out
Preparing: ../objects/application.hex
Preparing: ../objects/application.bin
DONE application
```

### Bootloader Binary

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\bootloader

make bootloader `
  GNU_INSTALL_ROOT="$gnuRoot" `
  GNU_PREFIX=arm-none-eabi `
  APP_FW_VER_MAJOR=2 `
  APP_FW_VER_MINOR=2 `
  GIT_VERSION=<VERSION_STRING>
```

**Expected Success Output:**
```text
Linking target: ../objects/bootloader.out
Preparing: ../objects/bootloader.hex
Preparing: ../objects/bootloader.bin
DONE bootloader
```

### Important Build Notes

⚠️ **NEVER run `make clean application` together with another build target**  
The Nordic Makefile rejects combined clean+build operations.

For a complete rebuild:
```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\application
make clean GNU_INSTALL_ROOT="$gnuRoot" GNU_PREFIX=arm-none-eabi APP_FW_VER_MAJOR=2 APP_FW_VER_MINOR=2 GIT_VERSION=<VERSION_STRING>
make application GNU_INSTALL_ROOT="$gnuRoot" GNU_PREFIX=arm-none-eabi APP_FW_VER_MAJOR=2 APP_FW_VER_MINOR=2 GIT_VERSION=<VERSION_STRING>
```

## DFU Package Creation

### Generate Full DFU ZIP

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

$nrf = 'C:\Users\mirot\.nrfutil\bin\nrfutil.exe'
$out = 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'

# Create output directory if needed
New-Item -ItemType Directory -Force -Path 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build' | Out-Null

# Preserve previous build if exists for comparison
if (Test-Path $out) {
  Copy-Item -LiteralPath $out -Destination 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full-before-build.zip' -Force
  Remove-Item -LiteralPath $out -Force
}

# Generate signed DFU package
& $nrf nrf5sdk-tools pkg generate `
  --hw-version 0 `
  --sd-req 0x0100 `
  --sd-id 0x0100 `
  --application-version 1 `
  --bootloader-version 1 `
  --softdevice 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\nrf52_sdk\components\softdevice\s140\hex\s140_nrf52_7.2.0_softdevice.hex' `
  --bootloader 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\objects\bootloader.hex' `
  --application 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\objects\application.hex' `
  --key-file 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\resource\dfu_key\chameleon.pem' `
  $out
```

### Verify DFU Package Contents

```powershell
& $nrf nrf5sdk-tools pkg display 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'
```

**Expected Output:**
- Image count: 2
- Image #0 type: sd_bl (SoftDevice + Bootloader)
- Image #1 type: application
- Application size: ~294160 bytes
- Signature type: ECDSA_P256_SHA256

### File Information

```powershell
Get-Item 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip' | Select-Object FullName,Length,LastWriteTime
Get-FileHash -Algorithm SHA256 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'
```

## Feature String Verification

### Verify All Expected Features Present

```powershell
@'
import zipfile, os, hashlib
p = r'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'
needles = [b'DESFire EV1', b'DESFire EV2', b'T5577', b'EM4305', b'FDX-B', b'Indala', b'Keri', b'Paradox', b'PAC']
with zipfile.ZipFile(p) as z:
 app = z.read('application.bin')
 print('zip_size', os.path.getsize(p))
 print('zip_sha256', hashlib.sha256(open(p,'rb').read()).hexdigest())
 print('app_size', len(app))
 print('app_sha256', hashlib.sha256(app).hexdigest())
 for n in needles:
  print(n.decode().ljust(12), app.lower().count(n.lower()))
'@ | python -
```

**Expected Results:**
- Every listed feature should have count > 0
- `app_size` should be around 294160 bytes

### PAC/Paradox GUI Compatibility Check

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

rg -n "TAG_TYPE_PAC|TAG_TYPE_PARADOX|paradox_data" `
 firmware/application/src/rfid/nfctag/tag_emulation.c `
 firmware/application/src/rfid/nfctag/tag_base_type.h `
 firmware/application/src/rfid/nfctag/lowfreq/paradox_fsk.c
```

**Required Source State:**
```c
// tag_base_type.h should show:
TAG_TYPE_PAC = 150       // Must use original lf_tag_* handlers
TAG_TYPE_PARADOX = 215   // Must use separate paradox_* handlers

// tag_emulation.c handlers must be distinct:
{TAG_SENSE_LF, TAG_TYPE_PAC, lf_tag_data_loadcb, lf_tag_pac_data_savecb, lf_tag_pac_data_factory, &m_tag_data_lf},
{TAG_SENSE_LF, TAG_TYPE_PARADOX, paradox_data_loadcb, paradox_data_savecb, paradox_data_factory, &m_tag_data_lf_extended},
```

## Testing Checklist

Before declaring firmware flash-ready:

### Build Verification
- [ ] `make application` completes successfully
- [ ] `make bootloader` completes successfully
- [ ] No compiler or linker errors/warnings
- [ ] Objects directory contains `.hex`, `.bin`, `.out` files

### DFU Package Verification  
- [ ] `nrfutil nrf5sdk-tools pkg display` shows 2 images
- [ ] Image #0 is type sd_bl (SoftDevice + Bootloader)
- [ ] Image #1 is type application
- [ ] Application size ~294KB
- [ ] ZIP file size ~491KB

### Feature String Verification
- [ ] All 9 required features present in application.bin
- [ ] No missing protocol handlers detected

### Compatibility Verification
- [ ] TAG_TYPE_PAC handler uses original implementation
- [ ] TAG_TYPE_PARADOX has separate handler
- [ ] Android app settings/buttons work with existing cards

## DFU Package Contents (Expected Structure)

```
ultra-dfu-full.zip contains:
├── application.bin          # Application firmware
├── application.hex          # Hex dump of application
├── bootloader.hex           # Bootloader hex dump
├── softdevice.hex           # SoftDevice S140 7.2.0
└── [metadata files]         # Nordic toolchain metadata
```

## Known Working Configuration

**SoftDevice:** S140, Version 7.2.0, ID 0x0100  
**Hardware Version:** NRF52840 (hw-version 0)  
**Application Version:** 2.2  
**Git Version:** Custom string for tracking

## Rollback Procedure

If build fails or verification checks fail:

```powershell
# Get previous commit hash
git log -1 --format=%H

# Revert changes to key files
git checkout <commit-hash>-- firmware/application/src/rfid/nfctag/tag_emulation.c

# Rebuild
make application bootloader GIT_VERSION=<safe-version>

# Recreate DFU package
nrfutil nrf5sdk-tools pkg generate ... ultra-dfu-full.zip
```

## Verification Command Summary

Quick verification checklist:
```powershell
# 1. Check build outputs exist
Test-Path "firmware\objects\application.bin"
Test-Path "firmware\objects\bootloader.hex"

# 2. Verify DFU package
nrfutil nrf5sdk-tools pkg display build\ultra-dfu-full.zip

# 3. Get SHA256 hashes
Get-FileHash -Algorithm SHA256 build\ultra-dfu-full.zip

# 4. Check feature strings
python check_features.py  # See full script in BUILD_NOTES.md

# 5. Verify tag type definitions
rg "TAG_TYPE_PAC|TAG_TYPE_PARADOX" firmware/application/src/rfid/nfctag/tag_base_type.h
```

---
*Last updated: 2026-06-14 16:48 GMT+2*  
*Build environment: Windows 10.0.26200 (x64) / PowerShell*
