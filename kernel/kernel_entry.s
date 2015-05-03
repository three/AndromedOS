.extern kmain
.global kentry

.text 0

kentry:
    cli
    call kmain
kentry_end:
    cli
    hlt
    jmp kentry_end
