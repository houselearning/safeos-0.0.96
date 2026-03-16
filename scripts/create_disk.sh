#!/usr/bin/env bash
set -e
IMG=${1:-safeos_disk.img}
SIZE=${2:-1G}

echo "Creating disk image $IMG size $SIZE"
if command -v qemu-img >/dev/null 2>&1; then
  qemu-img create -f raw "$IMG" "$SIZE"
else
  # fallback: use dd to create raw filled with zeros
  bytes=$(echo "$SIZE" | awk '/G/{print $1*1073741824} /M/{print $1*1048576} !/M|G/{print $1}')
  if [ -z "$bytes" ]; then
    bytes=1073741824
  fi
  dd if=/dev/zero of="$IMG" bs=1 count=0 seek=$bytes
fi

echo "Disk image $IMG created"