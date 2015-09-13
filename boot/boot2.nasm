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
    call LoadGDT
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
    ; Detect Upper Memory using e820 BIOS call
    ;  No input
    ;  Stores memory list at 0x30000 (not past 0x40000)
    ; Carry flag set on error, BIOS errorcode in AH

    ; Initialization
    xor ebx, ebx
    mov ax, 0x3000
    mov es, ax
    xor di, di

.call_e820:
    ; Set registers (assume these are trashed)
    mov edx, 0x534D4150 ; Magic Number
    mov eax, 0xe820 ; BIOS code
    mov ecx, 24 ; Always ask for 24 bytes

    ; Zero where memory map should go
    mov dword [es:di],    0
    mov dword [es:di+4],  0
    mov dword [es:di+8],  0
    mov dword [es:di+12], 0
    mov dword [es:di+16], 0
    mov dword [es:di+20], 1 ; Ensure valid ACPI code

    ; Clear Carry Flag
    clc

    ; Make BIOS call
    int 15h

    ; If carry flag is set, function is done
    ;  (this could be an error, eax must be checked)
    jc .done

    ; If EBX == 0, function completed successfully
    test ebx, ebx
    je .done

    ; Make sure the count is not too large
    add di, 24
    cmp bp, 1300
    jg .large

    ; Do it again
    jmp .call_e820

.large:
    ; Set "too large" flag
    mov eax, [0x38000]
    or eax, 0x1
    mov [0x38000], eax

    ; return as "success" (let kernel deal with it)
    clc
    ;jmp .done ; Let it fall through

.done:
    ; If eax == Magic, function exited successfully
    cmp eax, 0x534D4150
    jne .fail
    clc
    ret

.fail:
    ; Error code will already be in ah from BIOS
    ; Call count will be in bp
    stc
    ret

LoadGDT:
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

    ; Setup 32-bit segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    jmp long 0x100000
    jmp Hang_32

Hang_32:
    cli
    hlt
    jmp Hang_32

times 2048 - ($-$$) db 0
