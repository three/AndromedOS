set architecture i8086
target remote localhost:1234
add-symbol-file kernel/debug.elf 0x100000
break *0x100000
continue
