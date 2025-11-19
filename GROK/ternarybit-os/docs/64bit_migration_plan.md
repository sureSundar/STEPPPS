# TernaryBit OS - 64-bit Migration Plan

## Overview
Migrate TernaryBit OS from 32-bit protected mode to 64-bit long mode to support modern AMD x86-64 hardware natively.

## Current State (32-bit)
- **Bootloader**: 16-bit real mode → 32-bit protected mode
- **Kernel**: 32-bit ELF compiled with `-m32`
- **Address Space**: 4GB limit
- **Issue**: Function calls crash in larger kernels (GCC ABI quirks)

## Target State (64-bit)
- **Bootloader**: 16-bit → 32-bit → 64-bit long mode
- **Kernel**: 64-bit ELF compiled with `-m64`
- **Address Space**: Full 64-bit (16 EB theoretical, 256 TB practical)
- **Benefits**: Native AMD64, no ABI quirks, future-proof

## Migration Steps

### Phase 1: Bootloader Long Mode Transition (2-3 hours)

#### 1.1 Enable PAE (Physical Address Extension)
```asm
; In stage2_protected.asm, before entering protected mode
mov eax, cr4
or eax, (1 << 5)  ; Set PAE bit
mov cr4, eax
```

#### 1.2 Setup Page Tables
Create 4-level paging structure:
- PML4 (Page Map Level 4)
- PDPT (Page Directory Pointer Table)
- PD (Page Directory)
- PT (Page Table)

Identity map first 2MB:
```asm
; Setup page tables at 0x1000
mov edi, 0x1000
mov cr3, edi      ; Set page table base

; PML4[0] → PDPT
mov dword [edi], 0x2003
add edi, 0x1000

; PDPT[0] → PD
mov dword [edi], 0x3003
add edi, 0x1000

; PD[0] → 2MB page
mov dword [edi], 0x00000083  ; Present, RW, 2MB
```

#### 1.3 Enable Long Mode
```asm
; Enable long mode in EFER MSR
mov ecx, 0xC0000080  ; EFER MSR
rdmsr
or eax, (1 << 8)     ; Set LME bit
wrmsr

; Enable paging
mov eax, cr0
or eax, (1 << 31)    ; Set PG bit
mov cr0, eax
; Now in compatibility mode

; Jump to 64-bit code segment
jmp CODE64_SEG:long_mode_entry
```

#### 1.4 Create 64-bit GDT
```asm
gdt64:
    dq 0                    ; Null descriptor
    dq 0x00AF9A000000FFFF   ; 64-bit code segment
    dq 0x00AF92000000FFFF   ; 64-bit data segment
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64 - 1
    dq gdt64
```

### Phase 2: 64-bit Kernel (3-4 hours)

#### 2.1 Update Build Flags
In `build_universal.sh`:
```bash
gcc -m64 -mcmodel=kernel -mno-red-zone \
    -ffreestanding -fno-builtin -fno-stack-protector \
    -fno-pic -fno-pie -fcf-protection=none \
    -nostdlib -Wall -Wextra -O2 \
    -c "$absolute" -o "$out"
```

Key changes:
- `-m64`: 64-bit compilation
- `-mcmodel=kernel`: Kernel code model
- `-mno-red-zone`: Disable red zone (128-byte stack buffer)

#### 2.2 Update Linker Script
```ld
ENTRY(_start)

SECTIONS
{
    /* Load at 1MB for 64-bit (canonical address) */
    . = 0xFFFFFFFF80100000;  /* -2GB + 1MB */

    .text : AT(0x100000) {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
}
```

#### 2.3 Update Assembly Entry Point
```asm
[BITS 64]
[GLOBAL _start]
[EXTERN kernel_main]

section .text
_start:
    ; Clear segment registers
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Setup 64-bit stack
    mov rsp, 0x200000
    mov rbp, rsp

    ; Clear BSS
    mov rdi, __bss_start
    mov rcx, __bss_end
    sub rcx, rdi
    xor rax, rax
    rep stosb

    ; Call kernel_main (no arguments)
    call kernel_main

    ; Halt
    cli
.halt:
    hlt
    jmp .halt
```

#### 2.4 Update C Code for 64-bit
- Change pointer types: `uint32_t*` → `uint64_t*` where needed
- Update inline assembly constraints
- Fix any size_t/uintptr_t assumptions

### Phase 3: Testing & Validation (1-2 hours)

#### 3.1 QEMU Testing
```bash
qemu-system-x86_64 -drive file=build/tbos_universal.img,format=raw,if=floppy \
    -m 512 -nographic
```
Note: Use `qemu-system-x86_64` instead of `qemu-system-i386`

#### 3.2 Validation Checklist
- [ ] Bootloader reaches long mode
- [ ] Kernel entry point called
- [ ] BSS cleared successfully
- [ ] Serial output works
- [ ] VGA text mode works
- [ ] Function calls work (main test!)
- [ ] Interrupts can be enabled
- [ ] Keyboard input works
- [ ] Shell launches
- [ ] File system operations work

### Phase 4: Optimization (1-2 hours)

#### 4.1 Enable Full 64-bit Features
- Use RIP-relative addressing
- Take advantage of additional registers (R8-R15)
- Use 64-bit integer operations

#### 4.2 Performance Tuning
- Re-enable `-O2` optimization
- Profile hotspots
- Optimize critical paths

## Timeline

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: Bootloader | 2-3 hours | None |
| Phase 2: Kernel | 3-4 hours | Phase 1 complete |
| Phase 3: Testing | 1-2 hours | Phase 2 complete |
| Phase 4: Optimization | 1-2 hours | Phase 3 complete |
| **Total** | **7-11 hours** | |

## Risks & Mitigations

### Risk 1: Page Table Setup Complexity
**Mitigation**: Start with simple identity mapping, expand later

### Risk 2: GDT/IDT Structure Changes
**Mitigation**: Test GDT first before enabling interrupts

### Risk 3: ABI Differences
**Mitigation**: Compile everything with same flags, test incrementally

### Risk 4: Debugging Difficulty
**Mitigation**: Add extensive serial logging at each step

## Rollback Plan

If 64-bit migration fails:
1. Keep 32-bit codebase in git branch
2. Can switch back with `git checkout 32bit-stable`
3. Continue debugging 32-bit function call issue

## Success Criteria

✅ System boots to 64-bit long mode
✅ Kernel executes in 64-bit
✅ All function calls work correctly
✅ Shell is interactive with keyboard
✅ File system operations complete
✅ No regression in functionality

## Next Steps

1. Create new branch: `git checkout -b 64bit-longmode`
2. Start with Phase 1 bootloader changes
3. Test each phase incrementally
4. Document any issues encountered

---
*Document created: 2025-11-10*
*Status: PLANNING*
