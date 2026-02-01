bits 32
global kernel_entry
extern kernel_main

kernel_entry:
    call kernel_main
    jmp $
