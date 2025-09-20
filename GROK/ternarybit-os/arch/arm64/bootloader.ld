/* ARM64 Bootloader Linker Script for TernaryBit OS */

OUTPUT_ARCH(aarch64)
ENTRY(_start)

/* Base address where the bootloader will be loaded by the ROM */
BOOTLOADER_BASE = 0x0;

SECTIONS
{
    /* Start at the base address */
    . = BOOTLOADER_BASE;

    /* Bootloader code */
    .text : {
        *(.text.boot)  /* Boot code must come first */
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Read-write data (initialized) */
    .data : {
        *(.data)
        *(.data.*)
    }

    /* Read-write data (uninitialized) and stack */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    /* Ensure the bootloader is small enough */
    . = ALIGN(4K);
    _end = .;

    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}

/* Define symbols for use in startup code */
__bss_size = (__bss_end - __bss_start) >> 3;

/* Ensure the bootloader is small enough */
ASSERT((_end - BOOTLOADER_BASE) <= 64K, "Bootloader too large");
