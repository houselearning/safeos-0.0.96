# BootRepair Debug Capture

This directory contains artifacts produced by the automated BootRepair pipeline.

Latest screenshots: `debug/*.png` (example: `debug/image-20260402-220913.png`)

What I ran

- Built the ISO:

```
make iso
```

- Packaged a recovery ISO that includes the host-side `tools/` directory:

```
python3 tools/krow_bootrepair.py package --recovery
```

- Ran a diagnosis (dry-run):

```
python3 tools/krow_bootrepair.py diagnose --target build/iso
```

- Booted the ISO under QEMU and captured the framebuffer (screendump -> PPM -> PNG):

```
bash scripts/capture_debug_screenshots.sh SafeOS-1.0.iso 90
```

Notes & findings

- The automated diagnosis reported `GRUB_CFG: OK`, `LIMINE_CFG: MISSING/DIFF`, and `SafeOS.bin: OK`.
- The kernel now accepts `debug=1`, `nogui`, or `nomodeset` on the multiboot command line and will skip the interactive BootRepair menu and GUI initialization when one of those flags is present. This helps boot in environments where the graphical initialization triggers crashes.
- The recovery ISO includes the `tools/krow_bootrepair.py` script which supports `diagnose`, `repair`, `rollback`, and `package` subcommands. Use `--apply` to actually write files; by default it performs a dry-run.

How to reproduce locally

1. Build the ISO:

```
make iso
```

2. Boot and capture a screenshot (headless):

```
bash scripts/capture_debug_screenshots.sh SafeOS-1.0.iso 90
```

3. If a host image needs repair (raw `.img`), create a backup and run repair (requires `sudo` for loopback and grub-install):

```
sudo python3 tools/krow_bootrepair.py repair --target /path/to/image.img --mount-image --install-bootloader --apply
```

If you want me to attempt further automated fixes (e.g., apply repairs to the ISO tree, attempt a grub-install to a loopback image, or try alternate framebuffer modes), tell me which you'd like me to run next.
