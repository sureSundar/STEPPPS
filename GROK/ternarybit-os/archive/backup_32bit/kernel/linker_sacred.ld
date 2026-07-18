/* TBOS Kernel Linker Script - Sprint 7
 * 🕉️ Manikanta Saranam 🕉️
 * Links kernel at 1MB (0x100000)
 */

ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 64KB (matches stage2_protected.asm) */
    . = 0x10000;

    /* Code section - no alignment to avoid file padding */
    .text :
    {
        *(.multiboot)
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data :
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data */
    .bss :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }

    /* End of kernel */
    __kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
    }
}