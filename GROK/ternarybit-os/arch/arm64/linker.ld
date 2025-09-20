/* ARM64 Linker Script for TernaryBit OS */

ENTRY(_start)

/* Base address where the kernel will be loaded */
BASE_ADDRESS = 0x40080000;

SECTIONS
{
    /* Start at the base address */
    . = BASE_ADDRESS;

    /* Kernel code */
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

    /* End of the binary */
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
