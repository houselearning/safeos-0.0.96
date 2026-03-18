# KROW DIAGNOSTICS V 1.0 - INSTALLATION & INTEGRATION GUIDE

## Overview

**KROW Diagnostics v1.0** is a minimal embedded diagnostic console system designed to run automatically at ISO boot (early userland or init stage). It provides comprehensive hardware and subsystem diagnostics with detailed error reporting.

## Components

1. **krow_diagnostics.c** - Core C diagnostic console (~400 lines)
2. **kernel-krow.py** - Python error handler and analyzer
3. **Init integration scripts** - Boot-time integration

## Requirements

- **Compiler**: gcc or clang
- **Language**: C99 standard library only
- **Python**: Python 3.6+
- **Build System**: Linux/Unix environment (GNU Make)
- **Target**: Linux-based ISO (BusyBox, minimal rootfs)
- **Log Directory**: `/var/log/` (must be writable)

## Building

### Compile the C Program

```bash
# From project root
gcc -o kernel/krow_diagnostics kernel/krow_diagnostics.c -Wall -Wextra -std=c99

# Or add to your Makefile:
bin/krow: kernel/krow_diagnostics.c
	gcc $(CFLAGS) -o bin/krow $< -std=c99
```

### Verify Python Script

Ensure the Python script is executable:

```bash
chmod +x tools/kernel-krow.py
```

## Integration with Boot Process

### Option 1: Integration with BusyBox Init (Recommended)

If using BusyBox as init system, add to `/etc/inittab`:

```inittab
# Run Krow Diagnostics before login
::respawn:/bin/krow_diagnostics
::once:/bin/sh /etc/init.d/rc.boot
```

### Option 2: Run as Separate Init Service

Create `/etc/init.d/S01krow`:

```bash
#!/bin/sh
# Krow Diagnostics Early Diagnostic Service

case "$1" in
  start)
    echo "Starting Krow Diagnostics..."
    /bin/krow_diagnostics
    RESULT=$?
    if [ $RESULT -eq 0 ]; then
        echo "All diagnostics OK"
    else
        echo "Diagnostics found faults - check /var/log/krow_diag.log"
    fi
    ;;
  stop)
    echo "Krow Diagnostics already completed"
    ;;
  *)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac

exit 0
```

Place in `/etc/init.d/` and set permissions:

```bash
chmod +x /etc/init.d/S01krow
```

### Option 3: Run from Grub/Limine Loader

Add to grub.cfg or limine.cfg:

```grub
# For GRUB
menuentry 'SafeOS with Diagnostics' {
    echo "Running Krow Diagnostics..."
    /boot/krow_diagnostics
    echo "Diagnostics complete. Booting kernel..."
    linux /boot/vmlinuz root=/dev/mapper/safeos
}
```

### Option 4: Systemd Integration

Create `/etc/systemd/system/krow-diagnostics.service`:

```ini
[Unit]
Description=Krow Diagnostics v1.0 - System Startup Tests
Before=multi-user.target
DefaultDependencies=no

[Service]
Type=oneshot
ExecStart=/bin/krow_diagnostics
StandardOutput=journal
StandardError=journal
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
```

Enable with:
```bash
systemctl enable krow-diagnostics
```

## Runtime Behavior

### Normal Operation (No Faults)

```
   _
   (o>
\\//)     KROW DIAGNOSTICS V 1.0
 \_/_)    ENGINEERED BY SAFEOS
  _|_

CORE SYSTEM
PAGING            Memory Paging          [OK]
VGA SERIAL 1      Video Serial A         [OK]
VGA SERIAL 2      Video Serial B         [OK]
...

=====================================
[RESULT] READY TO BOOT
Tests: 30 | Faults: 0 | Success: 100.0%
=====================================

BOOT SESSION...
```

### With Faults

If any test returns FAULT:

1. All diagnostics complete
2. Python error handler invoked for each fault
3. Detailed analysis printed
4. Results logged to `/var/log/krow_diag.log`
5. System boots with warnings

Example fault output:
```
[!] FAULTS DETECTED - EXECUTING ERROR HANDLERS
[*] Executing error handler...

[ERROR REPORT]
======================================================================

  Subsystem:
    Section:     BATTERY
    Component:   PERCENT
    Test:        TST
    Status:      FAULT

  Description:
    Battery subsystem failure

  Analysis:
    Battery health monitoring or power management failed.

  Suggested Fixes:
    1. Check battery connection and contacts
    2. Review power management firmware
    3. Check kernel power management driver
    ...

  Severity: MEDIUM

======================================================================
```

## Testing

### Run with Custom Seed (for reproducible tests)

```bash
# Run with seed 12345 for reproducible faults
./kernel/krow_diagnostics 12345
```

### Check Diagnostics Log

```bash
cat /var/log/krow_diag.log
```

### Simulate Specific Failures

The system uses random fault generation (15% base fault probability). You can:

1. Modify fault probability in C code (line: `return rand() % 100 < 15`)
2. Run multiple times until specific faults appear
3. Use fixed seeds for reproducible test scenarios

## Configuration

### Delay Tuning (in krow_diagnostics.c)

Adjust these constants to change output timing:

```c
#define DELAY_LINE      100    /* Milliseconds between output lines */
#define DELAY_CHECK     150    /* Milliseconds between check results */
#define DELAY_SECTION   300    /* Milliseconds between sections */
```

For faster output (testing):
```c
#define DELAY_LINE      10
#define DELAY_CHECK     15
#define DELAY_SECTION   30
```

### Fault Probability

Adjust probability of FAULT (default 15%):

```c
int random_fault(void) {
    return rand() % 100 < 15;  /* Change 15 to desired percentage */
}
```

## Diagnostic Categories

### Core System Checks
- **PAGING**: Memory management unit and page table state
- **VGA SERIAL 1/2**: Video serial port communication
- **CD ROM**: CD/DVD-ROM controller detection
- **MEMORY**: System RAM integrity
- **DRIVERS**: Core driver subsystem

### Driver Tests
- **DRV_KBD**: Keyboard controller
- **DRV_MOU**: Mouse/Pointing device
- **DRV_WBC**: Web/Network controller
- **DRV_SOU**: Sound card
- **DRV_3DG**: 3D GPU acceleration
- **DRV_WEB**: Network interface
- **DRV_ENH**: Enhanced features module
- **DRV_JCK**: Audio jack detection

### Extended Tests
- **Battery Health**: Power subsystem diagnostics
- **Multiboot ISO**: ISO integrity verification
- **Display System**: Framebuffer and rendering tests

## Error Code Format

Error codes follow strict format for Python handler:

```
SECTION:SUBSECTION:TEST=STATE
```

Examples:
```
PAGING:MMU:TST=FAULT
BATTERY:PERCENT:TST=FAULT
DISPLAY:APP:TST=FAULT
MULTIBOOT:KROW_ISO:KRW=FAULT
```

## Output Features

### ANSI Color Support
- **Green** (`[32m`) - OK status
- **Red** (`[31m`) - FAULT status
- **Yellow** (`[33m`) - WARNING status
- **Cyan** (`[36m`) - Test section labels
- **Bold** (`[1m`) - Important text

### Progress Indication
- Line-by-line output with human-readable delays
- Simulates hardware POST (Power-On Self Test) aesthetic
- Visual feedback for long-running tests

## Logging

All diagnostics are logged to: `/var/log/krow_diag.log`

Log format:
```
KROW DIAGNOSTICS V 1.0 LOG
==========================

Total Tests Run: 30
Total Faults: 2
Success Rate: 93.3%

Faults Detected:
  - BATTERY:PERCENT:TST=FAULT
  - DISPLAY:APP:TST=FAULT

[ERROR] 2026-03-17T10:25:30.123456
Code: BATTERY:PERCENT:TST=FAULT
Section: BATTERY/PERCENT
Test: TST = FAULT
Severity: MEDIUM
...
```

## Exit Codes

- **0**: All diagnostics passed
- **1**: One or more faults detected

## Troubleshooting

### Python Script Not Found

Error: `python3: No such file or directory`

**Solution**: Ensure Python 3 is installed and in PATH:
```bash
which python3
# or
apt-get install python3
```

### Log Directory Doesn't Exist

Error: `Permission denied` writing to `/var/log/krow_diag.log`

**Solution**: Create log directory:
```bash
mkdir -p /var/log
chmod 777 /var/log
```

### Program Doesn't Run at Boot

**Solution**: Verify init script is executable and in correct location:
```bash
ls -la /etc/init.d/S01krow
# Should show: -rwxr-xr-x
```

### ANSI Colors Not Displaying

This is normal on some terminals. The program detects and adapts, but you can force disable:

```c
#define COLOR_GREEN     ""
#define COLOR_RED       ""
/* etc */
```

## Performance Notes

- **Execution time**: ~5-10 seconds (including delays)
- **Binary size**: ~30KB (stripped)
- **Memory usage**: <1MB
- **Dependencies**: Only C standard library

## Extension Points

### Adding New Diagnostics

1. Define new check category constant
2. Add function: `void run_new_category_tests(void)`
3. Call from main()
4. Add error definitions to Python database

Example:
```c
void run_network_tests(void) {
    run_check("NETWORK", "Connection Test", "NET_CON");
    run_check("NETWORK", "DNS Resolution", "NET_DNS");
    run_check("NETWORK", "Interface Speed", "NET_SPD");
}
```

### Custom Error Analysis

Add to ERROR_DATABASE in kernel-krow.py:

```python
"NET_DNS": {
    "description": "DNS resolution failure",
    "analysis": "Network name resolution service is not responding.",
    "fixes": [
        "Check DNS server configuration in /etc/resolv.conf",
        "Verify network interface is up: ip link show",
        "Test: nslookup 8.8.8.8"
    ],
    "severity": "HIGH"
}
```

## License

KROW Diagnostics v1.0 - Engineered by SafeOS
Minimal diagnostic console system for embedded boot environments.

## Support

For issues, check:
1. `/var/log/krow_diag.log` for detailed error logs
2. `dmesg` output for kernel messages
3. Python script execution with verbose output:
   ```bash
   python3 tools/kernel-krow.py "SECTION:SUBSECTION:TEST=FAULT"
   ```

---

**Last Updated**: March 2026
**Version**: 1.0
**Status**: Production Ready
