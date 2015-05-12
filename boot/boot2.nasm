;
; boot2.nasm
;

ORG 0x800
BITS 16

start:
    mov esp, 0x80000 ;Setup stack

    ; Let user know Boot2 has successfully loaded
    mov si, bootWelcome
    call Print

    ; Turn on A20
    call FlipA20
    jnc .A20success
    mov si, error_A20
    call Print
    jmp Hang
.A20success:

    ; Detect Memory
    call DetectUpperMemory
    jnc .MemSuccess
    mov si, error_memory
    call Print
    jmp Hang
.MemSuccess:

    ; Load GDT
    call LoadTempGDT
    jnc .GDTSuccess
    mov si, error_GDT
    call Print
    jmp Hang
.GDTSuccess:

    ; Enter Protected Mode
    jmp EnterProtected ;After entering protected, will jump to start_32

Hang:
    cli
    hlt
    jmp Hang

Print:
    ; Prints message to screen
    ;  Store string pointer in SI
    ;  String must be null-terminated
    ;  Does not check if message will run off bottom of screen

    mov bl, 0Fh ; White on Black
    mov cx, 1
    mov dh, [.CurrentLine]
    xor dl, dl
    xor bh, bh
.print_loop:
    ; Set cursor position
    mov ah, 02h
    int 10h

    ; Write Character
    mov ah, 09h
    mov al, [si]
    int 10h

    inc si
    inc dl
    cmp dl, 80
    jne .checknull
    inc dh
    xor dl, dl
.checknull:
    cmp byte [si], 0
    jne .print_loop

    inc dh
    mov [.CurrentLine], dh
    ret
.CurrentLine db 0

FlipA20:
    ; Sets the A20 gate using the BiOS interrupt
    ;  No input
    ;  Carry flag set on error. BiOS errorcode in AH.
    mov ax, 0x2401
    int 15h ;BiOS will already set carry flag on error and set AH.
    ret

DetectUpperMemory:
    ; Detect Upper Memory using BiOS interrupt
    ;  No input
    ;  Stores memory list at 0x1000 (up to 0x2000)
    ;  Carry flag set on error, BiOS errorcode in AH.

    ; Initialization
    xor ebx, ebx
    mov edx, 0x534D4150
    mov eax, 0xE820
    mov ecx, 0x18 ;24 bytes
    xor bp, bp ;Entry count
    mov di, 0x30000 ;Buffer
    clc

    ; First Call
    int 0x15
    jc .failMem
    cmp eax, 0x534D4150
    jne .failMem

    ; Subsequent Calls
.call_e820:
    mov eax, 0xe820
    mov ecx, 0x18
    inc bp
    add di, 0x18
    int 0x15
    jc .done

.done:
    clc
    ret

.failMem:
    stc
    ret

LoadTempGDT:
    ; Sets up a temporary GDT before (necessary before entering protected mode)
    ;  Sets carry flag if error (should never happen)
    lgdt [gdt_desc]
    clc
    ret

EnterProtected:
    ; Enters protected mode and jumps to start_32
    cli
    mov eax, cr0
    or ax, 1
    mov cr0, eax
    jmp (gdt_code-gdt):start_32

; Data
bootWelcome db "Boot 2 loaded.",0

error_A20 db "Failed to turn on A20.",0
error_memory db "Failed to properly detect memory",0
error_GDT db "Failed to load GDT",0

align 4
gdt:
gdt_null:
    dq 0
gdt_code:
    dw 0xFFFF, 0
    db 0x0, 0x9A, 0xCF, 0x0
gdt_data:
    dw 0xFFFF, 0
    db 0x0, 0x92, 0xCF, 0x0
gdt_end:

align 4
gdt_desc:
    dw (gdt_end - gdt) - 1
    dd gdt

BITS 32

start_32:
    mov dword [0xB809C], 0xF032F033 ; Print "32" at top right corner

    ; Copy Kernel to its proper place
    mov cx, 0x1000
    mov esi, 0x1000
    mov edi, 0x100000
    rep movsd

    jmp long 0x100000
    jmp Hang_32

Hang_32:
    cli
    hlt
    jmp Hang_32

times 2048 - ($-$$) db 0
