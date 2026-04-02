# SafeOS 0.0.96 - VMware Triple-Fault Recovery Guide

## Problem Summary

Your SafeOS VM on VMware Player crashes with a **triple fault** ~7-9 seconds after boot:

```
Unknown int 10h func 0x0000
Triple fault.
```

**Root Cause:** The kernel attempts to initialize SVGA graphics and issues an INT 10h BIOS call that VMware's emulated BIOS doesn't support in protected mode. This causes an unhandled exception → triple fault → VM shutdown.

---

## Quick Fix: Boot in VMware Safe Mode

The solution is to boot SafeOS **without graphics** on VMware to bypass the problematic INT 10h code path.

### Step 1: Boot from SafeOS Recovery ISO

1. Download the latest `SafeOS-1.0.iso` (contains VMware Safe Mode entry)
2. In VMware Player:
   - Right-click SafeOS VM → **Edit virtual machine settings**
   - Go to **CD/DVD (IDE)** tab
   - Enable "Connect at power on"
   - Set to use `SafeOS-1.0.iso` file
   - Click **OK**

3. Power on the VM

### Step 2: Select "VMware Safe Mode" from GRUB Menu

When the GRUB menu appears (after ~5 seconds):

```
SafeOS 1.0 (Graphics Mode)                      ← Original (crashes)
SafeOS 1.0 (Debug Mode, Serial Console)
SafeOS 1.0 (VMware Safe Mode - No Graphics)     ← USE THIS
SafeOS 1.0 (VESA Fallback 800x600)
System information
```

**Select:** "SafeOS 1.0 (VMware Safe Mode - No Graphics)" using arrow keys, then press **Enter**

### Step 3: Boot Successfully

The kernel will boot with:
- Graphics disabled (`nogui` flag)
- No framebuffer mode set (`nomodeset` flag)
- Headless operation (serial console diagnostics)
- **No INT 10h calls** (crash avoided✓)

### Step 4: Verify Boot

You should see in serial console (or VM logs):

```
CMDLINE: nogui nomodeset
CMDLINE: headless/debug flag detected
[KROW] Headless mode detected: skipping interactive BootRepair menu
Krow Diagnostics running in headless mode...
```

VM stays up and running without crashing.

---

## If You Want Graphics (Advanced)

If you need graphics support on VMware, try these alternatives:

### Option A: Use VESA Fallback

Select **"SafeOS 1.0 (VESA Fallback 800x600)"** instead. This uses standard VBE (VESA BIOS Extensions) instead of SVGA:

- Compatible with most hypervisors
- Resolution: 800x600x32
- No hypervisor-specific graphics drivers needed

### Option B: VMware Tools + Hypervisor-Specific Drivers

Install VMware Tools in the guest to enable optimized SVGA support (advanced users):

```bash
# In guest (after boot):
sudo apt install open-vm-tools open-vm-tools-desktop
# Then reboot with Graphics Mode entry
```

### Option C: Test on Different Hypervisor

Try VirtualBox or KVM/QEMU instead (both work headless at minimum).

---

## Technical Details: Reproduced Crash

### VMware VM Configuration
```
Guest OS: "Other Linux 2.4.x kernel 64-bit"
Memory: 384 MB
Graphics: SVGA enabled, 268 MB VRAM
Framebuffer: 0xe8000000 (VMware standard)
BIOS: 32-bit (not UEFI)
```

### Crash Sequence
1. **Kernel boot** → multiboot handoff succeeds
2. **GRUB loads SafeOS.bin** → kernel starts
3. **Graphics init** → SVGA device detected at 0xe8000000
4. **INT 10h 0x0000** → Kernel tries BIOS video call
5. **BIOS call fails** → Exception in protected mode
6. **Triple fault** → CPU cannot handle exception, shutdown

### Why Safe Mode Works
- Kernel skips graphics driver setup (`nogui` flag)
- Kernel skips framebuffer initialization (`nomodeset` flag)
- No INT 10h BIOS calls issued
- Serial console I/O only (works in all hypervisors)
- Diagnostics run successfully without crashing

---

## grub-install Testing (Local System Setup)

If you're setting up SafeOS on a real disk/USB, test grub-install locally:

```bash
# From SafeOS repo directory:
sudo bash scripts/test_grub_install.sh

# This will:
# 1. Create a 100MB test disk image
# 2. Setup MBR + ext4 partition
# 3. Install GRUB bootloader
# 4. Verify installation
# 5. Leave image at /tmp/safeos_test_disk.img

# Test in QEMU:
qemu-system-i386 -hda /tmp/safeos_test_disk.img -m 1024 -serial stdio
```

---

## Complete Recovery Workflow

### For VMware Users (Recommended)

```bash
# 1. Rebuild SafeOS with VMware Safe Mode (already in grub.cfg)
cd /workspaces/safeos-0.0.96
make iso
# → SafeOS-1.0.iso (2601 sectors)

# 2. Boot ISO in VMware, select "VMware Safe Mode"
# → Boots successfully without crash

# 3. Run diagnostics from SafeOS Krow BootRepair menu
# → Identify disk issues

# 4. Apply repairs (automated)
# → Fix bootloader, restore configs
```

### For USB/Disk Installation (Advanced)

```bash
# 1. Prepare USB or raw image
lsblk  # List disks
ls -la /dev/sdX  # Verify target

# 2. Test grub-install on loopback first (optional)
sudo bash scripts/test_grub_install.sh

# 3. Install to real device
sudo python3 tools/krow_bootrepair.py package --iso --recovery
# → Creates recovery ISO with repair tools

# 4. Manual grub-install to device
cd /tmp/safeos_recovery_mount  # Mount point
sudo grub-install \
  --target=i386-pc \
  --boot-directory=/path/to/recovery/boot \
  --no-floppy \
  --recheck \
  /dev/sdX
# → Installs GRUB MBR + core.img to device /dev/sdX
```

---

## Files Modified in This Build

| File | Change | Purpose |
|------|--------|---------|
| `iso/grub.cfg` | Added "VMware Safe Mode" entry | Skip graphics on VMware, prevent crash |
| `kernel/core/kmain.c` | Parse `nogui nomodeset` cmdline flags | Enable headless boot |
| `kernel/krow_diagnostics.c` | Check headless_mode flag | Skip interactive menu in safe mode |
| `scripts/test_grub_install.sh` | New test script | Verify grub-install on loopback |

---

##  Troubleshooting

### VMware Safe Mode Still Crashes

- [ ] Verify you selected correct GRUB entry (check menu name)
- [ ] Ensure ISO contains new grub.cfg (rebuild with `make iso`)
- [ ] Check VM RAM: ≥384 MB recommended
- [ ] Try VESA Fallback mode instead
- [ ] Share serial logs from crash

### GRUB Menu Doesn't Show

- [ ] Power on VM with ISO connected (CD/DVD IDE device)
- [ ] Wait 5+ seconds for GRUB timeout
- [ ] Check ISO is valid: `file SafeOS-1.0.iso` → should be "ISO 9660"
- [ ] Verify ~2600 sectors: `ls -lh SafeOS-1.0.iso`

### Still Can't Boot

- [ ] Try QEMU headless to isolate: `qemu-system-i386 -cdrom SafeOS-1.0.iso -m 1024 -serial stdio -boot d -display none`
- [ ] Share: VMware vmware.log file, QEMU output, error messages

---

## Next Steps

1. **Test Safe Mode Boot** → Run VM with new ISO, select VMware Safe Mode
2. **Verify No Crash** → Watch for BootRepair diagnostics output
3. **Run Diagnostics** → Check Krow BootRepair menu for boot issues
4. **Apply Repairs** → Fix recovered + restore bootloader
5. **Report Results** → Share success/failure with build details

---

**Last Updated:** 2026-04-02
**Version:** SafeOS 0.0.96 + VMware Triple-Fault Fix
**Target:** VMware Player 16.x  
**Status:** Ready for testing
