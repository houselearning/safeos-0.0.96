#!/usr/bin/env bash
set -euo pipefail

# Simple packaging helper for SafeOS
# - build ISO using project's Makefile
# - create raw disk image and convert to VMDK/VDI with qemu-img

ISO=${1:-SafeOS-1.0.iso}
RAW_IMG=${2:-safeos_disk.img}
SIZE=${3:-1G}

echo "Building ISO..."
if make iso; then
  echo "ISO build complete"
else
  echo "make iso failed; ensure build dependencies are installed" >&2
  exit 1
fi

if [ ! -f "$ISO" ]; then
  # copy from build/iso if produced there
  if [ -f build/iso/SafeOS.bin ]; then
    echo "Note: ISO not found at $ISO; you may find SafeOS.bin in build/iso/" 
  fi
fi

echo "Creating raw disk image: $RAW_IMG ($SIZE)"
if command -v qemu-img >/dev/null 2>&1; then
  qemu-img create -f raw "$RAW_IMG" "$SIZE"
else
  dd if=/dev/zero of="$RAW_IMG" bs=1 count=0 seek=$SIZE
fi

echo "To install SafeOS onto the raw image, run:"
echo "  sudo scripts/install_to_disk.sh $RAW_IMG SafeOS.bin"
echo "To convert raw image to VMDK:" 
echo "  qemu-img convert -f raw -O vmdk $RAW_IMG safeos_disk.vmdk"
echo "To convert raw image to VDI:" 
echo "  qemu-img convert -f raw -O vdi $RAW_IMG safeos_disk.vdi"
echo "NOTE: For a fully bootable disk you still need to install a bootloader into the image (use grub-install on the host with loopback). See scripts/install_to_disk.sh for guidance."
