ENTRY(kernel_entry)

SECTIONS {
    . = 1M;
    .text : { *(.text*) }
    .data : { *(.data*) }
    .bss  : { *(.bss*) }
}
