/* Multiboot-compliant linker script for TernaryBit OS */

ENTRY(_start)

SECTIONS
{
    /* Multiboot specification requires kernel to be loaded at 1MB */
    . = 1M;

    /* Multiboot header MUST come first */
    .multiboot ALIGN(4K) : {
        *(.multiboot)
    }

    /* Text section */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    /* End of kernel */
    _kernel_end = .;
}