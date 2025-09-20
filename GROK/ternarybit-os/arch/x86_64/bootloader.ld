/* x86_64 Bootloader Linker Script for TernaryBit OS */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
    /* The bootloader will look at this image and start execution at the symbol
       designated as the entry point. */
    . = 0x7C00;
    
    /* First put the multiboot header, as it's required to be early in the image. */
    .text : {
        *(.text.boot)
        *(.text)
    }
    
    /* Read-only data. */
    .rodata : {
        *(.rodata)
    }
    
    /* Read-write data (initialized) */
    .data : {
        *(.data)
    }
    
    /* Read-write data (uninitialized) and stack */
    .bss : {
        *(COMMON)
        *(.bss)
    }
    
    /* The end of the binary. */
    _end = .;
    
    /* The boot signature. */
    .signature : {
        /* The boot signature must be the last 2 bytes of the 512 byte sector. */
        . = 510;
        SHORT(0xAA55)
    }
}

/* The size of the bootloader must be exactly 512 bytes. */
ASSERT(_end <= 0x7E00, "Bootloader too large");
