# Krow Extended Test Suite (Force Mode)

## Overview

The Krow Diagnostics system now supports an **extended test mode** that runs comprehensive system validation with 600+ tests and **REAL ISO validation** (not simulated).

**Version:** Krow Diagnostics v1.0 Extended  
**Command:** `make krow-livedn force`  
**Test Count:** 800+ comprehensive tests  
**Execution Time:** ~2-3 minutes (with realistic diagnostics)  
**ISO Validation:** Real SafeOS-1.0.iso file analysis (not simulated)

## Quick Start

### Standard Mode (14 original checks)
```bash
make krow-livedn
```

### Extended Mode (600+ tests + REAL ISO validation)
```bash
make krow-livedn force
```

## What's Real vs. Simulated

### ✅ REAL ISO Validation
- **Actual ISO file detection:** Searches for `SafeOS-1.0.iso`
- **Real CRC32 calculation:** Full ISO file integrity checksum
- **Boot sector validation:** Checks boot signatures
- **File parsing:** Extracts actual file list from ISO
- **Size reporting:** Shows actual ISO file size
- **File count:** Reports actual files in ISO

### 📊 Simulated Tests (for comparison/stress testing)
- Network connectivity (PING, DNS, TCP, UDP)
- Hardware detection (PCI, USB, ACPI, MSR, DMI)
- Performance benchmarks (memory, cache, context switching)
- Advanced driver checks (stress testing)

## Extended Test Breakdown

### Part 1: Comprehensive Stress Tests (600 tests)
**Duration:** ~50-60 seconds

- **50 iterations** of core and driver checks
- Tests 6 core subsystems: CPU, MEMORY, GDT, IDT, PIC, TIMER
- Tests 8 drivers: SERIAL, VGA, FRAMEBUFFER, KEYBOARD, MOUSE, ATA, RTL8139, E1000
- Each iteration runs 14 check combinations
- **Total:** ~600 comprehensive stress tests

### Part 2: Network & Connectivity Tests (40 mini-tests)
**Duration:** ~30-40 seconds | **Type:** Simulated

Tests 10 network subsystems × 4 cycles:
- Interface enumeration, ping (local/gateway), DNS, TCP/UDP, ARP, routing
- **Fault Rate:** 20% (network less reliable in simulation)

### Part 3: ISO Filesystem Integrity Tests (8 REAL tests)
**Duration:** ~30-40 seconds | **Type:** REAL (not simulated)

Analyzes the actual `SafeOS-1.0.iso` file:
1. **ISO_FOUND** - Locates SafeOS-1.0.iso file and reports size
2. **ISO_BOOT** - Validates boot sector and signatures
3. **VOLUME_DESC** - Checks volume descriptor table  
4. **DIR_RECORDS** - Reads and validates directory structure
5. **CRC_CHECK** - Calculates and displays CRC32 checksum
6. **FILE_EXTENTS** - Validates file extent allocation
7. **KERNEL_IMAGE** - Detects kernel/boot files in ISO
8. **CONSISTENCY** - Verifies filesystem consistency

**Output Sample:**
```
╔═══════════════════════════════════════╗
║  REAL ISO FILESYSTEM INTEGRITY TESTS  ║
║  Analyzing SafeOS-1.0.iso             ║
╚═══════════════════════════════════════╝

  [PASS] ISO_FOUND: SafeOS-1.0.iso (45.2 MB)
  [PASS] ISO_BOOT: ISO boot sector validation OK
  [PASS] VOLUME_DESC: Volume descriptor table OK
  [PASS] DIR_RECORDS: Directory structure OK (23 files)
  [PASS] CRC_CHECK: CRC32=a1b2c3d4
  [PASS] FILE_EXTENTS: File extent allocation OK
  [PASS] KERNEL_IMAGE: Bootable kernel found in ISO
  [PASS] CONSISTENCY: ISO filesystem consistency verified

  ISO Details:
    - Path: SafeOS-1.0.iso
    - Size: 45.2 MB
    - Bootable: Yes
    - CRC32: a1b2c3d4
    - Files: 23
```

### Part 4: Advanced Hardware Detection (40 mini-tests)
**Duration:** ~30-40 seconds | **Type:** Simulated

Tests hardware subsystems:
- PCI enumeration/config, USB scanning, ACPI parsing, CPUID, MSR, DMI, BIOS ROM, memory map, E820
- **Fault Rate:** 12% (moderate reliability)

### Part 5: Performance Benchmarks (20 mini-tests)
**Duration:** ~20-35 seconds | **Type:** Simulated

Tests performance subsystems:
- Memory bandwidth, cache latency, context switching, interrupt latency, TSC frequency, timer accuracy
- **Fault Rate:** 8% (usually stable)

## ISO Validation Details

### How ISO Detection Works
The simulator searches for the ISO file in multiple locations:
1. `SafeOS-1.0.iso` (current directory)
2. `/workspaces/safeos-0.0.96/SafeOS-1.0.iso` (development workspace)
3. `./build/SafeOS-1.0.iso` (build directory)

### What Gets Validated

**File Existence & Size**
```python
- Checks if ISO file can be found
- Reads actual file size from filesystem
- Reports in megabytes
```

**Boot Sector Validation**
```python
- Reads first sectors looking for boot signatures
- Checks volume descriptor table (at offset 0x8000)
- Validates ISO 9660 standard compliance
```

**CRC32 Integrity**
```python
- Reads entire ISO file in 64KB chunks
- Calculates cumulative CRC32 checksum
- Reports hex value for integrity tracking
- Useful for detecting corruption
```

**File Structure Analysis**
```python
- Parses ISO directory entries
- Extracts actual filenames from ISO
- Counts total files present
- Reports file count
```

**Bootability Check**
```python
- Verifies file is actually bootable (has boot sectors)
- Checks for kernel image files (SafeOS.bin, kernel files)
- Validates boot signatures
```

## Test Statistics

### Standard Mode
- **Total Tests:** 20
- **Unique Tests:** 6 core + 8 drivers + 6 extended
- **Typical Faults:** 2-3

### Extended Mode
- **Comprehensive Tests:** ~600
- **Network Tests:** 40 (simulated)
- **ISO Tests:** 8 (REAL)
- **Hardware Tests:** 40 (simulated)
- **Performance Tests:** 20 (simulated)
- **Total Tests:** 708
- **Expected Pass Rate:** 88-95%
- **Real vs. Simulated:** 8/708 (~1%) real, rest simulated for variety

## Fault Rates by Category

| Category | Fault Rate | Type | Reason |
|----------|-----------|------|--------|
| Core Checks | 15% | Simulated | Standard hardware faults |
| Network Tests | 20% | Simulated | Network connectivity issues |
| ISO Tests | 0-12% | REAL | Based on actual ISO |
| Hardware Tests | 12% | Simulated | Hardware detection variability |
| Performance Tests | 8% | Simulated | Usually stable |

## Output Summary

### Final Summary (Extended Mode)
```
╔═══════════════════════════════════════╗
║      DIAGNOSTIC SUMMARY               ║
╚═══════════════════════════════════════╝

  CPU: Intel Core i7 (simulated)
  Memory: 4096 MB total, 3984 MB available
  Drivers: 8 initialized
  Checkpoints: 14 passed
  Total Tests: 708
  Passed: 650 (91.8%)
  Failed: 58 (8.2%)
  Status: BOOT DIAGNOSTICS PASSED - SYSTEM READY
  Timestamp: 2024-01-15 14:23:45
```

## Status Determination

| Pass Rate | Status | Color |
|-----------|--------|-------|
| ≥95% | System Ready | Green |
| ≥80% | Warnings Present | Yellow |
| <80% | Critical Issues | Red |

## Use Cases

### Scenario 1: Pre-Release Verification
```bash
make clean && make iso      # Build fresh ISO
make krow-livedn force      # Validate it
```

### Scenario 2: ISO Integrity Monitoring
```bash
make krow-livedn force      # Track CRC32 values over time
```

### Scenario 3: Debug Boot Issues
```bash
make krow-livedn force      # Check what got built into ISO
```

## Technical Implementation

### ISOReader Class
```python
class ISOReader:
    - __init__(iso_path)      # Initialize with path
    - read()                   # Validate and parse ISO
    - is_bootable()           # Check boot sectors
    - get_size_mb()           # Report size
    - get_file_count()        # Count files
    - get_crc_hex()           # Get CRC32 as hex
```

### Real ISO Validation Steps
1. Search for ISO file in multiple locations
2. Open file in binary mode
3. Calculate complete CRC32 checksum
4. Parse boot sector signatures
5. Read volume descriptor table
6. Extract file directory entries
7. Report all findings

### Performance
- Entire ISO analyzed in ~5-10 seconds
- CRC32 calculation for 45MB ISO: ~2-3 seconds
- No external tools or libraries needed
- Pure Python 3 (uses only stdlib)

## Integration Notes

- **Real ISO:** Part 3 does actual validation
- **Simulated Tests:** Parts 1, 2, 4, 5 use deterministic RNG
- **Backward Compatible:** Standard mode unchanged
- **No Breaking Changes:** All original tests still work

## Troubleshooting

### "ISO not found" Error
```
make iso                    # Build the ISO first
make krow-livedn force      # Then run diagnostics
```

### No Files Detected in ISO
- ISO file may be corrupted
- Directory table parsing limitation
- Normal warning, not critical

### CRC32 Mismatch
- ISO file was modified
- Corruption during transfer
- Expected if you manually edited files

## See Also
- [Krow Diagnostics README](README.md)
- [Krow Integration Guide](KROW_INTEGRATION.md)
- [Kernel Quick Start](QUICK_START.md)
