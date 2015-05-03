QEMU = qemu-system-i386
QEMUopts = -usb -hda andromedos.img
QEMUdebug = -S -s $(QEMUopts)

KOBJS = kernel/kernel_entry.o kernel/kernel_main.o

# Directives

build: andromedos.img
	echo "Build Complete!"

run: build
	$(QEMU) $(QEMUopts)

debug: build
	setsid $(QEMU) $(QEMUdebug) & true
	gdb -x gdbinit

clean:
	rm **.img || true
	rm **.log || true
	rm **.o   || true

# Bootloader

boot/boot.img: boot/boot1.img boot/boot2.img
	cat boot/boot1.img boot/boot2.img >boot/boot.img

boot/boot1.img: boot/boot1.nasm
	nasm -f bin -o boot/boot1.img boot/boot1.nasm

boot/boot2.img: boot/boot2.nasm
	nasm -f bin -o boot/boot2.img boot/boot2.nasm

# Kernel

kernel/kernel_entry.o: kernel/kernel_entry.s
	gcc -m32 -c kernel/kernel_entry.s -o kernel/kernel_entry.o

kernel/kernel_main.o: kernel/kernel_main.c
	gcc -m32 -c kernel/kernel_main.c -o kernel/kernel_main.o

kernel/kernel.img: kernel/kernel_link.ld $(KOBJS)
	ld -T kernel/kernel_link.ld -o kernel/kernel.img $(KOBJS)

# Final Processing

andromedos.img: boot/boot.img kernel/kernel.img
	cat boot/boot.img kernel/kernel.img >andromedos.img
