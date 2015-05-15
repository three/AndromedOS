QEMU = qemu-system-i386
QEMUopts = -usb -hda andromedos.img
QEMUdebug = -S -s $(QEMUopts)

CC=gcc
CFLAGS=-m32 -ffreestanding -nostdlib -lgcc -nostdinc -Wall

KOBJS=entry.o main.o memory.o
KDEPS=$(addprefix kernel/, $(KOBJS))

# Directives

build: andromedos.img
	echo "Build Complete!"

run: build
	$(QEMU) $(QEMUopts)

debug: build
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

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

kernel/kernel.img: kernel/link.ld $(KDEPS)
	ld -T kernel/link.ld -o kernel/kernel.img $(KDEPS)

# Final Processing

andromedos.img: boot/boot.img kernel/kernel.img
	cat boot/boot.img kernel/kernel.img >andromedos.img
