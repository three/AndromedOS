.extern kmain
.global kentry

.text 0

kentry:
    cli
    mov $0x80000, %ebp
    mov $0x80000, %esp
    call kmain
kentry_end:
    cli
    hlt
    jmp kentry_end
