# HOWTO: Use requirements.txt in SafeOS

This guide explains how to use the repository-level `requirements.txt` for building and running SafeOS tools.

## 1) Location
- `requirements.txt` is at the repo root.
- It contains both OS package names and Python package names.

## 2) Install System Dependencies
### Debian/Ubuntu
```bash
sudo apt update
sudo apt install -y make nasm gcc pkg-config xorriso grub-pc-bin grub-efi-amd64-bin qemu-system-x86 libc6-dev python3 python3-pip git util-linux
```

### Fedora/RHEL
```bash
sudo dnf install -y make nasm gcc pkg-config xorriso grub2-tools qemu-system-x86 python3 python3-pip git util-linux
```

### macOS (Homebrew)
```bash
brew install nasm pkg-config qemu xorriso python git
```

### Windows (WSL recommended)
- Use Ubuntu/Mint shell and follow Debian instructions.
- If native Windows, install Chocolatey and packages (make, nasm, qemu, python, git).

## 3) Install Python Requirements
```bash
python3 -m pip install --upgrade pip
python3 -m pip install -r requirements.txt
```

## 4) Verify Build Tools
```bash
make --version
qemu-system-x86_64 --version
python3 --version
```

## 5) Build + Test
```bash
make clean
make iso
make krow-livedn force
```

## 6) Troubleshooting
- Missing package: install with distro package manager, or adapt `requirements.txt` for your platform.
- Inconsistent Python version: use `python3` not `python`.
- `qemu` headless/GUI mode: add `-display gtk` or `-display sdl` as needed.

## 7) See Also
- `README/SEE_ALSO.md` (index of moved readme docs)
- `README/KROW_README.md` (Krow-specific project guide)
