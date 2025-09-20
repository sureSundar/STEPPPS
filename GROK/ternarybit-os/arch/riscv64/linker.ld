/* RISC-V Linker Script for TernaryBit OS */

OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Base address where the kernel will be loaded */
BASE_ADDRESS = 0x80200000;

SECTIONS
{
    /* Start at the base address */
    . = BASE_ADDRESS;

    /* Kernel code */
    .text : {
        *(.text.boot)  /* Boot code must come first */
        *(.text .text.*)
        . = ALIGN(0x1000);
        *(.text.trampoline)
        . = ALIGN(0x1000);
        *(.tohost)
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

    /* End of the binary */
    . = ALIGN(4K);
    _end = .;
    _heap_start = .;

    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}

/* Define symbols for use in startup code */
__bss_size = (__bss_end - __bss_start) >> 3;
__stack_top = BASE_ADDRESS + 128K;  /* 128KB stack */
__global_pointer$ = __bss_end + 0x800;  /* For RISC-V gp-relative addressing */
