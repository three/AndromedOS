;
; boot1.nasm
;  Stage 1 of boot process. Loads stage 2 into 0x600.
;

ORG 0x7C00
BITS 16

; This stage tries not to use the stack

jmp 0:start

; Data

error db "Bootloader Stage 1 failed. Error Letter: "
errorletter db 0
error2 db ". Error Code: 0x"
errorcode dw 0
error3 db ".",0

drive db 0

diskpacket:
    db 0x10, 0 ; Header
    dw 0x10 ; 16 sectors
    dw 0x600
    dw 0x0
    dq 0

; Text
;  Starts at 0x7D00 to make debugging more convenient
times 256 - ($-$$) db 0

start:
    ; Store any registers we want to keep
    mov byte [drive], dl

    ; If drive is 0, set to 0x80
    cmp byte [drive], 0
    jne .regc
    mov byte [drive], 0x80
.regc:

    ; Reset segment registers
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    jmp load_stage2

fail:
    ; Setup error code (stored in ah)
    ; Error letter (stored in al). followed by hexified error code will print
    mov [errorletter], al

    mov al, ah
    and ax, 0x0FF0
    shr al, 4

    add ah, 48
    add al, 48

    cmp al, 58
    jl .check1
    add al, 7
.check1:
    cmp ah, 58
    jl .check2
    add ah, 7
.check2:

    mov [errorcode], ax
.fail_init:
    ; Start printing
    mov bl, 4Fh ; White on Red
    mov cx, 1
    xor dh, dh
    xor dl, dl
    xor si, si
    xor bh, bh
.fail_loop:
    ; Set cursor position (starts 0,0)
    mov ah, 02h
    int 10h

    ; Write charater
    mov ah, 09h
    mov al, [error+si]
    int 10h

    inc si
    inc dl
    cmp byte [error+si], 0
    jne .fail_loop
hang:
    cli
    hlt
    jmp hang

load_stage2:
    ; Copy the first 5 sectors of the disk (boot1+boot2) to 0x600

    ; Check INT13 extensions
    mov ah, 41h
    mov dl, [drive]
    mov bx, 0x55AA
    int 13h
    mov ah, [drive]
    mov al, 'D' ;Error Letter D
    jnc .lba

    ; Reset Disk
    xor ax, ax
    mov dl, 0x80
    int 13h
    inc dl
    mov ah, 0x80
    mov al, 'E' ;Error Letter E
    cmp cl, 0
    je fail

.chs:
    ; CHS
    mov ah, 02h
    mov al, 15
    mov ch, 0
    mov cl, 1
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x600
    int 13h
    mov al, 'C' ;Error Letter C
    jc fail
    jmp check_stage2

.lba:
    ; LBA
    mov ah, 42h
    mov dl, [drive]
    mov si, diskpacket
    int 13h
    mov al, 'A' ;Error Letter A
    jc fail

check_stage2:
    ; If the disk was read correctly then memory 0x7d00-0x7e00 (text from
    ; boot1) should be the same as 0x700-0x800. (0x7c00-0xd00 contains writable
    ; variables)
    mov si, 0x700
    mov ax, 0x0142 ;Error letter B, code 0x01
.check_loop:
    mov ebx, [si+0x7600]
    cmp ebx, [si]
    jne fail

    add si, 4
    cmp si, 0x800
    jne .check_loop

enter_stage2:
    jmp 0:0x800

times 510 - ($-$$) db 0
dw 0xAA55
