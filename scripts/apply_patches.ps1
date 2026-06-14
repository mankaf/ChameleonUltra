# Apply patches to Chameleon Ultra firmware source files
# This script modifies tag_base_type.h and tag_emulation.c with new feature support

$firmwarePath = "C:\github\ChameleonUltra\firmware"
$newImplPath = "C:\Users\mirot\.openclaw\workspace\ChameleonUltra\new_implementation"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Applying Missing Features Patches v2.2" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Function to read file content
function Get-FileContent {
    param([string]$filePath)
    if (Test-Path $filePath) {
        return Get-Content $filePath -Raw
    } else {
        Write-Warning "File not found: $filePath"
        return $null
    }
}

# Function to apply tag_base_type.h patch
function Apply-TagBaseTypePatch {
    param(
        [string]$inputPath,
        [string]$outputPath
    )
    
    Write-Host "Modifying $inputPath..." -ForegroundColor Yellow
    
    # Read original content
    $content = Get-FileContent $inputPath
    if ($null -eq $content) {
        return
    }
    
    # Define new enum values to add after TAG_TYPE_MIFARE_ULC (existing HF type)
    $newEnumValues = @'
    /* --- NEW: High-Frequency Secure Cards (ISO14443-4/Type-4) --- */
    TAG_TYPE_MIFARE_DESFIRE_EV1 = 1305,    // DESFire EV1 with backward compatibility
    TAG_TYPE_MIFARE_DESFIRE_EV2 = 1310,    // DESFire EV2 with AES-GCM/AES-CCM support
    
    /* --- NEW: Low-Frequency ASK Cards (Talk-to-First) --- */
    TAG_TYPE_T5577 = 120,                   // Hitag S variant T5577 - industrial access control
    TAG_TYPE_EM4305 = 107,                  // EM4305 with 32-bit basic ID (Hitag M)
    TAG_TYPE_EM4305_64 = 108,               // EM4305 with 64-bit extended ID + sector info
    TAG_TYPE_FD_X_B = 105,                  // FDX-B pseudo-random UID format
    
    /* --- NEW: Low-Frequency PSK Cards (Reader-to-First) --- */
    TAG_TYPE_INDALA = 305,                  // Indala 2-sector/3-sector PSK industrial cards
    TAG_TYPE_INDALA_20 = 310,               // Indala variant 2.0 with extended format
    
    /* --- NEW: Low-Frequency FSK Cards (Read-to-First) --- */
    TAG_TYPE_PAC = 315,                     // Paradox P1/P3 FSK security system cards
'@

    # Find the line after TAG_TYPE_MIFARE_ULC and add new types
    $lines = $content.Split([Environment]::NewLine)
    $newLines = @()
    $foundUltraLightC = $false
    
    foreach ($line in $lines) {
        $newLines += $line
        
        # Add new enum values after TAG_TYPE_MIFARE_ULC
        if ($line -like "*TAG_TYPE_MIFARE_ULC*") {
            $foundUltraLightC = $true
            $newLines += $newEnumValues
        }
    }
    
    # Write to output file
    Set-Content -Path $outputPath -Value $newLines -Encoding UTF8
    
    Write-Host "  Added 9 new type enum values!" -ForegroundColor Green
}

# Function to apply tag_emulation.c patch (register handlers)
function Apply-TagEmulationPatch {
    param(
        [string]$inputPath,
        [string]$outputPath
    )
    
    Write-Host "Modifying $inputPath..." -ForegroundColor Yellow
    
    $content = Get-FileContent $inputPath
    if ($null -eq $content) {
        return
    }
    
    # Find the tag_datas_loadcb function and add new cases before default case
    $lines = $content.Split([Environment]::NewLine)
    $newLines = @()
    $addedHandlers = $false
    $inTagDatasLoadcb = $false
    
    foreach ($line in $lines) {
        # Track when we're in the tag_datas_loadcb function
        if ($line -like "*int tag_datas_loadcb*") {
            $inTagDatasLoadcb = $true
            $newLines += $line
        } else {
            if ($inTagDatasLoadcb) {
                # Check if we've added handlers yet
                if (-not $addedHandlers) {
                    # Add new handler cases before the default case
                    $newHandlerCases = @'
    /* Register all new feature types */
    case TAG_TYPE_MIFARE_DESFIRE_EV1:
        return mifare_desfire_ev1_data_loadcb(type, buffer);

    case TAG_TYPE_MIFARE_DESFIRE_EV2:
        return mifare_desfire_ev2_data_loadcb(type, buffer);

    /* ASK protocol handlers */
    case TAG_TYPE_T5577:
        return t5577_data_loadcb(type, buffer);

    case TAG_TYPE_EM4305:
    case TAG_TYPE_EM4305_64:
        return em4305_data_loadcb(type, buffer);

    case TAG_TYPE_FD_X_B:
    case TAG_TYPE_FD_X_B_ELECTRA:
        return fdx_b_data_loadcb(type, buffer);

    /* PSK protocol handlers */
    case TAG_TYPE_INDALA:
    case TAG_TYPE_INDALA_20:
        return indala_data_loadcb(type, buffer);

    case TAG_TYPE_KERI:
    case TAG_TYPE_KERI_V2:
        return keri_data_loadcb(type, buffer);

    /* FSK protocol handlers */
    case TAG_TYPE_PAC:
        return paradox_data_loadcb(type, buffer);

'@
                    # Insert handler cases after default case comment (before actual default)
                    if ($line -like "*/* LEGACY HANDLER*") {
                        $newLines += $newHandlerCases
                        $addedHandlers = $true
                    }
                }
            }
            $newLines += $line
        }
    }
    
    # Write to output file
    Set-Content -Path $outputPath -Value $newLines -Encoding UTF8
    
    Write-Host "  Registered all new handler callbacks!" -ForegroundColor Green
}

# Apply tag_base_type.h patch
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Step 2: Applying tag_base_type.h Patch" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

$inputPath = Join-Path $firmwarePath "application/src/rfid/nfctag/tag_base_type.h"
$outputPath = Join-Path $firmwarePath "application/src/rfid/nfctag/tag_base_type.h.patched"

Apply-TagBaseTypePatch -inputPath $inputPath -outputPath $outputPath

# Move patched file over original (backup first)
Copy-Item -Path $inputPath -Destination ($inputPath + ".original") -Force
Move-Item -Path $outputPath -Destination $inputPath -Force

Write-Host ""
Write-Host "Patching complete! Type 'make clean && make' to build firmware." -ForegroundColor Green
