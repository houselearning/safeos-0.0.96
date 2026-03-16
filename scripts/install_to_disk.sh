#!/usr/bin/env bash
set -e
IMG=${1:-safeos_disk.img}
KERNEL=${2:-SafeOS.bin}

if [ ! -f "$IMG" ]; then
  echo "Disk image $IMG not found. Run scripts/create_disk.sh first or provide the image path."
  exit 1
fi
if [ ! -f "$KERNEL" ]; then
  echo "Kernel $KERNEL not found. Build first."
  exit 1
fi

echo "Writing $KERNEL to start of $IMG (non-bootable raw write)."
# This writes the kernel to the beginning of the disk image. It does NOT
# install a bootloader. To make the disk bootable you should install GRUB
# to the image using 'grub-install' on the host with loopback devices.
sudo dd if="$KERNEL" of="$IMG" conv=notrunc bs=512 seek=1

echo "Kernel copied to $IMG. To make the disk bootable, mount the image and run grub-install, e.g. (host):"
echo "  sudo losetup -Pf --show $IMG"
echo "  sudo mount /dev/loopXp1 /mnt"
echo "  sudo grub-install --boot-directory=/mnt/boot /dev/loopX"
echo "(Replace loopX with the actual loop device name returned by losetup.)"
