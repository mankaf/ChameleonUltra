# ChameleonUltra Features Implementation Plan

## Goals
- Add CLI support for FDX-B, Indala, Keri, Paradox
- EM4305 alias support (em4305 read / em 4305 read)
- T5577/T55xx write group visibility
- DESFire EV1/EV2 classification in `hf des info`

## Status
✅ Firmware handlers exist: FDX-B, Indala, Keri, Paradox (all in tag_emulation.c/tag_base_type.h)  
✅ Python CLI command handlers exist in chameleon_cmd.py for PAC, Viking, Jablotron, EM4x05 as templates  
❌ Command enum entries missing for new protocols  
❌ CLI group registrations missing for new protocols  
❌ DESFire EV1/EV2 classification enhancement needed  

## Implementation Steps
1. Add command IDs to chameleon_enum.py  
2. Add CLI group definitions and read commands to chameleon_cli_unit.py  
3. Enhance hf des info output for EV1/EV2 detection  
4. Build firmware with all features  
5. Test end-to-end
