# Chameleon Ultra - Build Instructions

*Last updated: June 14, 2026*  
*Status: All features marked NOT IMPLEMENTED*

---

## Prerequisites

### System Requirements
- Windows/Linux/macOS development environment
- Python 3.10+ for Python-based components
- GCC compiler (for C code)
- OpenSSL development libraries (`libssl-dev` on Linux, `OpenSSL dev` on Windows)

### Python Dependencies
```bash
# Install required packages
pip install -r requirements.txt
```

Requirements include:
- python-dotenv >= 1.0.0
- pytest >= 7.0.0
- cryptography >= 41.0.0 (for crypto operations)
- pynacl >= 1.5.0 (for secure communications)

---

## Quick Start Build

```bash
cd C:\Users\mirot\.openclaw\workspace\ChameleonUltra

# Compile all stub implementations
make all

# View build status
ls bin/
```

**Expected output:** Stub implementation binaries ready for incremental development.

---

## Feature-Specific Build Targets

### Build Relay Attack Framework Only
```bash
cd firmware/relay_attacks
gcc -c relay_api.h relay_impl.c -I.. -o ../bin/relay.o
```

### Build Fingerprinting Module Only
```bash
cd firmware/fingerprinting_mgr
gcc -c fingerprint_api.h fingerprint_impl.c -I../include -o ../bin/fingerprint.o
```

### Build Complete Test Suite
```bash
make test-all
./bin/firmware_tests
```

---

## Development Workflow

### 1. Initialize Framework
The stub implementations provide:
- Compile-ready source code
- Proper error handling patterns  
- Interface contracts (function signatures)
- Ready-to-use data structures

### 2. Implement Feature Logic
Each feature has TODO comments marking missing functionality:
```c
// Example from relay_impl.c:
// TODO: Forward packet between connected sockets
// send(fd_connect, data, len, MSG_NOSIGNAL);
```

### 3. Integrate with Hardware
Features requiring hardware have placeholder functions:
- `hal_get_device_descriptor()` - device enumeration stub
- Network stack hooks ready for NIC binding
- DMA handler interface prepared for PCI access

---

## Build System Details

### Makefile Structure
```makefile
SRC_DIR = src                # Source files location
OBJ_DIR = obj                # Object files location  
BIN_DIR = bin                # Binary output directory
TARGET = $(BIN_DIR)/chameleon_firmware  # Main firmware binary
```

### Feature Flags (All Disabled)
The build system uses feature flags to enable/disable components:
```makefile
ENABLE_RELAY_ATTACKS = 1      # NOT IMPLEMENTED
ENABLE_DECORPTION_ENGINE = 0   # NOT IMPLEMENTED  
ENABLE_FINGERPRINTING = 0      # NOT IMPLEMENTED
ENABLE_SIDE_CHANNEL = 0        # NOT IMPLEMENTED
ENABLE_HW_EXPLOITS = 0        # NOT IMPLEMENTED
```

---

## Testing

### Unit Tests Available
Only features with stub tests can be run:

```bash
make test-all
./bin/firmware_tests
```

Expected test coverage:
- ✅ Relay attack framework structure tests (stub only)
- ✅ Decryption engine interface tests (stub only)
- ✅ Fingerprinting extraction tests (stub only)

Missing:
- ❌ Side-channel hardware integration tests
- ❌ Hardware exploit driver tests
- ❌ Full integration tests with actual devices

---

## Debugging

### Enable Verbose Output
```bash
make clean all CFLAGS="-Wall -Wextra -g -O0 -DVERBOSE"
```

### Check Build Warnings
```bash
make all 2>&1 | grep -E "(warning|error)"
```

### Run with Valgrind (Linux only)
```bash
valgrind --leak-check=full ./bin/firmware_tests
```

---

## Known Limitations (All Features NOT IMPLEMENTED)

### Build System Limitations
1. ⚠️ No hardware detection during build - requires manual setup
2. ⚠️ OpenSSL integration stubbed, not linked against actual library
3. ⚠️ No cross-compilation support configured

### Feature Implementation Status
4. ⚠️ All features marked NOT IMPLEMENTED - no functional code
5. ⚠️ Stub implementations compile but provide NO functionality
6. ⚠️ Test suites are stubs - don't test actual behavior

### Hardware Requirements (for Full Implementation)
7. Oscilloscope for side-channel measurements
8. PCIe passthrough or Thunderbolt for DMA access
9. NIC with firmware update capability
10. USB controller for HID injection testing

---

## Incremental Development Guide

### Phase 1: Pure Software Features
Start here - no special hardware required:
1. Implement relay packet forwarding (uses only sockets)
2. Add TLS session ticket passthrough logic
3. Create protocol parsers for decryption engine

### Phase 2: Common Hardware Interfaces
Requires standard PC hardware:
1. USB HID descriptor parsing
2. SMBIOS table reading  
3. Network stack extension with actual NIC

### Phase 3: Specialized Hardware
Requires oscilloscope/probe equipment:
1. Oscilloscope driver for power analysis
2. Near-field probe capture interface
3. Thermal sensor integration

---

## Troubleshooting

### Build Errors

**Error:** "undefined reference to OpenSSL"
```bash
# Fix: Link against crypto library
LDFLAGS += -lcrypto -lssl
make clean all
```

**Error:** "fatal error: windows.h: No such file"
```bash
# Already a Windows build (not cross-compiling)
# If targeting Linux, install gcc-multilib or use mingw-w64
```

### Runtime Errors

**Symptom:** Stub implementations return -1 from all functions
**Cause:** Expected behavior - stubs mark features as NOT IMPLEMENTED  
**Solution:** Read TODO comments to identify implementation gaps

---

## Next Steps After Building

Once stub implementations are built:

1. **Add Real Logic** - Replace TODO comments with actual code
2. **Link Dependencies** - Add OpenSSL, libusb, etc. to LDFLAGS
3. **Update Feature Flags** - Enable features as they're implemented
4. **Write Integration Tests** - Test against actual hardware
5. **Document API Usage** - Complete documentation for developers

---

## Code Style Guidelines (From PROJECT.md)

- Python version: 3.10+
- Formatting: black + isort (Python files only)
- Type hints: Required for all C functions  
- Logging: Structured logging with levels
- TODO comments: Mark all incomplete features

---

*Build system ready - all features marked NOT IMPLEMENTED as per specification*
