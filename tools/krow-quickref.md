# KROW DIAGNOSTICS v1.0 - QUICK REFERENCE

## 🚀 Quick Start

### Build
```bash
gcc -Wall -Wextra -std=c99 -o kernel/krow_diagnostics kernel/krow_diagnostics.c
```

### Run
```bash
./kernel/krow_diagnostics          # Run with random results
./kernel/krow_diagnostics 12345    # Run with seed 12345 (reproducible)
```

### Install to ISO
```bash
make krow-install ROOTFS=/path/to/rootfs
```

---

## 📋 File Locations

```
Diagnostic System:
  /vsls/kernel/krow_diagnostics.c      ← Main C program (~400 lines)
  /vsls/tools/kernel-krow.py           ← Error handler (Python ~300 lines)
  /vsls/tools/krow-howto.md            ← Full documentation
  /vsls/tools/krow-quickref.md         ← This file

Integration:
  /vsls/scripts/S01krow                ← Init script
  /vsls/scripts/krow-integration.sh    ← Integration examples
  /vsls/Makefile.krow                  ← Build rules

Testing:
  /vsls/tools/test-krow.py             ← Test suite
```

---

## 🔧 Build & Installation

### Method 1: Make (Recommended)
```bash
# Include in your Makefile:
include Makefile.krow

# Build:
make krow

# Build and run:
make krow-run

# Build with custom seed:
make krow-run-seed SEED=42

# Install to rootfs:
make krow-install ROOTFS=/path/to/rootfs
```

### Method 2: Manual GCC
```bash
gcc -Wall -Wextra -std=c99 -O2 \
    -o kernel/krow_diagnostics \
    kernel/krow_diagnostics.c

# Optional: Strip for smaller size
strip kernel/krow_diagnostics
```

### Method 3: Cross-Compile
```bash
arm-linux-gnueabihf-gcc -std=c99 \
    -o krow_diagnostics.arm \
    kernel/krow_diagnostics.c

# For 32-bit x86:
gcc -m32 -std=c99 -o krow_diag.i386 kernel/krow_diagnostics.c
```

---

## 🎯 Integration Methods

### 1. **BusyBox Init** (Simplest)
Edit `/etc/inittab`:
```inittab
::once:/etc/init.d/S01krow
```

### 2. **Init Script Service**
```bash
cp scripts/S01krow /etc/init.d/
chmod +x /etc/init.d/S01krow
```

### 3. **Systemd**
```bash
cat > /etc/systemd/system/krow.service << 'EOF'
[Unit]
Description=Krow Diagnostics
Before=multi-user.target
DefaultDependencies=no

[Service]
Type=oneshot
ExecStart=/bin/krow_diagnostics
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

systemctl enable krow.service
```

### 4. **Limine Bootloader**
In `limine.cfg`:
```cfg
:SafeOS
KERNEL_PATH=boot:///vmlinuz
KERNEL_CMDLINE=init=/etc/init.d/S01krow ro root=/dev/mapper/root
```

### 5. **GRUB**
In `grub.cfg`:
```grub
linux /vmlinuz init=/bin/krow_diagnostics root=/dev/mapper/root
initrd /initramfs.cpio.gz
```

---

## 🧪 Testing

### Run Tests
```bash
# All tests
python3 tools/test-krow.py --run-tests

# C and Python syntax only
python3 tools/test-krow.py --check-syntax

# Benchmark
python3 tools/test-krow.py --benchmark

# Stress test (50 iterations)
python3 tools/test-krow.py --stress-test 50
```

### Manual Testing
```bash
# Run with fixed seed (reproducible)
./kernel/krow_diagnostics 42

# Different seeds each run
./kernel/krow_diagnostics $RANDOM
./kernel/krow_diagnostics $((RANDOM * RANDOM))

# Capture to file
./kernel/krow_diagnostics | tee krow_output.log

# Redirect to log
./kernel/krow_diagnostics > /var/log/krow_manual.log 2>&1
```

---

## 📊 Output Examples

### Successful Run
```
   _
   (o>
\\//)     KROW DIAGNOSTICS V 1.0
 \_/_)    ENGINEERED BY SAFEOS
  _|_

CORE SYSTEM
PAGING            Memory Paging          [OK]
VGA SERIAL 1      Video Serial A         [OK]
...

DRIVERS
DRV KBD           KEYBOARD               [OK]
...

BATTERY HEALTH
BATTERY HEALTH:   95 PRC
    CRG 170 OK
    PRC 096 OK

=====================================
[RESULT] READY TO BOOT
Tests: 30 | Faults: 0 | Success: 100.0%
=====================================

BOOT SESSION...
```

### With Faults
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
    Battery health monitoring failed.
  
  Suggested Fixes:
    1. Check battery connection
    2. Review power management firmware
    ...
  
  Severity: MEDIUM
======================================================================

[RESULT] BOOT WITH WARNINGS
Tests: 30 | Faults: 2 | Success: 93.3%
```

---

## 🔍 Error Code Format

```
SECTION:SUBSECTION:TEST=STATE
```

Examples:
```
PAGING:MMU:TST=FAULT
BATTERY:PERCENT:TST=FAULT
DISPLAY:APP:TST=FAULT
MEMORY:RAM:TST=OK
MULTIBOOT:KROW_ISO:KRW=FAULT
DRV_KBD:INPUT:TST=OK
```

### Test in Python
```bash
python3 tools/kernel-krow.py "BATTERY:PERCENT:TST=FAULT"
python3 tools/kernel-krow.py "DISPLAY:APP:TST=FAULT"
python3 tools/kernel-krow.py "PAGING:MMU:TST=FAULT"
```

---

## 📝 Configuration

### Adjust Delays (in C source)
```c
#define DELAY_LINE      100    /* ms per output line */
#define DELAY_CHECK     150    /* ms per test result */
#define DELAY_SECTION   300    /* ms between sections */
```

For fast output (testing):
```c
#define DELAY_LINE      0
#define DELAY_CHECK     0
#define DELAY_SECTION   0
```

### Adjust Fault Probability
```c
int random_fault(void) {
    return rand() % 100 < 15;  /* 15% fault chance - change here */
}
```

### Enable/Disable Colors
Remove or comment out COLOR definitions to disable ANSI codes for logging systems that don't support them.

---

## 🛠️ Troubleshooting

### Python Not Found
```bash
which python3
apt-get install python3
```

### Log Directory Missing
```bash
mkdir -p /var/log
chmod 777 /var/log
```

### Init Script Not Running
```bash
# Check if executable
ls -la /etc/init.d/S01krow

# Should show: -rwxr-xr-x

# Manually run
/etc/init.d/S01krow start
```

### No Color Output
Check if terminal supports ANSI:
```bash
echo -e "\033[32mGreen\033[0m"

# If colors don't work, disable in source:
#define COLOR_GREEN ""
```

### Fast Exit (No Delays)
Set all delays to 0:
```bash
# Edit source before building
vim kernel/krow_diagnostics.c
# Change DELAY_* values to 0
gcc -o kernel/krow_diagnostics kernel/krow_diagnostics.c
```

---

## 📊 Diagnostic Coverage

**Core System**: PAGING, VGA SERIAL 1/2, CD ROM, MEMORY, DRIVERS
**Drivers**: KBD, MOU (MOUSE), WBC (WEB), SOU (SOUND), 3DG (GPU), WEB (NET), ENH, JCK
**Power**: BATTERY (health, charge, percent)
**Media**: MULTIBOOT (krowtest.iso, safeosframe.iso)
**Display**: APP, USB, BCK, LOG, ANM, WEB

**Total Tests**: ~30 per run
**Fault Chance**: ~15% per test (configurable)
**Execution Time**: ~5-10 seconds

---

## 🔐 Exit Codes

```
0 = Success (all tests OK)
1 = Fault detected (see log for details)
-1 = Error in Python script
```

---

## 📂 Log Files

```
/var/log/krow_diag.log     ← Main diagnostic log
```

View log:
```bash
cat /var/log/krow_diag.log
tail -f /var/log/krow_diag.log
grep FAULT /var/log/krow_diag.log
```

---

## 💡 Tips & Tricks

### Test Specific Seed
```bash
# Reproducible test run
./kernel/krow_diagnostics 999

# Same seed = same results
./kernel/krow_diagnostics 999   # Exact same output as above
```

### Chain with Other Init Services
```bash
# In S01krow, after diagnostics:
/etc/init.d/S02filesystem start
/etc/init.d/S03networking start
```

### Minimal Build
```bash
gcc -Os -s kernel/krow_diagnostics.c -o krow  # 15KB stripped
```

### Cross-Platform Build
```bash
# ARM (Raspberry Pi, etc)
arm-linux-gnueabihf-gcc -O2 -o krow_arm kernel/krow_diagnostics.c

# RISC-V
riscv64-unknown-linux-gnu-gcc -O2 -o krow_riscv kernel/krow_diagnostics.c
```

---

## 🎓 Learning Resources

- [Full Documentation](krow-howto.md)
- [Integration Examples](../scripts/krow-integration.sh)
- [C Source](../kernel/krow_diagnostics.c)
- [Python Script](kernel-krow.py)

---

## 📝 Version Info

```
KROW Diagnostics v1.0
Engineered by SafeOS
Production Ready
```

---

## 🤝 Contributing

To extend KROW:

1. **Add new test**: Modify C source `run_*_checks()` function
2. **Add error handling**: Add to `ERROR_DATABASE` in Python script
3. **Test changes**: Run `python3 tools/test-krow.py --run-tests`

---

**Last Updated**: March 2026
