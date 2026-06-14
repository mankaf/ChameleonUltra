# Patch Makefile to add new feature source files
$makefilePath = "C:\github\ChameleonUltra\firmware\application\Makefile"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Adding New Feature Source Files to Makefile" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Read original content
$content = Get-Content $makefilePath -Raw
$lines = $content.Split([Environment]::NewLine)
$newLines = @()

foreach ($line in $lines) {
    $newLines += $line
    
    # Find the line with wiegand.c and add new source files after it
    if ($line -like "*lf/protocols/wiegand.c*") {
        # Add comment section for new features
        $newLines += ""  # blank line
        $newLines += "# === NEW: Missing Features Implementation v2.2 ==="
        $newLines += "# HF Extended Features (ISO14443-4/Type-4 Secure Cards)"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/hf_extended/mifare_desfire_ev1_data.c"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/hf_extended/mifare_desfire_ev2_data.c"
        $newLines += ""  # blank line
        $newLines += "# LF Extended Card Types (all protocols: ASK/PSK/FSK)"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/lowfreq/t5577.c                    # Hitag S variant T5577 - industrial access control"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/lowfreq/em4305.c                   # EM4305 with 32-bit basic ID (Hitag M)"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/lowfreq/fdx_b.c                    # FDX-B pseudo-random UID format"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/lowfreq/indala_psk.c               # Indala 2-sector/3-sector PSK industrial cards"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/lowfreq/keri_psk.c                 # Keri two-phase protocol security cards"
        $newLines += "$($(PROJ_DIR)/rfid/nfctag/lowfreq/paradox_fsk.c              # Paradox P1/P3 FSK security system cards"
    }
}

# Write modified content back
Set-Content -Path $makefilePath -Value ($newLines -join [Environment]::NewLine) -Encoding UTF8

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Makefile Updated Successfully!" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
