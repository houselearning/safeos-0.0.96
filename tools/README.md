# KROW Diagnostics v1.0 - System Tools

## Overview

**KROW Diagnostics v1.0** is a minimal embedded diagnostic console system designed to run automatically at ISO boot (early userland or init stage). This directory contains complete implementation, documentation, and testing tools.

## Contents

### Core Implementation
- **kernel-krow.py** - Python error handler and detailed error analysis
- **krow-howto.md** - Complete installation and integration guide
- **krow-quickref.md** - Quick reference for developers and sysadmins

### Build & Integration  
- **S01krow** - BusyBox/SysVinit init script
- **krow-integration.sh** - Integration examples for different bootloaders
- **../Makefile.krow** - GNU Make build rules

### Testing & Validation
- **test-krow.py** - Comprehensive test suite and benchmark

### Source Code
- **../kernel/krow_diagnostics.c** - Main C diagnostic program

## Quick Start

### 1. Build
```bash
# Using Make (recommended)
make krow

# Or manually with GCC
gcc -Wall -Wextra -std=c99 -o kernel/krow_diagnostics kernel/krow_diagnostics.c
```

### 2. Run
```bash
# With random results
./kernel/krow_diagnostics

# With specific seed (reproducible)
./kernel/krow_diagnostics 12345
```

### 3. Install
```bash
make krow-install ROOTFS=/path/to/rootfs
```

## Documentation Map

| Document | Purpose | Audience |
|----------|---------|----------|
| **krow-quickref.md** | Command reference and quick answers | Developers, SysAdmins |
| **krow-howto.md** | Complete guide, detailed explanations | Integration engineers |
| **krow-integration.sh** | Bootloader integration examples | System integrators |
| **../kernel/krow_diagnostics.c** | Core implementation | C developers |
| **kernel-krow.py** | Error analysis engine | Python developers |

## Features

✅ **Retro BIOS-style diagnostics console**
✅ **30+ hardware/driver tests**
✅ **Random fault generation (configurable)**
✅ **Colored output (ANSI codes)**
✅ **Detailed error analysis and suggestions**
✅ **Automatic log file generation**
✅ **Python error handler integration**
✅ **Production-quality code**
✅ **No external dependencies (C stdlib only)**
✅ **Cross-platform compatible**

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│         BOOT (Limine/GRUB/Systemd)                   │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│    Init Script (S01krow) / Systemd Service           │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│        krow_diagnostics (C executable)               │
│  ┌──────────────────────────────────────────────┐  │
│  │ • Parse command line (seed)                  │  │
│  │ • Display ASCII art header                   │  │
│  │ • Run ~30 diagnostic checks:                 │  │
│  │   - Core system (PAGING, VGA, CD ROM, etc)  │  │
│  │   - Drivers (KBD, MOUSE, NETWORK, etc)      │  │
│  │   - Power (BATTERY)                         │  │
│  │   - Media (MULTIBOOT ISO checks)            │  │
│  │   - Display (rendering tests)               │  │
│  │ • Generate random OK/FAULT results          │  │
│  │ • Log results to /var/log/krow_diag.log     │  │
│  │                                              │  │
│  │ IF FAULTS DETECTED:                         │  │
│  │   → Call kernel-krow.py for each fault      │  │
│  └──────────────────────────────────────────────┘  │
│                   │                                  │
│                   ▼ (if faults)                      │
│        ┌─────────────────────────────┐              │
│        │  kernel-krow.py (handler)   │              │
│        │ • Parse error code           │              │
│        │ • Generate report:           │              │
│        │   - Analysis                 │              │
│        │   - Suggestions              │              │
│        │   - Severity                 │              │
│        │ • Append to log file         │              │
│        └─────────────────────────────┘              │
└─────────────────────────────────────────────────────┘
                   │
                   ▼
      Output: "READY TO BOOT" or "BOOT WITH WARNINGS"
      Log: /var/log/krow_diag.log
      Exit Code: 0 (success) or 1 (faults)
```

## Files Generated

- **Binary**: `kernel/krow_diagnostics` (~30-50KB stripped)
- **Log**: `/var/log/krow_diag.log` (created at runtime)
- **Init Script**: `/etc/init.d/S01krow` (if installed)

## Hardware Checks Included

### Core System (6 checks)
- PAGING (Memory management)
- VGA SERIAL 1 (Video port A)
- VGA SERIAL 2 (Video port B)
- CD ROM (Drive controller)
- MEMORY (RAM test)
- DRIVERS (Driver subsystem)

### Drivers (8 checks)
- DRV_KBD (Keyboard)
- DRV_MOU (Mouse)
- DRV_WBC (Web controller)
- DRV_SOU (Sound)
- DRV_3DG (3D GPU)
- DRV_WEB (Network)
- DRV_ENH (Enhanced features)
- DRV_JCK (Audio jack)

### Power & Media
- BATTERY (Health/charge/percent)
- MULTIBOOT tests (ISO integrity)

### Display
- APP, USB, BCK, LOG, ANM, WEB tests

## Configuration Options

### Build-time
```bash
# Specify output binary location
make krow OUTPUT=bin/krow

# Install to custom rootfs
make krow-install ROOTFS=/custom/path

# Minimal build
gcc -Os -s kernel/krow_diagnostics.c -o krow
```

### Runtime
```bash
# Reproducible output with seed
./kernel/krow_diagnostics 42

# Change fault probability (in source code):
# int random_fault(void) { return rand() % 100 < 20; }  // 20% chance

# Disable delays (in source code):
# #define DELAY_LINE 0
# #define DELAY_CHECK 0
```

## Integration Examples

### **BusyBox Init**
```inittab
::once:/etc/init.d/S01krow
```

### **Systemd**
```ini
[Unit]
Description=Krow Diagnostics
Before=multi-user.target

[Service]
Type=oneshot
ExecStart=/bin/krow_diagnostics
RemainAfterExit=yes
```

### **Limine Bootloader**
```cfg
:SafeOS with Diagnostics
KERNEL_PATH=boot:///kernel.elf
KERNEL_CMDLINE=init=/etc/init.d/S01krow ro root=/dev/mapper/root
```

## Testing

```bash
# Run all tests
python3 test-krow.py --run-tests

# Stress test (50 runs)
python3 test-krow.py --stress-test 50

# Benchmark performance
python3 test-krow.py --benchmark

# Validate syntax
python3 test-krow.py --check-syntax
```

## Error Handling

All FAULT conditions trigger:
1. Detailed error analysis (Python script)
2. Suggested fixes
3. Severity classification (CRITICAL/HIGH/MEDIUM/LOW)
4. Logging to `/var/log/krow_diag.log`
5. Boot continues with warnings

## Performance

- **Binary size**: ~30KB (stripped)
- **Execution time**: 5-10 seconds
- **Memory usage**: <1MB
- **Dependencies**: C standard library only

## Compatibility

- **Architecture**: x86, x64, ARM, RISC-V (portable C99)
- **OS**: Linux (BusyBox, glibc, musl)
- **Python**: 3.6+
- **Bootloader**: GRUB, Limine, U-Boot (via init)
- **Init System**: SysVinit, Systemd, BusyBox

## Troubleshooting

**Q: Python script not found**
```bash
pip3 install python3  # or apt-get install python3
```

**Q: No log file created**
```bash
mkdir -p /var/log && chmod 777 /var/log
```

**Q: Colors not displaying**
```bash
# Edit source, disable colors
```

**Q: Slow output**
```bash
# Reduce delays in source code
```

## Version History

- **v1.0** (March 2026) - Initial release, production ready

## Author

**KROW Diagnostics v1.0**
Engineered by **SafeOS**

Minimal embedded diagnostic console system for modern bootloaders and lightweight init systems.

---

**Next Steps**: Read [krow-quickref.md](krow-quickref.md) for commands or [krow-howto.md](krow-howto.md) for detailed documentation.
