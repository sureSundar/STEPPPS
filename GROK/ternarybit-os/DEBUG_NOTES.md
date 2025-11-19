# TernaryBit OS - Step 3 Shell Debugging Notes

## Problem Summary
Step 3 kernel (command shell) was causing boot loops - the system would continuously reboot after entering protected mode, never displaying the shell prompt.

## Root Causes Identified

### Bug #1: Data Section Ordering Issue ⚠️

**Symptom**: Triple fault (General Protection Fault → Double Fault → Triple Fault) at EIP=0x102c9

**Root Cause**: The `scancode_table` was placed BEFORE the `idt` in the `.data` section:

```assembly
; WRONG ORDER - causes boot loop
section .data
scancode_table:
    db 0,0,'1','2','3'...  ; 58 bytes
idt:
    times 256*8 db 0       ; 2048 bytes
idt_ptr:
    dw 256*8 - 1
    dd idt
```

When `setup_idt` tried to write to `[idt + 33*8]` (keyboard interrupt entry), the linker/assembler generated incorrect addresses that caused the CPU to jump into the middle of the data section and try to execute it as code.

**Fix**: Move `scancode_table` AFTER `idt` and `idt_ptr`:

```assembly
; CORRECT ORDER - works perfectly
section .data
idt:
    times 256*8 db 0
idt_ptr:
    dw 256*8 - 1
    dd idt
scancode_table:
    db 0,0,'1','2','3'...  ; AFTER IDT
```

**Why This Matters**: Large data arrays placed before interrupt tables can cause addressing issues during setup. Always place interrupt-related structures (IDT, GDT) before other data arrays.

---

### Bug #2: Uninitialized BSS Variables ⚠️

**Symptom**: After fixing Bug #1, shell still had intermittent crashes or strange behavior

**Root Cause**: The `.bss` section (uninitialized data) is NOT zeroed by our simple bootloader. Variables like `cmd_buffer_pos` contained garbage values.

```assembly
section .bss
cmd_buffer:     resb 80
cmd_buffer_pos: resd 1    ; Could contain ANY value!
```

When the shell tried to access `cmd_buffer + [cmd_buffer_pos]`, if `cmd_buffer_pos` was a large garbage value (e.g., 0x8AF92034), it would access invalid memory.

**Fix**: Explicitly initialize all BSS variables at kernel entry:

```assembly
_start:
    ; Initialize BSS variables (bootloader doesn't zero BSS)
    mov dword [cmd_buffer_pos], 0

    ; Continue with initialization...
    call vga_clear_screen
    ...
```

**Why This Matters**: Unlike hosted applications, bare-metal code doesn't have a C runtime to zero `.bss`. You must explicitly initialize all variables or they contain random memory contents.

---

## Debugging Process

### Tools Used:
1. **QEMU debug mode**: `-d int,cpu_reset -D /tmp/debug.log -no-reboot`
2. **objdump**: Disassemble ELF to find crash location
3. **hexdump**: Examine binary data layout
4. **Boot counting**: `grep -c 'Stage 2 loaded'` to detect loops

### Key Debug Information:
- Crash at **EIP=0x000102c9** (inside IDT data section)
- Exception **0xd** (General Protection Fault)
- Error code **0x218** (IDT-related)
- Stack at unusual location: **ESP=0x0008fffc**

### Incremental Testing:
Created simplified versions to isolate issues:
- **Step 3A**: Echo test (no commands) - worked after data reordering
- **Step 3B**: Full shell - worked after BSS initialization

---

## Working Solution

### Complete Step 3 Structure:
```assembly
[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; CRITICAL: Initialize BSS first!
    mov dword [cmd_buffer_pos], 0

    call vga_clear_screen
    call vga_print_string
    call setup_idt
    call setup_pic
    sti
    call show_prompt
.loop:
    hlt
    jmp .loop

; ... all function implementations ...

section .data
cursor_pos:     dd 0
banner:         db '=== TernaryBit OS - Universal Shell ===', 10, 10, 0
prompt_str:     db 'TBOS> ', 0
cmd_help:       db 'help', 0
cmd_clear:      db 'clear', 0
cmd_about:      db 'about', 0
; ... other strings ...

; CRITICAL: IDT BEFORE scancode_table!
idt:
    times 256*8 db 0
idt_ptr:
    dw 256*8 - 1
    dd idt

; scancode_table AFTER IDT
scancode_table:
    db 0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0
    db 'q','w','e','r','t','y','u','i','o','p','[',']',0,0
    db 'a','s','d','f','g','h','j','k','l',';',39,96,0,92
    db 'z','x','c','v','b','n','m',',','.','/',0,'*',0,' '

section .bss
cmd_buffer:     resb 80
cmd_buffer_pos: resd 1
```

---

## Results

✅ **Step 0 (Minimal)**: 126 bytes - WORKING
✅ **Step 1 (VGA)**: 422 bytes - WORKING
✅ **Step 2 (Keyboard)**: 2,493 bytes - WORKING
✅ **Step 3 (Shell)**: 2,986 bytes - **NOW WORKING!**

Boot count: **1** (no boot loop)

### Shell Commands Available:
- `help` - Display command list
- `clear` - Clear screen
- `about` - Show system info

---

## Lessons Learned

1. **Data Section Ordering Matters**: Large arrays before interrupt tables can cause addressing issues
2. **BSS Requires Initialization**: Bare-metal code must manually zero uninitialized variables
3. **Incremental Testing**: Build simplified versions to isolate complex bugs
4. **QEMU Debug Mode**: `-d int,cpu_reset` is invaluable for finding triple faults
5. **Disassembly is Essential**: Use `objdump` to map crash addresses to code
6. **Test After Every Change**: Each step (0, 1, 2, 3) was verified independently

---

## Bug #3: BSS Section Addressing Issue ⚠️

**Symptom**: Kernel boots fine but crashes/loops when keyboard input is received

**Root Cause**: Incorrect addressing of BSS section variables. Using `mov esi, cmd_buffer` loads the VALUE at cmd_buffer (garbage), not the ADDRESS.

**Wrong Code**:
```assembly
; WRONG - loads value, not address
mov esi, cmd_buffer        ; ESI gets garbage from memory
mov byte [cmd_buffer + ebx], al  ; Crashes
```

**Fixed Code**:
```assembly
; CORRECT - loads effective address
lea esi, [cmd_buffer]      ; ESI gets address of cmd_buffer
lea ecx, [cmd_buffer]
mov byte [ecx + ebx], al   ; Works correctly
```

**Additional Fixes**:
1. Zero the cmd_buffer at initialization: `rep stosb`
2. Use LEA consistently for all BSS variable addresses
3. Fixed in `process_command`, `keyboard_handler`, and strcmp calls
4. **CRITICAL**: Fixed scancode_table lookup - was producing garbled characters:
   ```assembly
   ; WRONG - garbled output
   mov al, [scancode_table + ebx]

   ; CORRECT - proper characters
   lea ecx, [scancode_table]
   mov al, [ecx + ebx]
   ```

---

## Bug #4: Interrupt Handler Stack Corruption ⚠️

**Symptom**: Keyboard input causes system to "falter" - crashes, hangs, or reboots when typing

**Root Cause**: Calling complex functions (`process_command` → `strcmp` → `vga_print_string`) directly from interrupt handler causes deep call stack and potential corruption with `pusha/popa`.

**Wrong Architecture**:
```assembly
keyboard_handler:
    pusha
    ; ... handle key ...
    cmp al, 10
    je .enter

.enter:
    call vga_print_string    ; Deep call stack!
    call process_command     ; Even deeper!
    ; process_command calls strcmp, more vga functions...
    popa
    iretd
```

**Correct Architecture** - Deferred Command Processing:
```assembly
keyboard_handler:
    pusha
    ; ... handle key ...
    cmp al, 10
    je .enter

.enter:
    call vga_print_string    ; Only simple display
    mov byte [command_ready], 1  ; Set flag, don't process!
    popa
    iretd

; Main loop processes commands OUTSIDE interrupt context
_start.loop:
    cmp byte [command_ready], 0
    je .wait

    mov byte [command_ready], 0
    call process_command     ; Safe - not in interrupt
.wait:
    hlt
    jmp .loop
```

**Key Principle**: **Keep interrupt handlers minimal**. Only do essential work (read hardware, set flags), then return. Process complex logic in main loop.

### Final Ultra-Safe Architecture

The **working solution** uses a single-byte communication channel between interrupt and main loop:

```assembly
; Interrupt handler - ONLY stores character
keyboard_handler:
    pusha
    in al, 0x60
    test al, 0x80
    jnz .done
    call scancode_to_ascii
    mov byte [last_char], al  ; ← Single byte write
.done:
    out 0x20, al
    popa
    iretd

; Main loop - does ALL the work
.loop:
    mov al, [last_char]
    test al, al
    jz .check_command

    mov byte [last_char], 0

    ; Handle Enter/Backspace/Characters here
    ; Buffer management, VGA writes, command processing
    ; All safe outside interrupt context
```

This eliminates:
- ❌ Complex function calls from interrupt
- ❌ BSS variable access from interrupt (except single byte)
- ❌ Stack corruption from deep call chains
- ❌ VGA function calls during interrupt

Result: **Stable, reliable shell with no crashes!**

---

## Next Steps

- Integrate STEPPPS framework (7-dimensional system architecture)
- Add more shell commands
- Implement memory management
- Add file system support
- Port to other architectures (ARM, RISC-V)

---

**🕉️ Om Swamiye Saranam Aiyappa 🕉️**

*From 2-bit to Supercomputer - Universal Design Proven*
