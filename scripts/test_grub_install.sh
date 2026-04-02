#!/bin/bash
#
# test_grub_install.sh - Test grub-install on a loopback image
# Run with: sudo bash scripts/test_grub_install.sh
#
# This script:
# 1. Creates a 100MB test disk image
# 2. Formats it with FAT32 (BIOS-compatible)
# 3. Mounts it as loopback
# 4. Installs GRUB bootloader
# 5. Verifies the installation
# 6. Cleans up
#

set -e

TEST_IMAGE="/tmp/safeos_test_disk.img"
TEST_MOUNT="/tmp/safeos_test_mnt"
REPO_ROOT="/workspaces/safeos-0.0.96"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== SafeOS grub-install Loopback Test ===${NC}"
echo "Image: $TEST_IMAGE"
echo "Mount: $TEST_MOUNT"
echo ""

# Step 1: Create test disk image
echo -e "${YELLOW}[1/5] Creating 100MB test disk image...${NC}"
dd if=/dev/zero of="$TEST_IMAGE" bs=1M count=100 status=progress
echo -e "${GREEN}✓ Image created${NC}"

# Step 2: Create MBR and ext4 partition (BIOS-compatible)
echo -e "${YELLOW}[2/5] Setting up MBR and partition...${NC}"
# Create a simple MBR with fdisk
{
  echo "o"      # Create new MBR label
  echo "n"      # New partition
  echo "p"      # Primary partition
  echo "1"      # Partition number 1
  echo ""       # Accept default first sector
  echo ""       # Accept default last sector (full disk)
  echo "a"      # Make bootable
  echo "1"      # Partition 1
  echo "t"      # Set partition type
  echo "83"     # Linux type (ext4 supported)
  echo "w"      # Write and exit
} | fdisk "$TEST_IMAGE" 2>/dev/null || true

# Setup loop device for partition
LOOP_DEV=$(losetup -f)
losetup -P "$LOOP_DEV" "$TEST_IMAGE" 2>/dev/null || losetup "$LOOP_DEV" "$TEST_IMAGE"

# Format first partition with ext4
PARTITION="${LOOP_DEV}p1"
if [ ! -b "$PARTITION" ]; then
    PARTITION="${LOOP_DEV}1"
fi

echo "Formatting partition (may try multiple methods)..."
mkfs.ext4 "$PARTITION" 2>/dev/null || {
    echo -e "${YELLOW}Trying ext2 format...${NC}"
    mkfs.ext2 "$PARTITION" 2>/dev/null || {
        echo -e "${YELLOW}Trying ext3 format...${NC}"
        mkfs.ext3 "$PARTITION" 2>/dev/null
    }
}

losetup -d "$LOOP_DEV"
echo -e "${GREEN}✓ Partition formatted${NC}"

# Step 3: Mount the image
echo -e "${YELLOW}[3/5] Mounting image as loopback...${NC}"
mkdir -p "$TEST_MOUNT"

# Create loop device with partitions
LOOP_DEV=$(losetup -f)
losetup -P "$LOOP_DEV" "$TEST_IMAGE" 2>/dev/null || losetup "$LOOP_DEV" "$TEST_IMAGE"

# Find the partition device
PARTITION="${LOOP_DEV}p1"
if [ ! -b "$PARTITION" ]; then
    PARTITION="${LOOP_DEV}1"
fi
if [ ! -b "$PARTITION" ]; then
    # If partition doesn't exist, use loop device directly
    PARTITION="$LOOP_DEV"
fi

# Mount the partition
mount "$PARTITION" "$TEST_MOUNT" 2>/dev/null || {
    # Try alternative mount options
    mount -o loop "$TEST_IMAGE" "$TEST_MOUNT" 2>/dev/null || {
        echo -e "${RED}✗ Failed to mount loopback device${NC}"
        losetup -d "$LOOP_DEV" 2>/dev/null
        exit 1
    }
}
echo -e "${GREEN}✓ Mounted: $PARTITION → $TEST_MOUNT${NC}"

# Step 4: Install GRUB bootloader
echo -e "${YELLOW}[4/5] Installing GRUB bootloader...${NC}"
mkdir -p "$TEST_MOUNT/boot/grub"
# Copy kernel and GRUB config
if [ -f "$REPO_ROOT/SafeOS.bin" ]; then
    cp "$REPO_ROOT/SafeOS.bin" "$TEST_MOUNT/boot/"
    echo -e "${GREEN}✓ SafeOS.bin copied${NC}"
fi
if [ -f "$REPO_ROOT/iso/grub.cfg" ]; then
    cp "$REPO_ROOT/iso/grub.cfg" "$TEST_MOUNT/boot/grub/"
    echo -e "${GREEN}✓ grub.cfg copied${NC}"
fi

# Install GRUB to the loopback device (MBR)
echo "Running grub-install on $LOOP_DEV..."
grub-install \
    --target=i386-pc \
    --boot-directory="$TEST_MOUNT/boot" \
    --no-floppy \
    --recheck \
    "$LOOP_DEV" \
    && echo -e "${GREEN}✓ GRUB installed successfully${NC}" \
    || echo -e "${RED}✗ grub-install failed${NC}"

# Step 5: Verify installation
echo -e "${YELLOW}[5/5] Verifying GRUB installation...${NC}"
if [ -f "$TEST_MOUNT/boot/grub/i386-pc/core.img" ]; then
    echo -e "${GREEN}✓ core.img found${NC}"
fi
if [ -f "$TEST_MOUNT/boot/grub/grub.cfg" ]; then
    echo -e "${GREEN}✓ grub.cfg found${NC}"
fi
if [ -f "$TEST_MOUNT/boot/SafeOS.bin" ]; then
    SIZE=$(stat -f%z "$TEST_MOUNT/boot/SafeOS.bin" 2>/dev/null || stat -c%s "$TEST_MOUNT/boot/SafeOS.bin")
    echo -e "${GREEN}✓ SafeOS.bin found (${SIZE} bytes)${NC}"
fi

echo ""
echo -e "${YELLOW}Listing boot directory:${NC}"
ls -lh "$TEST_MOUNT/boot/"
echo ""
ls -lh "$TEST_MOUNT/boot/grub/" 2>/dev/null || echo "  (no grub dir)"

# Cleanup
echo -e "${YELLOW}Cleaning up...${NC}"
# Unmount if needed
if mountpoint -q "$TEST_MOUNT"; then
    umount "$TEST_MOUNT" 2>/dev/null || umount -l "$TEST_MOUNT" 2>/dev/null || true
fi
# Detach loop device if it was created
if losetup "$LOOP_DEV" &>/dev/null; then
    losetup -d "$LOOP_DEV" 2>/dev/null || true
fi
rm -rf "$TEST_MOUNT" 2>/dev/null || true
echo -e "${GREEN}✓ Cleanup complete${NC}"

# Summary
echo ""
echo -e "${GREEN}=== Test Complete ===${NC}"
echo "Test image available at: $TEST_IMAGE"
echo "To boot this image in QEMU:"
echo "  qemu-system-i386 -hda $TEST_IMAGE -m 1024 -serial stdio"
echo "To clean up:"
echo "  rm $TEST_IMAGE"
