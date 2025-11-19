ENTRY(kernel_entry)
OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
    . = 0x00010000; /* physical address where stage2 loads us */

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    }
}
