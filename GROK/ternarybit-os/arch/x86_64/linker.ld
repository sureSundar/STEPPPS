/* x86_64 Linker Script for TernaryBit OS */

ENTRY(_start)
SECTIONS
{
    /* Load the kernel at 1MB, a conventional place for kernels to be loaded at by the bootloader. */
    . = 1M;

    /* First put the multiboot header, as it's required to be early in the image. */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    /* Read-only data. */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    /* The compiler may produce other sections, by default it will put them in
     * a segment with the same name. Simply add stuff here as needed. */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
