/* TernaryBit OS Kernel Linker Script */

ENTRY(_start)

SECTIONS
{
    . = 0x10000; /* Kernel load address */

    .text : {
        *(.text)
    }

    .data : {
        _data_start = .;
        *(.data)
        _data_end = .;
    }

    .bss : {
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }
}
