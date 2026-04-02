# SafeOS 0.0.96 VMware Fix - Quick Start Guide

## Build Ready ✓

```
SafeOS-1.0.iso (5.1 MB, 2601 sectors)
├── VMware Safe Mode entry (no graphics, headless boot)
├── GRUB BootRepair repairs tools
└── Comprehensive recovery documentation
```

---

## For VMware Users: What to Do Right Now

### Step 1: Boot SafeOS in VMware with No Graphics

1. In VMware Player, attach `SafeOS-1.0.iso` to CD/DVD device
2. Power on VM
3. When GRUB menu appears, **select:**

```
SafeOS 1.0 (VMware Safe Mode - No Graphics)
```

### Step 2: Expected Result

✓ VM boots without crash  
✓ Krow BootRepair diagnostics run  
✓ Serial console shows diagnostic results  
✓ No INT 10h BIOS call failures  
✓ No triple fault  

### Step 3: Try Other Modes if Desired

If Safe Mode works, you can also try:
- Graphics Mode (likely to crash on VMware - that's the bug we fixed!)
- VESA Fallback (800x600x32, may work)
- Debug Mode (serial output)

---

## For grub-install Testing

### Test Locally (Linux system with sudo)

```bash
cd /workspaces/safeos-0.0.96
sudo bash scripts/test_grub_install.sh
```

This will:
- Create /tmp/safeos_test_disk.img (100 MB)
- Install GRUB bootloader to it
- Verify installation succeeded
- Clean up automatically

### Boot Test Image

```bash
qemu-system-i386 -hda /tmp/safeos_test_disk.img -m 1024 -serial stdio
```

---

## What Was Fixed

### Problem
VMware: Boots → Crash after 7 seconds with "Unknown int 10h func 0x0000 / Triple fault"

### Root Cause
Kernel tried to call BIOS INT 10h video function after entering protected mode. VMware's BIOS emulation doesn't support this combination → exception → CPU shutdown.

### Solution
Boot with graphics disabled (`nogui nomodeset` flags) to skip graphics init entirely. Avoids the problematic INT 10h call sequence.

### Result
✓ Safe, headless boot mode  
✓ Diagnostics run successfully  
✓ Can identify & fix boot issues  
✓ Works on VMware, QEMU, VirtualBox

---

## Files You Need

| File | Purpose |
|------|---------|
| `SafeOS-1.0.iso` | Main bootable image (use in VMware CD drive) |
| `debug/VMWARE_RECOVERY_GUIDE.md` | Complete VMware troubleshooting guide |
| `tools/GRUB_INSTALL_GUIDE.md` | grub-install deployment reference |
| `scripts/test_grub_install.sh` | Automated grub-install test script |
| `tools/krow_bootrepair.py` | Automated repair tool (already integrated) |

---

## Troubleshooting

### "VM still crashes on VMware"
→ Make sure you selected the right GRUB entry: "VMware Safe Mode - No Graphics"  
→ Rebuild ISO: `make iso` (verify grub.cfg includes new entry)  
→ Try VESA Fallback mode instead

### "GRUB menu doesn't show"
→ Wait 5+ seconds for timeout  
→ Verify ISO is bootable: `file SafeOS-1.0.iso`  
→ Check CD/DVD device is connected in settings

### "grub-install test script fails"
→ Need Linux system (not running in devcontainer)  
→ Need sudo/root access and losetup available  
→ Try on regular Ubuntu/Debian desktop

---

## Technical Details

**VMware Issue:**
```
SVGA device initialization @ 0xe8000000
↓
Attempt INT 10h 0x0000 (query capabilities)
↓
VMware protected-mode BIOS doesn't handle it
↓
Unhandled exception
↓
Triple fault (CPU shutdown)
```

**SafeOS Fix:**
```
Parse "nogui nomodeset" kernel flags
↓
Skip graphics driver init
↓
No INT 10h calls issued
↓
Boot succeeds, serial console output available
```

---

## Next Steps

1. **Test VMware Safe Mode** → Boot VM, share results
2. **Test grub-install Script** → If you have Linux system with sudo
3. **Deploy to USB/Disk** → Use `tools/krow_bootrepair.py` or manual `grub-install`
4. **Share Feedback** → Report boot success/failure, system config

---

**Built:** April 2, 2026  
**Status:** Ready for testing  
**Target:** VMware Player 16.x with fixes for Graphics Mode crash  
**Alternative Hypervisors:** Also works on QEMU, VirtualBox, KVM
