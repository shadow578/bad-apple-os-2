SHELL = /bin/bash
NAME = bad-apple
METALKIT_LIB = ./metalkit/lib
TARGET = $(NAME).img
LIB_MODULES = console console_vga timer intr keyboard bios vbe puff
APP_SOURCES = src/main.c test.raw.z.data.o

include $(METALKIT_LIB)/Makefile.rules

# extends original target to merge the .img bootloader and .elf kernel into one boot.img
# maybe i understood something wrong in the usage of metalkit, but this way the file is bootable with qemu and virtualbox
target:
# 	get size of .img and .elf file
# 	then merge the img and elf file aswell as however many bytes are needed to align to 512 bytes
# 	to the output file
	@echo "-- merge bootloader and kernel --"; \
	IMG_SIZE=$$(stat -c%s $(NAME).img); \
	ELF_SIZE=$$(stat -c%s $(NAME).elf); \
	PAD_BYTES=$$(( ($$IMG_SIZE + $$ELF_SIZE) % 512 )); \
	(dd if=$(NAME).img bs=512; \
	dd if=$(NAME).elf bs=512; \
	dd if=/dev/zero bs=$$PAD_BYTES count=1) > $(NAME)_boot.img; \
	echo "$(NAME)_boot.img size: $$(( $$IMG_SIZE + $$ELF_SIZE + $$PAD_BYTES )) bytes"

# build a floppy image compatible with virtualbox, etc...
# essentially just uses the merged image from target step and pads to 
# 1.44 or 2.88 MB
# $$(( 1474560 - ($$IMG_SIZE + $$ELF_SIZE) )); 
floppy: target
#	get the size of the boot.img file
# 	and pad it to a valid floppy size (1.474.560 or 2.949.120 bytes)
# 	use 2.88MB floppy if image does not fit 1.44MB
	@echo "-- pad floppy image --"; \
	IMG_SIZE=$$(stat -c%s $(NAME)_boot.img); \
	PAD_BYTES=$$(( 1474560 - $$IMG_SIZE)); \
	if [[ $$PAD_BYTES < 0 ]]; then \
		PAD_BYTES=$$(( 2949120 - $$IMG_SIZE)); \
		echo "image too big, fallback to 2.88MB floppy"; \
	fi;\
	if [[ $$PAD_BYTES < 0 ]]; then \
		echo "image larger than 2.88MB, wont fit floppy"; \
		exit 1; \
	fi; \
	(dd if=$(NAME)_boot.img bs=512; \
	dd if=/dev/zero bs=$$PAD_BYTES count=1) > $(NAME)_floppy.img

# extend clean to remove additional files
clean:
	rm -f $(TARGET) $(ELF_TARGET) $(LST_TARGET) $(NAME)_boot.img $(NAME)_floppy.img *.o 