# Requirements

This folder contains organized requirement lists for different components of the SafeOS project.

## Installation Guide

### System Packages
Install these via your OS package manager:

```bash
# Ubuntu/Debian
sudo apt install $(cat requirements-build.txt requirements-grub.txt requirements-qemu.txt requirements-iso.txt requirements-optional.txt | grep -v '^#' | tr '\n' ' ')

# macOS (using Homebrew)
brew install $(cat requirements-build.txt requirements-grub.txt requirements-qemu.txt requirements-iso.txt requirements-optional.txt | grep -v '^#' | tr '\n' ' ')
```

### Python Packages
```bash
pip install -r requirements-python.txt
```

## File Descriptions

- **requirements-build.txt** — Build tools, compilers, and development headers (make, GCC, NASM, etc.)
- **requirements-grub.txt** — GRUB bootloader utilities for creating bootable images
- **requirements-python.txt** — Python libraries (install via pip)
- **requirements-qemu.txt** — QEMU emulator for testing the kernel
- **requirements-iso.txt** — ISO image creation tools
- **requirements-optional.txt** — Optional utilities for testing and version control

## Quick Setup

For a complete development environment install all requirements:

```bash
# System packages (Linux example)
sudo apt install make nasm gcc pkg-config libc6-dev python3 python3-pip grub-pc-bin grub-efi-amd64-bin qemu-system-x86 xorriso git util-linux

# Python packages
pip install -r requirements/requirements-python.txt
```
