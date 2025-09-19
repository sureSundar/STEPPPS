# multiboot_kernel.S - Assembly file with proper multiboot header

.set MULTIBOOT_MAGIC,    0x1BADB002
.set MULTIBOOT_FLAGS,    0x00000003
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# Multiboot header MUST be in first 8192 bytes and aligned on 4-byte boundary
.section .multiboot
.align 4
multiboot_header:
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM

# Entry point
.section .text
.global _start
.type _start, @function

_start:
    # Disable interrupts
    cli

    # Set up stack
    mov $stack_top, %esp

    # Reset EFLAGS
    pushl $0
    popf

    # Call kernel main
    call kernel_main

    # Halt if kernel_main returns
hang:
    hlt
    jmp hang

# Stack section
.section .bss
.align 16
stack_bottom:
    .skip 16384  # 16KB stack
stack_top: