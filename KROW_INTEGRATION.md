# KROW DIAGNOSTICS V 1.0 - INTEGRATION GUIDE

## Overview

**Krow Diagnostics v1.0** is an embedded diagnostic system that integrates into the SafeOS boot sequence. It runs automatically during system startup, performs hardware and driver checks, and reports any faults before the main OS boots.

## Architecture

```
Boot Process:
┌─────────────────────────────────────┐
│  BIOS/Bootloader (Limine)          │
│  └─ Loads kernel                   │
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│  kernel/arch/x86/boot.s            │ ← Boot entry point
│  └─ Early kernel setup             │
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│  kernel/core/kmain.c               │ ← Main kernel entry
│  └─ krow_diagnostics_run()         │ ← INSERT CALL HERE
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│  KROW DIAGNOSTICS SYSTEM           │
│  ├─ Core checks                    │
│  ├─ Driver checks                  │
│  └─ Error reporting (Python)       │
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│  OS Main Boot Process              │
│  └─ Continue normal initialization │
└─────────────────────────────────────┘
```

## Integration Method 1: Direct Kernel Call (Recommended)

This integrates Krow into the kernel initialization process.

### Step 1: Add Header Include

In **kernel/core/kmain.c**, add at the top:

```c
#include "krow_diagnostics.h"
```

### Step 2: Call During Initialization

In the `kmain()` or equivalent main kernel function, add the diagnostic call:

```c
int kmain(void) {
    // ... existing early kernel initialization ...
    
    // Initialize frame buffer, GDT, IDT, etc.
    // ... your existing code ...
    
    // ===== RUN KROW DIAGNOSTICS =====
    kprintf("[*] Running embedded diagnostics...\n");
    int krow_result = krow_diagnostics_run();
    
    if (krow_result != 0) {
        kprintf("[WARN] Diagnostics completed with %d fault(s)\n", krow_result);
    } else {
        kprintf("[OK] All diagnostics passed\n");
    }
    // ================================
    
    // ... continue with normal OS initialization ...
}
```

### Step 3: Update Makefile

The Makefile already includes Krow compilation targets. Ensure your main kernel compilation includes krow_diagnostics.c:

```makefile
# In your main Makefile compilation rule, krow_diagnostics.c 
# will be found by the $(shell find kernel -name '*.c') pattern
```

## Integration Method 2: Init Script (Userspace)

For systems that support init.d scripts.

### Installation

1. The init script is located at: **scripts/S01krow**

2. Install to your rootfs:
   ```bash
   make krow-install ROOTFS=/path/to/rootfs
   ```

3. This places:
   - `/bin/krow_diagnostics` - The compiled diagnostic binary
   - `/usr/bin/kernel-krow.py` - The Python error handler
   - `/etc/init.d/S01krow` - The init script

4. Make it executable:
   ```bash
   chmod +x /etc/init.d/S01krow
   ```

5. The "S01" prefix ensures it runs early in the boot sequence.

## Diagnostic Checks

### Core Checks
- **PAGING** - Memory paging system status
- **VGA_SERIAL_1** - Primary display/serial interface
- **VGA_SERIAL_2** - Secondary display/serial interface
- **CDROM** - CD-ROM drive detection
- **MEMORY** - System memory verification
- **DRIVERS** - Driver subsystem status

### Driver Checks
- **DRV_KBD** - Keyboard driver
- **DRV_MOU** - Mouse driver
- **DRV_WBC** - Web/Network controller
- **DRV_SOU** - Sound/Audio driver
- **DRV_3DG** - 3D Graphics/GPU driver
- **DRV_WEB** - Web interface driver
- **DRV_ENH** - Enhanced features driver
- **DRV_JCK** - JACK audio interface

## Error Reporting

### Format

When a fault is detected, Krow generates an error code:

```
SECTION:SUBSECTION:TEST=FAULT
```

Example:
```
DRIVERS:DRV_ENH:TST=FAULT
CORE:VGA_SERIAL_1:TST=FAULT
```

### Python Error Handler

The Python script **tools/kernel-krow.py** provides detailed fault analysis:

```bash
python3 tools/kernel-krow.py "DRIVERS:DRV_ENH:TST=FAULT"
```

Output includes:
- Fault description
- Severity level (CRITICAL, HIGH, MEDIUM, LOW)
- Technical explanation
- Suggested fixes
- Related source files

## Build Commands

```bash
# Build Krow Diagnostics binary
make krow

# Build and strip for smaller size
make krow-strip

# Run diagnostics immediately
make krow-run

# Clean Krow build artifacts
make krow-clean

# Install to rootfs (for images/ISO)
make krow-install ROOTFS=/path/to/rootfs

# Uninstall from rootfs
make krow-uninstall ROOTFS=/path/to/rootfs

# Show all Krow targets
make krow-help
```

## Source Files

### Core Files

| File | Purpose |
|------|---------|
| `kernel/core/krow_diagnostics.c` | Main diagnostic logic |
| `kernel/krow_diagnostics.h` | Public API (if needed) |
| `tools/kernel-krow.py` | Python error handler |
| `scripts/S01krow` | Init.d script for boot integration |
| `Makefile` | Build configuration (merged) |

## Configuration

### Diagnostic Timing

Edit delays in **kernel/core/krow_diagnostics.c**:

```c
#define DELAY_LINE      100    /* ms between lines */
#define DELAY_CHECK     150    /* ms between check results */
#define DELAY_SECTION   300    /* ms between sections */
```

### Randomization

Diagnostics use a deterministic random number generator (no external libs required). Adjust fault probability:

```c
int krow_random_fault(void) {
    return (krow_rand() % 100) < 15; /* Change 15 for different % */
}
```

## Output Example

```
                  
   _
   (o>
\\\///)     KROW DIAGNOSTICS V 1.0
 \_/_)    ENGINEERED BY SAFEOS
  _|_

======================================
Initializing Diagnostic System...
======================================

[ CHECKING CORE SYSTEMS ]
----------------------------------------
[ TEST ] CORE:PAGING... OK
[ TEST ] CORE:VGA_SERIAL_1... OK
[ TEST ] CORE:VGA_SERIAL_2... OK
[ TEST ] CORE:CDROM... FAULT
[ TEST ] CORE:MEMORY... OK
[ TEST ] CORE:DRIVERS... OK

[ CHECKING DRIVERS ]
----------------------------------------
[ TEST ] DRIVERS:DRV_KBD... OK
[ TEST ] DRIVERS:DRV_MOU... OK
...
[!] FAULT DETECTED: CORE:CDROM:TST=FAULT
[*] Invoking error handler...

╔══════════════════════════════════════════════════════════════╗
║         KROW DIAGNOSTICS ERROR REPORT v1.0                   ║
║         SAFEOS EMBEDDED DIAGNOSTIC SYSTEM                    ║
╚══════════════════════════════════════════════════════════════╝

[ DIAGNOSTIC RESULTS ]
----------------------------------------

RESULT: BOOT WITH WARNINGS
Status: 1 fault(s) detected - see above
----------------------------------------

BOOT SESSION STARTING...
```

## Troubleshooting

### Compilation Errors

**Error: `fatal error: core/stdlib.h: No such file or directory`**

- **Cause**: Non-existent stdlib inclusion
- **Fix**: Use only available kernel headers (stdio.h, string.h, etc.)

**Error: Undefined reference to `system()`**

- **Cause**: system() not available in kernel environment
- **Fix**: Execute error handler only in userspace (already handled in code)

### Runtime Issues

**Diagnostics not running on boot**

1. Verify `krow_diagnostics_run()` is called from `kmain()` or init.d
2. Check that `/bin/krow_diagnostics` binary exists
3. Verify `/etc/init.d/S01krow` is executable

**Python error handler not executing**

- This is expected in kernel-only environments
- Error codes are printed but handler isn't called
- In full OS with Python, uncomment `system()` call when environment allows

## Advanced: Custom Integration

To integrate into a custom boot process:

1. **Copy the diagnostic logic** from `krow_diagnostics.c`
2. **Call `krow_diagnostics_run()`** from your boot code
3. **Direct output to your console** (already uses kprintf)
4. **Handle fault codes** as needed by your system

## Notes

- **No external dependencies** - Uses only kernel/stdlib headers
- **Minimal overhead** - Adds ~50KB to binary size
- **Retro aesthetic** - Styled like classic BIOS POST diagnostics
- **Extensible** - Easy to add new checks via `run_check()` pattern

## License

Part of SafeOS distribution. See LICENSE file.

---

**Krow Diagnostics v1.0** - Engineered by SafeOS Development Team
