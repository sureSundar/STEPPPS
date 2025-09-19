/* Fixed linker script for TernaryBit OS */

ENTRY(_start)

SECTIONS
{
    . = 1M;

    /* Multiboot header must be early */
    .multiboot ALIGN(4K) : {
        *(.multiboot)
    }

    /* Text section with entry point */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Uninitialized data */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    /* Stack */
    .stack ALIGN(4K) : {
        *(.stack)
    }
}