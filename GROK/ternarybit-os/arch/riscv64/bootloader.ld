/* RISC-V Bootloader Linker Script for TernaryBit OS */

OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Base address where the bootloader will be loaded by the ROM */
BOOTLOADER_BASE = 0x80000000;

SECTIONS
{
    /* Start at the base address */
    . = BOOTLOADER_BASE;

    /* Bootloader code */
    .text : {
        *(.text.boot)  /* Boot code must come first */
        *(.text .text.*)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    }

    /* Read-write data (initialized) */
    .data : {
        *(.data .data.*)
        *(.sdata .sdata.*)
    }

    /* Read-write data (uninitialized) and stack */
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
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
__stack_top = BOOTLOADER_BASE + 32K;  /* 32KB stack for bootloader */
__global_pointer$ = __bss_end + 0x800;  /* For RISC-V gp-relative addressing */

/* Ensure the bootloader is small enough */
ASSERT((_end - BOOTLOADER_BASE) <= 64K, "Bootloader too large");
