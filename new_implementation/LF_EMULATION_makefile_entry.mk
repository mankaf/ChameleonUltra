# LF Emulation Files - All Low-Frequency Card Types (125 kHz)
# Add these lines to your nrf_app.mk or equivalent Makefile

SRCS += $(APPDIR)/lowfreq/t5577.c              # ASK industrial cards
SRCS += $(APPDIR)/lowfreq/em4305.c             # ASK Hitag variants (32/64-bit)
SRCS += $(APPDIR)/lowfreq/fdx_b.c              # ASK legacy cards
SRCS += $(APPDIR)/lowfreq/indala_psk.c         # PSK industrial system cards
SRCS += $(APPDIR)/lowfreq/keri_psk.c           # PSK security cards
SRCS += $(APPDIR)/lowfreq/paradox_fsk.c        # FSK security system cards
