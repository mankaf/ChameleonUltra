# Qwen Build Instructions: ChameleonUltra Firmware

This repo path is the only correct working directory:

```powershell
C:\Users\mirot\.openclaw\workspace\ChameleonUltra
```

Do not use `C:\github\ChameleonUltra`.

## Goal

Build the ChameleonUltra application the same way the successful GPT-5 run did, then create a signed full DFU ZIP containing:

- SoftDevice
- Bootloader
- Application

Expected output:

```powershell
C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip
```

## Important Context

The previous successful build fixed a GUI/settings regression by keeping the old PAC handler intact and giving Paradox its own type:

- `TAG_TYPE_PAC = 150` must use original `lf_tag_*pac*` handlers.
- `TAG_TYPE_PARADOX = 215` must use the new `paradox_*` handlers.

Do not replace PAC with Paradox. That breaks existing Android GUI/card settings behavior.

The working package after this fix had:

- ZIP size: about `491347` bytes
- Application size inside DFU: about `294160` bytes
- Feature strings present: `DESFire EV1`, `DESFire EV2`, `T5577`, `EM4305`, `FDX-B`, `Indala`, `Keri`, `Paradox`, `PAC`

## Locate Tools

Use `where.exe` if a command is not found:

```powershell
where.exe make
where.exe arm-none-eabi-gcc
where.exe nrfutil
where.exe git
```

Known working paths from GPT-5 run:

```powershell
C:\ProgramData\chocolatey\bin\make.exe
C:\ProgramData\chocolatey\lib\gcc-arm-embedded\tools\gcc-arm-none-eabi-10.3-2021.10\bin\arm-none-eabi-gcc.exe
C:\Users\mirot\.nrfutil\bin\nrfutil.exe
C:\Program Files\Git\usr\bin
```

The Nordic SDK is already inside the repo:

```powershell
C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\nrf52_sdk
```

## Build Commands

Open PowerShell and run exactly from this repo:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

$env:Path = 'C:\Program Files\Git\usr\bin;C:\ProgramData\chocolatey\lib\gcc-arm-embedded\tools\gcc-arm-none-eabi-10.3-2021.10\bin;' + $env:Path

$gnuRoot = 'C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/gcc-arm-none-eabi-10.3-2021.10/bin/'
```

Build the application:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\application

make application `
  GNU_INSTALL_ROOT="$gnuRoot" `
  GNU_PREFIX=arm-none-eabi `
  APP_FW_VER_MAJOR=2 `
  APP_FW_VER_MINOR=2 `
  GIT_VERSION=gui-fix
```

Expected success output includes:

```text
Linking target: ../objects/application.out
Preparing: ../objects/application.hex
Preparing: ../objects/application.bin
DONE application
```

Build the bootloader:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\bootloader

make bootloader `
  GNU_INSTALL_ROOT="$gnuRoot" `
  GNU_PREFIX=arm-none-eabi `
  APP_FW_VER_MAJOR=2 `
  APP_FW_VER_MINOR=2 `
  GIT_VERSION=gui-fix
```

Expected success output includes:

```text
Linking target: ../objects/bootloader.out
Preparing: ../objects/bootloader.hex
Preparing: ../objects/bootloader.bin
DONE bootloader
```

## Notes About Clean

Do not run:

```powershell
make clean application
```

The Nordic Makefile rejects `clean` combined with another target.

If you need a clean build, run clean separately:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra\firmware\application
make clean GNU_INSTALL_ROOT="$gnuRoot" GNU_PREFIX=arm-none-eabi APP_FW_VER_MAJOR=2 APP_FW_VER_MINOR=2 GIT_VERSION=gui-fix
make application GNU_INSTALL_ROOT="$gnuRoot" GNU_PREFIX=arm-none-eabi APP_FW_VER_MAJOR=2 APP_FW_VER_MINOR=2 GIT_VERSION=gui-fix
```

If Windows says `../objects` is busy during clean, do not force-delete it. Just run the incremental `make application` command above.

## Create Full DFU ZIP

Run:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

$nrf = 'C:\Users\mirot\.nrfutil\bin\nrfutil.exe'
$out = 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'

New-Item -ItemType Directory -Force -Path 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build' | Out-Null
if (Test-Path $out) {
    Copy-Item -LiteralPath $out -Destination 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full-before-qwen-build.zip' -Force
    Remove-Item -LiteralPath $out -Force
}

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

Verify package:

```powershell
& $nrf nrf5sdk-tools pkg display 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'
Get-Item 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip' | Select-Object FullName,Length,LastWriteTime
Get-FileHash -Algorithm SHA256 'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip'
```

Expected `pkg display`:

- Image count: `2`
- Image #0 type: `sd_bl`
- Image #1 type: `application`
- Application size around `294160`
- Signature type: `ECDSA_P256_SHA256`

## Verify Feature Strings

Run this exact Python check:

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

Expected:

- Every listed feature should have a count greater than `0`.
- `app_size` should be around `294160`.

## Verify PAC/Paradox GUI Compatibility Fix

Run:

```powershell
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

rg -n "TAG_TYPE_PAC|TAG_TYPE_PARADOX|paradox_data" `
  firmware/application/src/rfid/nfctag/tag_emulation.c `
  firmware/application/src/rfid/nfctag/tag_base_type.h `
  firmware/application/src/rfid/nfctag/lowfreq/paradox_fsk.c
```

Required source state:

```c
TAG_TYPE_PAC = 150
TAG_TYPE_PARADOX = 215
```

In `tag_emulation.c`, PAC must look like this:

```c
{TAG_SENSE_LF, TAG_TYPE_PAC, lf_tag_data_loadcb, lf_tag_pac_data_savecb, lf_tag_pac_data_factory, &m_tag_data_lf},
```

Paradox must be separate:

```c
{TAG_SENSE_LF, TAG_TYPE_PARADOX, paradox_data_loadcb, paradox_data_savecb, paradox_data_factory, &m_tag_data_lf_extended},
```

## Final Deliverable

Give Miroslav this file:

```powershell
C:\Users\mirot\.openclaw\workspace\ChameleonUltra\build\ultra-dfu-full.zip
```

Tell him to flash this package and test Android GUI settings/buttons first, then test new card features.
