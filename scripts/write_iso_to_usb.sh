#!/usr/bin/env bash
set -euo pipefail

# Write SafeOS ISO to USB device (destructive). Use with caution.
# Usage: sudo ./write_iso_to_usb.sh /path/to/SafeOS-1.0.iso /dev/sdX

ISO=${1:-SafeOS-1.0.iso}
DEV=${2:-}

if [ -z "$DEV" ]; then
  echo "Usage: sudo $0 /path/to/SafeOS.iso /dev/sdX"
  exit 2
fi

if [ ! -f "$ISO" ]; then
  echo "ISO not found: $ISO" >&2
  exit 1
fi

read -p "About to write $ISO to $DEV — ALL DATA ON $DEV WILL BE LOST. Continue? (yes/NO) " ans
if [ "$ans" != "yes" ]; then
  echo "Aborted. Answer 'yes' to proceed."; exit 1
fi

echo "Writing ISO to $DEV with dd (this will take a while)..."
dd if="$ISO" of="$DEV" bs=4M status=progress conv=fdatasync
sync
echo "Done. You can now try booting from the USB device." 
