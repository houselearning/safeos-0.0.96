# grub-install Setup & Testing Guide

## Overview

The SafeOS project includes infrastructure for testing and deploying `grub-install` on disk images, USB drives, and raw devices. This guide explains the setup and how to use the provided tools.

---

## Files Provided

### Test Script: `scripts/test_grub_install.sh`

**Purpose:** Automated testing of grub-install functionality on a loopback device without risk to real system disks.

**Location:** `/workspaces/safeos-0.0.96/scripts/test_grub_install.sh`

**Requirements:**
- sudo/root access
- `grub-install` command available
- `losetup` utility
- `mkfs.ext*` or similar filesystem tools
- `mount` command

**What It Does:**
1. Creates a 100MB test disk image `/tmp/safeos_test_disk.img`
2. Sets up MBR partition table
3. Formats with ext4 filesystem (BIOS-compatible)
4. Mounts as loopback device
5. Installs GRUB bootloader (i386-pc target)
6. Verifies core.img and grub.cfg presence
7. Cleans up and reports results

**Usage:**

```bash
# Run the test (requires sudo)
cd /workspaces/safeos-0.0.96
sudo bash scripts/test_grub_install.sh

# Expected output:
# === SafeOS grub-install Loopback Test ===
# [1/5] Creating 100MB test disk image...
# ✓ Image created
# [2/5] Setting up MBR and partition...
# ✓ Partition formatted
# [3/5] Mounting image as loopback...
# ✓ Mounted: /dev/loopX → /tmp/safeos_test_mnt
# [4/5] Installing GRUB bootloader...
# ✓ GRUB installed successfully
# [5/5] Verifying GRUB installation...
# ✓ core.img found
# ✓ grub.cfg found
# ✓ SafeOS.bin found
# === Test Complete ===
# Test image available at: /tmp/safeos_test_disk.img
# To boot this image in QEMU:
#   qemu-system-i386 -hda /tmp/safeos_test_disk.img -m 1024 -serial stdio
```

---

## Repository Tool: `tools/krow_bootrepair.py`

**Purpose:** Host-side repair tool that includes grub-install integration for automated bootloader fixes.

**Relevant Methods:**

### 1. Mount Image
```python
bootrepair = BootRepair(repo_root)
mount_point = bootrepair.mount_image(image_path)
```

Mounts a disk image using loopback:
- Detects partitions (partition 1 assumed)
- Mounts read-write to temp directory
- Returns mount path

### 2. Install Bootloader
```python
bootrepair.install_bootloader_to_image(image_path, mount_point)
```

Installs GRUB to a disk image:
```bash
# Internally runs:
grub-install \
  --target=i386-pc \
  --boot-directory=$MOUNT_POINT/boot \
  --no-floppy \
  --recheck \
  $LOOP_DEVICE
```

**Flags Explained:**
- `--target=i386-pc`: 32-bit x86 BIOS (MBR era systems)
- `--boot-directory`: Where GRUB stores core.img and grub.cfg
- `--no-floppy`: Disable floppy drive probing (faster)
- `--recheck`: Force rescan of device (bypass caching)
- `$LOOP_DEVICE`: Target device (e.g., /dev/loop0)

### 3. Full Repair Workflow
```python
# Diagnose
issues = bootrepair.diagnose(target_dir="build/iso")

# Apply repair (with backup)
bootrepair.repair(target_dir="build/iso", apply=True)

# Image-based repair (requires mount)
bootrepair.repair(mount_image=True, install_bootloader=True, apply=True)
```

---

##  Manual grub-install (Desktop/Workstation)

If you want to manually install GRUB to a disk or USB:

### Prerequisites
```bash
# Ubuntu/Debian:
sudo apt-get install grub-pc grub-common

# Check installation:
which grub-install
grub-install --version
```

### Identify Target Device
```bash
# List all disks (WARNING: be very careful here!)
lsblk
# or
sudo fdisk -l

# Example output:
# Disk /dev/sda: 1000 GB, 1000204886016 bytes
# Disk /dev/sdb: 8 GB, 8053063680 bytes  ← Likely USB/target
```

### Create Partition Table & Format

```bash
# IMPORTANT: Replace /dev/sdX with your ACTUAL device
# (Not /dev/sda unless you want to wipe your main disk!)

DEVICE=/dev/sdX  # Change to your device

# 1. Partition the disk
sudo fdisk "$DEVICE"
# Commands:
#   o        → Create MBR label
#   n        → New partition
#   p        → Primary
#   1        → Partition 1
#   [Enter]  → Default start
#   [Enter]  → Default end (full disk)
#   a        → Make bootable
#   1        → Partition 1
#   t        → Set type
#   83       → Linux (ext4)
#   w        → Write

# 2. Format partition
sudo mkfs.ext4 "${DEVICE}1"

# 3. Mount temporarily
mkdir -p /tmp/bootsda
sudo mount "${DEVICE}1" /tmp/bootsda
```

### Install GRUB

```bash
# Create boot directory
sudo mkdir -p /tmp/bootsda/boot/grub

# Copy kernel and config
sudo cp SafeOS.bin /tmp/bootsda/boot/
sudo cp iso/grub.cfg /tmp/bootsda/boot/grub/

# Install bootloader to MBR
sudo grub-install \
  --target=i386-pc \
  --boot-directory=/tmp/bootsda/boot \
  --no-floppy \
  --recheck \
  "$DEVICE"

# Expected output:
# Installing for i386-pc platform.
# grub-install: warning: this GPT partition label contains no BIOS Boot Partition; ...
# Installation finished. No error reported.
```

### Verify Installation

```bash
# Check for core.img and grub.cfg
ls -la /tmp/bootsda/boot/grub/
# Should show:
#   core.img
#   grub.cfg
#   i386-pc/
#   (other GRUB files)

# Clean up
sudo umount /tmp/bootsda
# Device is now bootable!
```

### Boot from Device

```bash
# QEMU test
qemu-system-i386 -hda "$DEVICE" -m 1024 -serial stdio

# Or physically insert USB/disk and boot from BIOS
```

---

## Troubleshooting grub-install

### Error: "Could not find device for /boot/grub"

**Cause:** Loopback device not found or partition detection failed

**Solution:**
```bash
# Check loop device setup
sudo losetup -l

# If stuck, cleanup:
sudo losetup -D  # Detach all loop devs
```

### Error: "MBR doesn't have BIOS Boot Partition"

**Cause:** Using GPT partition table instead of MBR

**Solution:**
```bash
# Create MBR instead:
sudo parted "$DEVICE" mklabel msdos
sudo grub-install --target=i386-pc --boot-directory=/boot "$DEVICE"
```

### Error: "cannot find a GRUB drive for /dev/sda1"

**Cause:** grub-install can't map partition to disk

**Solution:**
```bash
# Use --recheck flag to force rescan
sudo grub-install \
  --target=i386-pc \
  --boot-directory=/boot \
  --recheck \
  /dev/sda  # Use disk, not partition
```

### grub-install Hangs

**Cause:** Device is busy or floppy detection taking too long

**Solution:**
```bash
# Use --no-floppy to disable floppy check
sudo grub-install \
  --target=i386-pc \
  --boot-directory=/boot \
  --no-floppy \
  --recheck \
  /dev/sda
```

---

## Integration with SafeOS Boot Repair

The `tools/krow_bootrepair.py` tool handles all of this automatically:

```bash
# 1. Diagnose boot issues
python3 tools/krow_bootrepair.py diagnose

# 2. Auto-repair with GRUB reinstall
python3 tools/krow_bootrepair.py repair --apply --mount-image --install-bootloader

# 3. Package as recovery ISO
python3 tools/krow_bootrepair.py package --iso --recovery

# All grub-install flags are pre-configured:
#   --target=i386-pc (32-bit BIOS)
#   --boot-directory=auto (uses mount point)
#   --no-floppy (for speed)
#   --recheck (force rescan)
```

---

## Performance Notes

**grub-install Typical Times:**

| Operation | Time |
|-----------|------|
| Loopback mount | <1s |
| Format ext4 | 1-5s |
| grub-install | 2-5s |
| Full test script | 10-20s |

**To Optimize:**
- Pre-create partition to avoid fdisk dialogs
- Use ext2/ext3 instead of ext4 (slightly faster)
- Disable device recheck if device is known stable (`--norecheck`)

---

##  Reference: grub-install Options

```
grub-install [OPTION]... INSTALL_DEVICE

Options:
  --target=TARGET           i386-pc (MBR/BIOS), x86_64-efi (UEFI), etc.
  --boot-directory=DIR      Directory where boot/grub is located (default: /boot)
  --root-directory=DIR      Root filesystem directory
  --modules=MODULE1,MOD2    Pre-load modules (default auto-detect)
  --install-modules=LIST    Modules to install on device
  --fonts=FONT1,FONT2       Fonts to install
  --themes=THEME            Theme directory to install
  --locales=LOCALE1,LOC2    Locales to install
  --compress=TYPE           Compression (auto/gzip/none)
  --no-floppy               Skip floppy device detection
  --recheck                 Force re-detection of block devices
  --force                   Force installation even if errors detected
  --directory=DIR           Alternative GRUB modules directory
  --verbose=LEVEL           Verbosity level (0-3)
  --debug                   Enable debug mode
  -h, --help                Show this help
  -v, --version             Show version
```

---

## Summary

| Method | Best For | Ease | Speed |
|--------|----------|------|-------|
| `scripts/test_grub_install.sh` | Testing & learning | Easy (auto) | ~20s |
| `tools/krow_bootrepair.py repair` | Automated repair ISO | Easy (CLI) | ~30s |
| Manual `grub-install` | Production USB | Hard (manual) | ~5m |
| Boot from ISO + menu | Emergency recovery | Easy (interactive) | ~10s |

---

**Last Updated:** 2026-04-02
**Version:** SafeOS 0.0.96
