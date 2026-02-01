bits 16
org 0x7C00

start:
    cli
    mov ax, 0x0003
    int 0x10

    ; enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:pmode

; -------- GDT --------
gdt_start:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

bits 32
pmode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x00100000

times 510-($-$$) db 0
dw 0xAA55
