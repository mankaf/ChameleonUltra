@echo off
REM Build Chameleon Ultra v2.3 with EM410X variants and settings compatibility fix

cd /d "%~dp0"
set GNU_INSTALL_ROOT="C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/gcc-arm-none-eabi-10.3-2021.10/bin/"

REM Apply version 2.3 to both application and bootloader Makefiles
cd firmware/application
findstr /I "export APP_FW_VER_MAJOR" Makefile >nul 2>&1
if errorlevel 1 (
    echo export APP_FW_VER_MAJOR := 2 >> Makefile
    echo export APP_FW_VER_MINOR := 2 >> Makefile
)

REM Check if GNU_INSTALL_ROOT is already set or add it
findstr /I "GNU_INSTALL_ROOT" Makefile >nul 2>&1
if errorlevel 1 (
    echo GNU_INSTALL_ROOT ?= "%GNU_INSTALL_ROOT%" >> Makefile
)

echo ==========================================
echo  Building Chameleon Ultra v2.3...
echo ==========================================

REM Build application firmware
cd ../application
make application 2>> build/app_build.log

if errorlevel 1 (
    echo [ERROR] Application build failed, checking log...
    type build\app_build.log | more
    goto :eof
)

echo [OK] Application firmware built successfully

REM Build bootloader firmware  
cd ../bootloader
make bootloader 2>> build/boot_build.log

if errorlevel 1 (
    echo [ERROR] Bootloader build failed, checking log...
    type build\boot_build.log | more
    goto :eof
)

echo [OK] Bootloader firmware built successfully

REM Generate DFU package
nrfutil nrf5sdk-tools pkg generate `
    --hw-version 0 `
    --sd-req 0x0100 ^
    --sd-id 0x0100 ^
    --application-version 1 ^
    --bootloader-version 1 ^
    --softdevice firmware\nrf52_sdk\components\softdevice\s140\hex\s140_nrf52_7.2.0_softdevice.hex `
    --bootloader firmware\objects\bootloader.hex ^
    --application firmware\objects\application.hex ^
    --key-file resource\dfu_key\chameleon.pem `
    build\ultra-dfu-v2.3-EM410X.zip 2>> build/pkg_gen.log

if errorlevel 1 (
    echo [ERROR] DFU package generation failed
    type build\pkg_gen.log | more
    goto :eof
)

echo ==========================================
echo  Build Complete!
echo ==========================================
echo.
echo Output files:
dir /b build\*.zip

echo.
echo Verification:
python verify_dfu.py

pause
