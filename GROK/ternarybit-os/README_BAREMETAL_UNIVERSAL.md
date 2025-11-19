# TBOS Universal Bare-Metal Boot + Shell

This path builds a real two-stage BIOS bootloader that switches to 32-bit protected mode, loads a minimal kernel, and drops into an interactive shell that talks directly to VGA text mode and a PS/2 keyboard (IRQ-driven).

## What gets built
- `boot/universal_stage1.asm` – 512-byte MBR that loads stage 2 to 0x0000:0x8000.
- `boot/universal_stage2.asm` – uses INT 13h extensions to pull the kernel into memory, enables A20, sets up a flat GDT, switches to protected mode, and jumps to the kernel.
- `kernel/universal.c` – freestanding 32-bit kernel with VGA text, PS/2 keyboard via IRQ1, PIT timer via IRQ0, and a minimal shell.
- Image output: `build/universal/tbos_universal.img`.

## Build
```bash
./build_universal_baremetal.sh
```
Deps: `nasm`, `gcc` with `-m32`, `ld`, `objcopy`, `dd`.

## Run (QEMU)
```bash
qemu-system-i386 -drive format=raw,file=build/universal/tbos_universal.img -serial stdio
```

## Shell commands
- `help` – list commands
- `about` – build overview
- `cls` – clear screen
- `echo <text>` – print text
- `mem` – show simple memory info
- `ticks` – show PIT tick counter (IRQ0)
- `reboot` – trigger PS/2 reset
- `halt` / `quit` – stop CPU (`hlt`)

## Notes
- Boot flow: MBR → Stage2 (0x8000) → kernel @ 0x10000 → protected mode shell.
- Keyboard uses set 1 scancodes via IRQ1 (works under QEMU/BIOS).
- The disk image is BIOS/MBR only; write it to a USB drive with `dd` for physical tests.
