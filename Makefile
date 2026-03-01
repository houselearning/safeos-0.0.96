TARGET   = SafeOS.bin
ISO_DIR  = build/iso
KERNEL   = $(TARGET)

CC       = i686-elf-gcc
LD       = i686-elf-ld
AS       = i686-elf-as

CFLAGS   = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  = -T linker.ld -nostdlib

SRC_C = $(shell find kernel -name '*.c')
SRC_S = $(shell find kernel -name '*.s')
OBJ   = $(SRC_C:.c=.o) $(SRC_S:.s=.o)

all: iso

$(KERNEL): $(OBJ)
	# Ensure the boot object is linked first so the multiboot header is within
	# the first 8 KiB of the output file.
	$(LD) $(LDFLAGS) -o $@ kernel/arch/x86/boot.o $(filter-out kernel/arch/x86/boot.o,$(OBJ))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $< -o $@

iso: $(KERNEL) grub
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/SafeOS.bin
	cp iso/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o SafeOS-1.0.iso $(ISO_DIR)

grub:
	mkdir -p iso

clean:
	rm -rf $(OBJ) $(KERNEL) build SafeOS-1.0.iso
