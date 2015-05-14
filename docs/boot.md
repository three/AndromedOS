# Booting

The first 2560 bytes of the final `andromedos.img` file are dedicated to the
bootloader. The ultimate job of the bootloader is to ensure the environment
is correct for the kernel and to load the kernel at the address of 0x100000.

The bootloader is made of 2 stages that work together to load the kernel. Files
relating the bootloader are in the `/boot/` directory.

## Memory

Before going into the individual boot stages it is first necessary to understand
how the memory is laid out and managed.

This code is based on the assumption that the memory from 0x500 to 0x80000 is
guaranteed to be free in 16-bit mode (with the knowledge that boot1 will take up
0x7C00-0x7DFF).

Boot1 is initially loaded into 0x7C00 and ends at 0x7DFF. Boot1 will load the
first 0x2FA00 bytes from the disk into address 0x600 (possibly less if contents
of disk are smaller). This means the section of disk containing memory ends at
0x2FFFF. 0x500-0x5FF is reserved for bootloader communication and data.
0x30000-0x40000 is reserved for bootloader-kernel communication (eg. memory
map). See below for chart.

    0x0       Unusable in real mode
    0x500     Bootloader Data and Communication
    0x600     Hard drive contents
      0x600     Boot1
      0x800     Boot2
      0x1000    Kernel
    0x30000   Bootloader-Kernel Communication
    0x40000   Bootloader stack
    0x80000   End of usable memory in read mode

Note the contents of the hard drive overwrites where boot1 is initially loaded.
Because of this, boot1 must copy itself somewhere inside the stack (boot1 does
not use the stack so this is not an issue) and continue running from there.
*NOTE: This behaviour is currently not implemented as the kernel is small enough
where this is not necessary. This must be implemented in the future, once the
kernel becomes large enough.*

The hard drive is never read from again after boot1 and the kernel is copied to
its final location, 0x100000, from 0x1000.

## boot1

The first stage of the bootloader is located in `/boot/boot1.nasm`. This stage,
512 bytes long is loaded and started from address 0x7C00 by the BIOS. This stage
accomplishes the following tasks:

1. Normalize CS and code pointer
2. Load contents of disk
3. Ensure everything completed without errors (display a message if not)
4. Jump to Stage 2

### Error Handling

When an error occurs the `ax` register must be set to the proper error value and
a jump to `fail` must be made. See code for more details.

## boot2

The second stage of the bootloader is located in `/boot/boot2.nasm`. This stage
is 2048 bytes long and is loaded and started from address 0x800 from boot1. This
stage accomplishes the following tasks:

1. Sets up the stack starting at 0x80000
2. Prints a message notifying the user boot2 has successfully loaded
3. Turns on the A20 gate, allowing for higher memory access
4. Ask BIOS for a map of upper memory.
5. Check memory map to make sure it is safe to load kernel (Not Implemented)
6. Load a GDT
7. Enter protected mode
8. Notify user we have entered 32-bit mode ("32" printed on top-right of screen)
9. Copy kernel to its proper location
10. Ensure kernel was loaded successfully (Not Implemented)
11. Enter Kernel

### Error Handling

In real mode, after each function call, CF is checked to see if there was an
error. If so a message is printed and `Hang` is called.

There is no error handling for boot2 in protected mode.
