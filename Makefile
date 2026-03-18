TARGET   = SafeOS.bin
ISO_DIR  = build/iso
KERNEL   = $(TARGET)

CC       = i686-elf-gcc
LD       = i686-elf-ld
AS       = i686-elf-as

CFLAGS   = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Ikernel
LDFLAGS  = -T linker.ld -nostdlib

SRC_C = $(shell find kernel -name '*.c')
SRC_S = $(shell find kernel -name '*.s')
OBJ_C = $(SRC_C:.c=.o)
OBJ_S = $(SRC_S:.s=.ao)
OBJ   = $(OBJ_C) $(OBJ_S)

# ============================================================================
# KROW DIAGNOSTICS CONFIGURATION
# ============================================================================

KROW_BINARY=kernel/krow_diagnostics
KROW_SOURCE=kernel/krow_diagnostics.c
KROW_PYTHON=tools/kernel-krow.py
KROW_INIT=scripts/S01krow

KROW_CFLAGS=-Wall -Wextra -std=c99 -O2 -fPIC
KROW_LDFLAGS=

.PHONY: all disk install-disk iso grub clean
.PHONY: krow krow-clean krow-install krow-uninstall krow-strip krow-run krow-help

all: iso

disk:
	@mkdir -p build
	./scripts/create_disk.sh safeos_disk.img 1G

install-disk: $(KERNEL) disk
	@echo "Installing kernel to disk image (non-bootable)."
	./scripts/install_to_disk.sh safeos_disk.img $(KERNEL)

$(KERNEL): $(OBJ)
	# Ensure the boot object is linked first so the multiboot header is within
	# the first 8 KiB of the output file.
	$(LD) $(LDFLAGS) -o $@ kernel/arch/x86/boot.ao $(filter-out kernel/arch/x86/boot.ao,$(OBJ))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.ao: %.s
	$(AS) $< -o $@

# ============================================================================
# KROW DIAGNOSTICS TARGETS
# ============================================================================
# Note: Krow Diagnostics is compiled as part of the main kernel build since
# it uses kernel headers. The targets below are for verification and utilities.

# Verify Krow Diagnostics source exists
krow:
	@echo "[*] Krow Diagnostics source check..."
	@if [ -f "$(KROW_SOURCE)" ]; then \
		echo "[OK] Found: $(KROW_SOURCE)"; \
		echo "[*] Krow will be compiled as part of kernel build"; \
		echo "[*] Run 'make iso' or 'make $(KERNEL)' to build"; \
	else \
		echo "[ERROR] $(KROW_SOURCE) not found"; \
		exit 1; \
	fi

# Verify object file was created
krow-check: $(KERNEL)
	@echo "[*] Checking Krow compilation..."
	@if grep -q "krow_diagnostics" $(KERNEL); then \
		echo "[OK] Krow Diagnostics is in kernel binary"; \
	else \
		echo "[WARN] Krow may not be linked yet"; \
	fi

# Clean Krow build artifacts (object files)
krow-clean:
	@echo "[*] Cleaning Krow Diagnostics build artifacts..."
	@rm -f kernel/core/krow_diagnostics.o
	@rm -f $(KROW_SOURCE:.c=.o)
	@echo "[OK] Krow cleaned"

# Install Python error handler to rootfs
krow-install: 
	@echo "[*] Installing Krow Diagnostics components to rootfs..."
	@if [ -z "$(ROOTFS)" ]; then \
		echo "[ERROR] ROOTFS not set. Usage: make krow-install ROOTFS=<path>"; \
		exit 1; \
	fi
	@mkdir -p $(ROOTFS)/usr/bin
	@mkdir -p $(ROOTFS)/etc/init.d
	@mkdir -p $(ROOTFS)/var/log
	@cp $(KROW_PYTHON) $(ROOTFS)/usr/bin/kernel-krow.py
	@chmod +x $(ROOTFS)/usr/bin/kernel-krow.py
	@cp scripts/S01krow $(ROOTFS)/etc/init.d/S01krow
	@chmod +x $(ROOTFS)/etc/init.d/S01krow
	@echo "[OK] Krow components installed to $(ROOTFS)"

# Uninstall from rootfs
krow-uninstall:
	@echo "[*] Uninstalling Krow Diagnostics..."
	@if [ -z "$(ROOTFS)" ]; then \
		echo "[ERROR] ROOTFS not set. Usage: make krow-uninstall ROOTFS=<path>"; \
		exit 1; \
	fi
	@rm -f $(ROOTFS)/usr/bin/kernel-krow.py
	@rm -f $(ROOTFS)/etc/init.d/S01krow
	@echo "[OK] Krow uninstalled"

# Help target
krow-help:
	@echo "Krow Diagnostics Build Targets:"
	@echo "  make krow              - Verify Krow source present"
	@echo "  make krow-check        - Check if Krow is in kernel binary"
	@echo "  make krow-clean        - Clean Krow build artifacts"
	@echo "  make krow-install      - Install to rootfs (ROOTFS=<path>)"
	@echo "  make krow-uninstall    - Uninstall from rootfs (ROOTFS=<path>)"
	@echo ""
	@echo "Integration:"
	@echo "  - Krow source: kernel/core/krow_diagnostics.c"
	@echo "  - Added to kernel build automatically"
	@echo "  - Call krow_diagnostics_run() from kmain.c"
	@echo "  - See KROW_INTEGRATION.md for details"
	@echo "  - Python handler: tools/kernel-krow.py"

# ============================================================================
# OS BUILD TARGETS
# ============================================================================

iso: $(KERNEL) grub
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/SafeOS.bin
	cp iso/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o SafeOS-1.0.iso $(ISO_DIR)

grub:
	mkdir -p iso

clean: krow-clean
	rm -rf $(OBJ) $(KERNEL) build SafeOS-1.0.iso
