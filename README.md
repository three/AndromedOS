# AndromedOS

AndromedOS is a hobby Operating System named after the Andromeda Galaxy. It is
made to run on the i386 architecture and is currently tested on QEMU.

AndromedOS does not support multiboot, and currently can only be loaded via
the built-in bootloader.

This project is released under the MIT License. See file `LICENSE` for more
details.

## Building and Running

AndromedOS makes use of the following dependencies:

- Netwide Assembler (nasm, 2.10.09 tested)
- GNU Compiler Collection (gcc, 5.1.0 tested, see cross-compilation)
- GNU Binutils (binutils, 2.24 tested, see cross-compilation)
- QEMU i386 Emulator (2.0.0 tested, used for running/testing)
- GNU Debugger (gdb, 7.7.1 tested, used for testing)
- GNU Make (make, 3.81 tested)
- An Unix Environment

Use `make build` to build the OS and `make run` to build and run it. OS will be
built as `andromedos.img`.

Use `make debug` to start QEMU and a gdb instance that will automatically
connect to QEMU and break after the BIOS initializes.

### Cross Compilation

When using make to build AndromedOS, make will add `./deps/bin` to the beginning
of its `PATH`. Instead of normal GCC and Binutil executables, these command will
be prefixed with `i686-elf-`. This setup allows you to use a cross compiler to
build AndromedOS without affecting the rest of your system. If you want to use a
compiler already on your system of a different name, make symbolic links to the
correct destinations.

## Contributing

AndromedOS is a personal project with all code written solely by me. I am not
accepting contributions.
