# Chameleon Ultra - Missing Features Implementation Plan

Based on the [Technical Whitepaper](https://github.com/RfidResearchGroup/ChameleonUltra/wiki/technical_whitepaper#low-frequency-emulation), here are all features marked "Not yet implemented" and their implementation approach.

---

## 📋 Feature List (18 total)

### **High-Frequency Features (ISO14443-A)**
| # | Feature | Type | Status | Implementation Notes |
|---|---------|------|--------|---------------------|
| 1 | **MIFARE Ultralight C** | HF, ASK, Talk-to-First | ❌ Not Yet Implemented | ISO14443A/106 kbit/s - Needs ULC protocol parser + crypto |
| 2 | **MIFARE DESFire EV1** | HF, High Rate (Low rate supported) | ❌ Not Yet Implemented | ISO14443-4 integration + secure channel establishment |
| 3 | **MIFARE DESFire EV2** | HF, High Rate (Low rate supported) | ❌ Not Yet Implemented | Same as EV1 but with newer cipher suites |
| 4 | **MIFARE Plus** | HF, High Rate | ❌ Not Yet Implemented | SLE4442-based - needs multi-card emulation support |

### **Attack Capabilities**
| # | Feature | Type | Status | Implementation Notes |
|---|---------|------|--------|---------------------|
| 5 | **Relay Attack** | HF (ISO14443A cards) | ❌ Not Yet Implemented | Multi-antenna sync + time-stamping for relay detection bypass |

### **Low-Frequency Card Emulation** (LF, 125 kHz)

#### ASK Modulation (Talk-to-First)
| # | Card Type | Code Name | Status | Notes |
|---|-----------|-----------|--------|-------|
| 6 | T5577 | "Hitag S-like" | ❌ Not Yet Implemented | Common in low-power locks |
| 7 | EM4305/EM4369 | "Hitag M/X" | ❌ Not Yet Implemented | Extended Hitag variants with write support |
| 8 | FDX-B | "FDX-B Electra" | ❌ Not Yet Implemented | EM-Micro cards, often found in older locks |

#### PSK Modulation (Reader-to-First)  
| # | Card Type | Code Name | Status | Notes |
|---|-----------|-----------|--------|-------|
| 9 | Indala | "Indala 20" | ❌ Not Yet Implemented | Common in industrial access control |
| 10 | Keri | "Keri V1/V2" | ❌ Not Yet Implemented | PSK protocol (different timing) |
| 11 | NexWatch | "NexWatch Pro" | ❌ Not Yet Implemented | Already has type enum but no implementation |

#### FSK Modulation (Read-to-First)
| # | Card Type | Code Name | Status | Notes |
|---|-----------|-----------|--------|-------|
| 12 | Paradox | "Paradox P1/P3" | ❌ Not Yet Implemented | Used in security systems |
| 13 | AWD | "AWD-6801/6803" | ❌ Not Yet Implemented | Access Control World cards |
| 14 | securakey | "securakey Prox" | ❌ Not Yet Implemented | EM-Micro, often with PIN code |
| 15 | gallagher | "Gallagher 606/607" | ❌ Not Yet Implemented | UK security systems |
| 16 | PAC/Stanley | "PAC/Stanley Prox" | ❌ Not Yet Implemented | Industrial access control |
| 17 | Presco | "Presco Card" | ❌ Not Yet Implemented | Simple 68-bit ID format |
| 18 | Visa2000 | "Visa-1000/2000" | ❌ Not Yet Implemented | Banking access control cards |
| 19 | Viking | "Viking Card" | ✅ **Already Supported** | Already implemented in codebase |
| 20 | Noralsy | "Noralsy Prox" | ❌ Not Yet Implemented | Industrial use case |

---

## 🏗️ Implementation Architecture

### **File Structure**

```
ChameleonUltra/
├── firmware/                          # Original source (GitHub)
├── new_implementation/                # NEW: All missing features
│   ├── lowfreq/
│   │   ├── t5577.c                   # ASK protocol + data format converter
│   │   ├── em4305.c                  # Hitag variants with write support
│   │   ├── fdx_b.c                   # FSK protocol implementation
│   │   ├── indala_psk.c              # PSK modulation handling
│   │   ├── keri_psk.c                # Keri PSK variant
│   │   ├── paradox_fsk.c             # FSK with 16-bit UID format
│   │   ├── awd_fsk.c                 # AWD card support
│   │   ├── securakey_ask.c           # ASK with PIN code handling
│   │   ├── gallagher_ask.c           | Simple 68-bit ID format
│   │   ├── pac_stanley_ask.c         | Industrial access control
│   │   ├── presco_ask.c              | Banking/financial sector
│   │   ├── visacard_ask.c            | Access control system cards
│   │   └── noralsy_ask.c             # Industrial applications
│   │
│   ├── hf_extended/
│   │   ├── ultralight_c.c            # ULC protocol + crypto1
│   │   ├── desfire_ev1.c             # Low-rate emulation
│   │   ├── desfire_ev2.c             | ISO14443-4 integration
│   │   └── mifare_plus.c             # Multi-card emulation
│   │
│   └── relay_attack/
│       ├── relay_core.c              # Challenge-response amplification
│       ├── antenna_sync.c            # Multi-antenna timing sync
│       └── timestamping.c            # Relay detection bypass
│
└── patches/                           # Required source modifications
    ├── tag_base_type.patch
    └── tag_emulation.patch
```

---

## 📝 Implementation Order (Recommended)

### **Phase 1: High-Priority Production Features**
1. **MIFARE DESFire EV1/EV2** - Most commercially valuable, backward compatible
2. **MIFARE Plus** - Enterprise deployments
3. **Relay Attack** - Critical for full attack vector coverage

### **Phase 2: Common LF Cards**
4. **T5577** - Widely used in low-power locks
5. **EM4305/Hitag variants** - Access control systems
6. **FDX-B/Electra** - Legacy industrial cards

### **Phase 3: Industrial/Niche Cards**
7-19. Industrial cards (Indala, Keri, Paradox, etc.) - Deployed in specific sectors

---

## 🔧 Implementation Guidelines

### **General Rules:**

1. **Use existing architecture patterns:**
   - LF cards: Extend `lf_tag_em.c` handler pattern
   - HF cards: Extend `nfc_mf1.c` data handler pattern
   - All need: `loadcb_t`, `savecb_t`, `factory_t` functions

2. **Memory layout conversion is key:**
   - Some cards use proprietary formats (not raw bytes)
   - Must implement byte↔proprietary converters
   - Often requires sector-based data structures

3. **Timing adjustments for low-power locks:**
   - Add initial delay before field activation (5-10ms)
   - Extend response window for battery-powered readers
   - Reduce transmission rate when needed

4. **PWM clock configuration:**
   - ASK/FSK: Keep 125kHz base frequency
   - PSK: Switch to 1MHz base clock for PWM subcarrier alignment

### **Code Template (LF ASK Card):**

```c
// In lf_tag_em.h, add handler functions:
typedef int (*lf_ask_data_loadcb_t)(tag_specific_type_t type, tag_data_buffer_t *buffer);
typedef int (*lf_ask_data_savecb_t)(tag_specific_type_t type, tag_data_buffer_t *buffer);
typedef bool (*lf_ask_data_factory_t)(uint8_t slot, tag_specific_type_t type);

// In lf_tag_em.c:
int lf_t5577_data_loadcb(tag_specific_type_t type, tag_data_buffer_t *buffer) {
    // Read from flash using fds_read_sync
    // Convert T5577 proprietary format to raw bytes
    return length;
}

bool lf_t5577_data_factory(uint8_t slot, tag_specific_type_t type) {
    // Create factory data for new card
    // Format: [4-bit UID][data bits]
    return true;
}
```

---

## 📖 Protocol Reference Documents

### **Need these specifications to implement each card:**

| Card | Data Length | UID Format | Special Features |
|------|-------------|------------|------------------|
| T5577 | 10-68 bits | 4-bit base + data | Write-only mode, variable bit length |
| EM4305 | 32/64 bits | Variable (32/64) | Multi-length support |
| FDX-B Electra | 20-68 bits | Pseudo-random | Data bits only, no ATQA |
| Indala 20 | 192 bits (3 sectors) | Fixed length | Sector-based data |
| Keri V1/V2 | 56+8 or 48+8 bits | 56-bit UID | PSK timing variations |
| NexWatch Prox | 72+16 bits | Variable length | Long/short forms |
| Paradox P1/P3 | 16-32 bits | 16-bit base | Security system cards |
| AWD-6801 | 68 bits | Fixed 68-bit ID | Simple data format |
| securakey | 64+16 bits | Variable + PIN | Optional authentication code |
| Gallagher | 72 bits (3×24) | Sector-based | UK security systems |
| PAC/Stanley | 50-68 bits | Variable length | Industrial use cases |
| Presco Card | 68 bits | Fixed length | Simple sequential format |
| Visa2000 | 72 bits (3×24) | Sector-based | Banking access control |
| Viking Card | 10-68 bits | Pseudo-random | Already implemented as example |
| Noralsy | 32-68 bits | Variable length | Industrial applications |

---

## 📋 Required Source Modifications

### **tag_base_type.h** additions:
```c
// Add to tag_specific_type_t enum (LF section):
TAG_TYPE_T5577 = 120,
TAG_TYPE_EM4305 = 107,
TAG_TYPE_FD_X_B = 105,
TAG_TYPE_INDALA = 305,
TAG_TYPE_KERI = 307,
TAG_TYPE_NEXWATCH = 310,
// HF additions:
TAG_TYPE_MIFARE_DESFIRE_EV1 = 1305,
TAG_TYPE_MIFARE_DESFIRE_EV2 = 1310,
```

### **tag_emulation.h** additions:
```c
// Extend tag_base_map_t to include new card types:
{TAG_SENSE_LF, TAG_TYPE_T5577,       lf_t5577_data_loadcb,      ...},
{TAG_SENSE_LF, TAG_TYPE_EM4305,      lf_em4305_data_loadcb,      ...},
// HF additions:
{TAG_SENSE_HF, TAG_TYPE_MIFARE_DESFIRE_EV1,  hf_desfire_ev1_loadcb,    ...},
```

---

## 🚀 Next Steps

**Ready to start implementation?** I'll create all the code files for each missing feature. Which do you want first:

1. **MIFARE DESFire EV1/EV2** (HF, high-value, production-critical)
2. **Relay Attack** (Attack capabilities)
3. **T5577/EM4305** (Most common LF industrial cards)
4. **All of them together**

Let me know and I'll start generating the implementation files! 🚀
