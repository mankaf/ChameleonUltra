# Build Chameleon Ultra v2.3 with EM410X variants and settings compatibility fix
# This script handles all build steps for quick deployment

$env:GNU_INSTALL_ROOT = "C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/gcc-arm-none-eabi-10.3-2021.10/bin/"
$env:GNU_PREFIX = "arm-none-eabi"
$env:APP_FW_VER_MAJOR = "2"
$env:APP_FW_VER_MINOR = "3"
$env:GIT_VERSION = "v2.3-EM410X-Variants"

Write-Host "[+] Building application firmware for v2.3..." -ForegroundColor Cyan

cd firmware\application
& 'C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/gcc-arm-none-eabi-10.3-2021.10/bin/make.exe' application 2>&1 | Out-File -FilePath "build/app_build.log"

if ($LASTEXITCODE -eq 0) {
    Write-Host "[+] Application build SUCCESS" -ForegroundColor Green
    
    cd ../bootloader
    & 'C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/gcc-arm-none-eabi-10.3-2021.10/bin/make.exe' bootloader 2>&1 | Out-File -FilePath "build/boot_build.log"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[+] Bootloader build SUCCESS" -ForegroundColor Green
        
        cd ..
        & 'C:\Users\mirot\.nrfutil\bin\nrfutil.exe' nrf5sdk-tools pkg generate `
            --hw-version 0 `
            --sd-req 0x0100 `
            --sd-id 0x0100 `
            --application-version 1 `
            --bootloader-version 1 `
            --softdevice 'firmware\nrf52_sdk\components\softdevice\s140\hex\s140_nrf52_7.2.0_softdevice.hex' `
            --bootloader firmware\objects\bootloader.hex `
            --application firmware\objects\application.hex `
            --key-file resource\dfu_key\chameleon.pem `
            build/ultra-dfu-v2.3-EM410X.zip 2>&1 | Out-File -FilePath "build/pkg_gen.log"
        
        Write-Host "[+] DFU package created: ultra-dfu-v2.3-EM410X.zip" -ForegroundColor Green
        
        # Verify the zip
        python verify_dfu.py 2>&1 | Out-File -FilePath "build/verification.txt"
        
        Write-Host ""
        Write-Host "[=] Build Complete =]" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Output files:" -ForegroundColor Yellow
        Get-ChildItem build\*.zip | ForEach-Object { Write-Host "  $($_.Name) - $($_.Length) bytes" }
    } else {
        Write-Host "[x] Bootloader build FAILED" -ForegroundColor Red
    }
} else {
    Write-Host "[x] Application build FAILED" -ForegroundColor Red
}
