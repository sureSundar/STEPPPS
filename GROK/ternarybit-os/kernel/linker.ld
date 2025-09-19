/* TernaryBit OS Kernel Linker Script */

ENTRY(_start)

SECTIONS
{
    . = 0x10000; /* Kernel load address */

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}
