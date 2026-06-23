# Update nrf_app.mk with new feature source files
$firmwarePath = "C:\github\ChameleonUltra\firmware"
$newImplPath = "C:\Users\mirot\.openclaw\workspace\ChameleonUltra\new_implementation"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Updating Makefile with New Source Files" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Define the new source file entries to add
$newSRCS = @'
# === HF Extended Features (ISO14443-4/Type-4 Secure Cards) ===
$(APPDIR)/hf_extended/mifare_desfire_ev1_data.c
$(APPDIR)/hf_extended/mifare_desfire_ev2_data.c

# === LF ASK Emulation Files (Talk-to-First Protocol) ===
$(APPDIR)/lowfreq/t5577.c                    # Hitag S variant T5577 - industrial access control
$(APPDIR)/lowfreq/em4305.c                   # EM4305 with 32-bit basic ID (Hitag M)
$(APPDIR)/lowfreq/em4305_64.c                # EM4305 with 64-bit extended ID + sector info  
$(APPDIR)/lowfreq/fdx_b.c                    # FDX-B pseudo-random UID format

# === LF PSK Emulation Files (Reader-to-First Protocol) ===
$(APPDIR)/lowfreq/indala_psk.c               # Indala 2-sector/3-sector PSK industrial cards
$(APPDIR)/lowfreq/keri_psk.c                 # Keri two-phase protocol security cards

# === LF FSK Emulation Files (Read-to-First Protocol) ===
$(APPDIR)/lowfreq/paradox_fsk.c              # Paradox P1/P3 FSK security system cards
'@

# Read existing Makefile
$makefilePath = Join-Path $firmwarePath "application/src/rfid/nrf_app.mk"
$content = Get-Content -Path $makefilePath -Raw -ErrorAction SilentlyContinue

if ($null -eq $content) {
    Write-Host "ERROR: Could not find nrf_app.mk at: $makefilePath" -ForegroundColor Red
} else {
    # Find a suitable insertion point (before the last SRCS line or end of file)
    $lines = $content.Split([Environment]::NewLine)
    $newContent = @()
    
    foreach ($line in $lines) {
        $newContent += $line
        
        # Insert new source files after existing SRCS lines but before comments like "=== Build end ==="
        if ($line -like "*#.*===*build*" -or $line.Trim() -eq "") {
            # Found a section break, insert our new sources before it
            $insertPos = $newContent.Count - 1
            $indentation = if ($lastLine = $newContent[$insertPos]) { [string]($lastLine.Substring(0,$lastLine.Length) -replace '^[^\S\n]+', '') } else { "" }
            
            # Create indented new sources block
            $sourcesBlock = @()
            foreach ($src in $newSRCS.Split([Environment]::NewLine)) {
                if ($src.Trim()) {
                    $sourcesBlock += "    $src"  # 4-space indentation
                }
            }
            
            # Add sources block with blank line before it
            $newContent.Insert($insertPos + 1, "`n") | Out-Null
            $newContent.Insert($insertPos + 2, "# === NEW: Missing Features Implementation v2.2 ===`n") | Out-Null
            $index = ($newContent.Count - 2) - 1
            foreach ($src in $sourcesBlock) {
                $newContent.Insert($index + 1, $src) | Out-Null
                $index++
            }
            
            break
        }
    }
    
    # Write updated content back to file
    Set-Content -Path $makefilePath -Value $newContent -Encoding UTF8
    
    Write-Host "  Added new feature source files to nrf_app.mk!" -ForegroundColor Green
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Makefile Updated Successfully!" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
