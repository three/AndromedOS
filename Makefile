QEMU = qemu-system-i386
QEMUopts = -usb -drive file=andromedos.img,media=disk,format=raw
QEMUdebug = -S -s $(QEMUopts)

CC=i686-elf-gcc
CFLAGS=-m32 -g -ffreestanding -nostdlib -lgcc -Wall -Wextra

LD=i686-elf-ld

KOBJS=entry.o main.o memory.o terminal.o general.o
KDEPS=$(addprefix kernel/, $(KOBJS))

export PATH := deps/bin:$(PATH)

# Directives

build: andromedos.img
	echo "Build Complete!"

run: build
	$(QEMU) $(QEMUopts)

debug: build kernel/debug.elf
	setsid $(QEMU) $(QEMUdebug) & true
	gdb -x gdbinit

clean:
	find . -type f -regextype posix-extended -regex ".*\.(img|log|o)$$" -delete

# Bootloader

boot/boot.img: boot/boot1.img boot/boot2.img
	cat boot/boot1.img boot/boot2.img >boot/boot.img

boot/boot1.img: boot/boot1.nasm
	nasm -f bin -o boot/boot1.img boot/boot1.nasm

boot/boot2.img: boot/boot2.nasm
	nasm -f bin -o boot/boot2.img boot/boot2.nasm

# Kernel

%.o: %.s
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c kernel/*.h
	$(CC) -c -o $@ $< $(CFLAGS)

kernel/kernel.img: kernel/link.ld $(KDEPS)
	$(LD) -T kernel/link.ld -o kernel/kernel.img $(KDEPS)

# There's probably a better way to do this that takes into account the linker
# file, but this is good enough for now.
kernel/debug.elf: $(KDEPS)
	$(LD) -r -o kernel/debug.elf $(KDEPS)

# Final Processing

andromedos.img: boot/boot.img kernel/kernel.img scripts/postprocess.py
	cat boot/boot.img kernel/kernel.img >andromedos.img
	python3 scripts/postprocess.py andromedos.img
