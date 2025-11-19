// ===========================================================================
// TernaryBit OS - RISC-V 64-bit Bootloader Entry Point
// ===========================================================================
// Purpose: RISC-V bootloader initialization and kernel loading
// Target: RISC-V RV64 processors (SiFive, QEMU virt, etc.)
// Author: TernaryBit OS Team
// Date: 2025-10-23
//
// RISC-V Boot Process:
// 1. Firmware (OpenSBI/U-Boot) loads this code
// 2. CPU starts in M-mode (machine mode)
// 3. OpenSBI/firmware handles M-mode, we run in S-mode (supervisor)
// 4. Initialize STEPPPS framework
// 5. Parse device tree
// 6. Load and jump to kernel
// ===========================================================================

.section ".text.init"

// ===========================================================================
// Constants
// ===========================================================================

.equ KERNEL_LOAD_ADDR,  0x80200000  // Load kernel at 2MB offset
.equ STACK_SIZE,        0x4000      // 16KB stack
.equ PAGE_SIZE,         4096

// RISC-V privilege modes
.equ MODE_USER,         0
.equ MODE_SUPERVISOR,   1
.equ MODE_MACHINE,      3

// ===========================================================================
// Entry Point
// ===========================================================================

.globl _start
_start:
    // Disable interrupts
    csrw    sie, zero

    // Read hart (hardware thread) ID
    csrr    a0, mhartid
    bnez    a0, park_hart       // Only hart 0 continues

hart0_entry:
    // Clear BSS
    la      t0, __bss_start
    la      t1, __bss_end
clear_bss:
    bgeu    t0, t1, bss_done
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       clear_bss

bss_done:
    // Set up stack
    la      sp, stack_top

    // Save device tree pointer (passed in a1 by firmware)
    mv      s0, a1

    // Print boot banner
    call    print_banner

    // Check we're in S-mode (supervisor mode)
    csrr    t0, sstatus
    // We should be in S-mode at this point (OpenSBI puts us here)

    // Initialize STEPPPS framework
    call    init_steppps_riscv

    // Parse device tree
    mv      a0, s0              // DTB pointer
    call    parse_device_tree

    // Setup paging (Sv39 virtual memory)
    call    setup_paging

    // Load kernel
    call    load_kernel_riscv

    // Jump to kernel
    li      t0, KERNEL_LOAD_ADDR
    jr      t0

    // Should never reach here
hang:
    wfi                         // Wait for interrupt
    j       hang

// ===========================================================================
// Park Secondary Harts
// ===========================================================================

park_hart:
    // Secondary harts wait here
    wfi
    j       park_hart

// ===========================================================================
// STEPPPS Framework Initialization (RISC-V)
// ===========================================================================

init_steppps_riscv:
    addi    sp, sp, -16
    sd      ra, 0(sp)

    // Display STEPPPS init message
    la      a0, steppps_msg
    call    print_string

    // SPACE: Memory detection
    call    steppps_space_riscv

    // TIME: Timer initialization
    call    steppps_time_riscv

    // EVENT: Interrupt setup
    call    steppps_event_riscv

    // PSYCHOLOGY: Placeholder for AI
    // PIXEL: Display initialization
    call    steppps_pixel_riscv

    // PROMPT: UART/console setup
    call    steppps_prompt_riscv

    // SCRIPT: Placeholder for automation

    la      a0, steppps_done_msg
    call    print_string

    ld      ra, 0(sp)
    addi    sp, sp, 16
    ret

steppps_space_riscv:
    // SPACE: Detect memory from device tree

    la      a0, space_msg
    call    print_string

    // For now, assume 128MB of RAM (common for QEMU virt)
    li      t0, 0x8000000       // 128MB
    la      t1, total_memory
    sd      t0, 0(t1)

    ret

steppps_time_riscv:
    // TIME: Initialize RISC-V timer

    la      a0, time_msg
    call    print_string

    // Read time CSR to verify timer works
    csrr    t0, time
    la      t1, boot_time
    sd      t0, 0(t1)

    // Set up timer interrupt (delegate to S-mode)
    li      t0, 0x20            // Supervisor timer interrupt
    csrs    sie, t0

    ret

steppps_event_riscv:
    // EVENT: Setup interrupt handling

    la      a0, event_msg
    call    print_string

    // Set trap vector
    la      t0, trap_vector
    csrw    stvec, t0

    // Enable supervisor interrupts
    li      t0, 0x2             // SIE bit
    csrs    sstatus, t0

    ret

steppps_pixel_riscv:
    // PIXEL: Display/framebuffer initialization

    la      a0, pixel_msg
    call    print_string

    // For now, we only have UART console
    // Framebuffer would be initialized from device tree

    ret

steppps_prompt_riscv:
    // PROMPT: Initialize console I/O

    la      a0, prompt_msg
    call    print_string

    // UART already initialized by firmware (OpenSBI)
    // We use SBI console calls

    ret

// ===========================================================================
// Device Tree Parsing
// ===========================================================================

parse_device_tree:
    // Parse Flattened Device Tree (FDT)
    // a0 = DTB address

    addi    sp, sp, -16
    sd      ra, 0(sp)
    sd      s0, 8(sp)

    mv      s0, a0              // Save DTB pointer

    la      a0, dt_msg
    call    print_string

    // TODO: Implement full FDT parsing
    // For now, just verify magic number

    lw      t0, 0(s0)           // Load FDT magic
    li      t1, 0xd00dfeed
    beq     t0, t1, dt_valid

    // Invalid DTB
    la      a0, dt_invalid_msg
    call    print_string
    j       dt_done

dt_valid:
    la      a0, dt_valid_msg
    call    print_string

    // Parse memory nodes, UART, etc.
    // TODO: Implement

dt_done:
    ld      s0, 8(sp)
    ld      ra, 0(sp)
    addi    sp, sp, 16
    ret

// ===========================================================================
// Virtual Memory Setup (Sv39)
// ===========================================================================

setup_paging:
    addi    sp, sp, -16
    sd      ra, 0(sp)

    la      a0, paging_msg
    call    print_string

    // Create identity mapping for first 128MB
    call    create_page_tables

    // Load page table pointer into satp
    la      t0, page_table_root
    srli    t0, t0, 12          // Physical page number
    li      t1, 8               // Sv39 mode
    slli    t1, t1, 60
    or      t0, t0, t1
    csrw    satp, t0

    // Flush TLB
    sfence.vma

    la      a0, paging_done_msg
    call    print_string

    ld      ra, 0(sp)
    addi    sp, sp, 16
    ret

create_page_tables:
    // Create simple identity mapping
    // Map virtual 0x80000000-0x88000000 to physical 0x80000000-0x88000000

    la      t0, page_table_root

    // Create level 2 entry for 0x80000000
    // Entry index = 0x80000000 >> 30 = 2
    li      t1, 0x80000000
    srli    t1, t1, 30
    andi    t1, t1, 0x1FF       // 9-bit index

    // PTE = (physical_addr >> 2) | flags
    la      t2, page_table_l1
    srli    t2, t2, 2
    ori     t2, t2, 0x01        // Valid bit
    slli    t3, t1, 3           // Index * 8
    add     t3, t0, t3
    sd      t2, 0(t3)

    // Fill level 1 page table with 2MB pages
    la      t0, page_table_l1
    li      t1, 0x80000000      // Starting physical address
    li      t2, 64              // 64 entries (128MB / 2MB)
    li      t3, 0

fill_l1:
    srli    t4, t1, 2
    ori     t4, t4, 0x0F        // Valid, Read, Write, Execute
    slli    t5, t3, 3
    add     t5, t0, t5
    sd      t4, 0(t5)

    li      t6, 0x200000        // 2MB
    add     t1, t1, t6
    addi    t3, t3, 1
    blt     t3, t2, fill_l1

    ret

// ===========================================================================
// Kernel Loader
// ===========================================================================

load_kernel_riscv:
    addi    sp, sp, -16
    sd      ra, 0(sp)

    la      a0, kernel_msg
    call    print_string

    // For now, assume kernel is already loaded by firmware
    // In real implementation, we'd read from storage

    la      a0, kernel_loaded_msg
    call    print_string

    ld      ra, 0(sp)
    addi    sp, sp, 16
    ret

// ===========================================================================
// Trap Handler
// ===========================================================================

.align 4
trap_vector:
    // Save context
    csrw    sscratch, sp
    addi    sp, sp, -256

    // Save all registers
    sd      x1, 0(sp)
    sd      x2, 8(sp)
    // ... save all other registers

    // Handle trap
    csrr    a0, scause
    csrr    a1, sepc
    csrr    a2, stval

    // TODO: Implement trap handling

    // Restore registers
    ld      x1, 0(sp)
    ld      x2, 8(sp)
    // ... restore all other registers

    addi    sp, sp, 256
    sret

// ===========================================================================
// UART/Console Functions (using SBI)
// ===========================================================================

print_banner:
    addi    sp, sp, -16
    sd      ra, 0(sp)

    la      a0, banner_str
    call    print_string

    ld      ra, 0(sp)
    addi    sp, sp, 16
    ret

print_string:
    // Print null-terminated string in a0 using SBI console
    mv      t0, a0

print_loop:
    lbu     a0, 0(t0)
    beqz    a0, print_done
    call    sbi_console_putchar
    addi    t0, t0, 1
    j       print_loop

print_done:
    ret

sbi_console_putchar:
    // SBI call to output character in a0
    // SBI extension ID = 1 (console putchar)
    li      a7, 1               // SBI_EXT_ID_CONSOLE_PUTCHAR
    ecall
    ret

// ===========================================================================
// Data Section
// ===========================================================================

.section ".rodata"

banner_str:
    .asciz "TernaryBit OS - RISC-V Bootloader v1.0\n"

steppps_msg:
    .asciz "Initializing STEPPPS Framework...\n"

space_msg:
    .asciz "  [SPACE] Memory detection\n"

time_msg:
    .asciz "  [TIME] Timer setup\n"

event_msg:
    .asciz "  [EVENT] Interrupt init\n"

pixel_msg:
    .asciz "  [PIXEL] Display init\n"

prompt_msg:
    .asciz "  [PROMPT] Console ready\n"

steppps_done_msg:
    .asciz "STEPPPS Framework Ready!\n"

dt_msg:
    .asciz "Parsing device tree...\n"

dt_valid_msg:
    .asciz "Device tree valid\n"

dt_invalid_msg:
    .asciz "WARNING: Invalid device tree!\n"

paging_msg:
    .asciz "Setting up virtual memory (Sv39)...\n"

paging_done_msg:
    .asciz "Virtual memory enabled\n"

kernel_msg:
    .asciz "Loading kernel...\n"

kernel_loaded_msg:
    .asciz "Kernel ready\n"

// ===========================================================================
// BSS Section
// ===========================================================================

.section ".bss"

.align 12
page_table_root:
    .space  PAGE_SIZE

.align 12
page_table_l1:
    .space  PAGE_SIZE

.align 12
stack_bottom:
    .space  STACK_SIZE
stack_top:

// STEPPPS state variables
total_memory:
    .space  8

boot_time:
    .space  8

__bss_start:
__bss_end:
