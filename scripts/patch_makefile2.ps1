# Patch Makefile to add new feature source files
$makefilePath = "C:\github\ChameleonUltra\firmware\application\Makefile"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Adding New Feature Source Files to Makefile" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Read original content  
$content = Get-Content $makefilePath -Raw -Encoding UTF8
$lines = $content.Split([Environment]::NewLine)
$newLines = @()

foreach ($line in $lines) {
    # Check for wiegand.c line and replace it with enhanced version including new sources
    if ($line -match "\*lf/protocols/wiegand\.c \\\") {
        # Keep original like but add our new source lines after
        $newLines += $line
        
        # Add comment section for new features  
        $newLines += ""  # blank line
        $newLines += "# === NEW: Missing Features Implementation v2.2 ==="
        $newLines += "# HF Extended Features (ISO14443-4/Type-4 Secure Cards)"
        $newLines += "$(PROJ_DIR)/rfid/nfctag/hf_extended/mifare_desfire_ev1_data.c"
        $newLines += "$(PROJ_DIR)/rfid/nfctag/hf_extended/mifare_desfire_ev2_data.c"
        
        $newLines += ""  # blank line
        $newLines += "# LF Extended Card Types (all protocols: ASK/PSK/FSK)"
        $newLines += "$(PROJ_DIR)/rfid/nfctag/lowfreq/t5577.c                    # Hitag S variant T5577"
        $newLines += "$(PROJ_DIR)/rfid/nfctag/lowfreq/em4305.c                   # EM4305 32-bit (Hitag M)"
        $newLines += "$(PROJ_DIR)/rfid/nfctag/lowfreq/fdx_b.c                    # FDX-B pseudo-random UID"  
        $newLines += "$(PROJ_DIR)/rfid/nfctag/lowfreq/indala_psk.c               # Indala PSK industrial cards"
        $newLines += "$(PROJ_DIR)/rfid/nfctag/lowfreq/keri_psk.c                 # Keri two-phase protocol"  
        $newLines += "$(PROJ_DIR)/rfid/nfctag/lowfreq/paradox_fsk.c              # Paradox P1/P3 FSK security cards"
    } else {
        $newLines += $line
    }
}

# Write modified content back with UTF-8 encoding
Set-Content -Path $makefilePath -Value ($newLines -join [Environment]::NewLine) -Encoding UTF8 -NoNewline

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan  
Write-Host "Makefile Updated Successfully!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Run: cd C:\github\ChameleonUltra\firmware" -ForegroundColor Gray  
Write-Host "2. Run: make clean && make" -ForegroundColor Gray
Write-Host "3. Create DFU: nrfutil dfu create ChameleonUltra_v2.2.dfu.zip --firmware-file firmware/app.hex --softdevice-file sd_s140_nano_6.x_x.s14 --product 0x637f --device 0x0042" -ForegroundColor Gray
