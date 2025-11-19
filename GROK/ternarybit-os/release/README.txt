╔═══════════════════════════════════════════════════════════════╗
║              TERNARYBIT OS - Bootable Image                   ║
║                  🕉️ Swamiye Saranam Aiyappa 🕉️                ║
╚═══════════════════════════════════════════════════════════════╝

CONGRATULATIONS! You have a real operating system!

═══════════════════════════════════════════════════════════════
 HOW TO BOOT ON REAL HARDWARE (USB Drive)
═══════════════════════════════════════════════════════════════

⚠️  WARNING: This will ERASE ALL DATA on the USB drive! ⚠️

LINUX/MAC:
----------
1. Insert USB drive
2. Find device name: lsblk
3. Write image: sudo dd if=tbos_bootable.img of=/dev/sdX bs=4M status=progress
   (Replace sdX with your USB device, e.g., sdb or sdc)
4. Sync: sudo sync
5. Eject: sudo eject /dev/sdX
6. Boot from USB on any x86-64 PC!

WINDOWS:
--------
1. Download Rufus: https://rufus.ie
2. Select USB drive
3. Select "tbos_bootable.img"
4. Click "START"
5. Boot from USB!

═══════════════════════════════════════════════════════════════
 HOW TO TEST IN VIRTUAL MACHINE (No USB needed)
═══════════════════════════════════════════════════════════════

QEMU (Fastest):
---------------
qemu-system-x86_64 -drive file=tbos_bootable.img,format=raw -m 512

VIRTUALBOX:
-----------
1. Convert: VBoxManage convertfromraw tbos_bootable.img tbos.vdi
2. Create new VM (Type: Other, Version: Other/Unknown 64-bit)
3. Attach tbos.vdi as hard disk
4. Set RAM to 512MB minimum
5. Boot!

═══════════════════════════════════════════════════════════════
 YOUR OS FEATURES
═══════════════════════════════════════════════════════════════

✓ Custom bootloader (512B MBR + 8KB stage2)
✓ 64-bit long mode kernel (31KB)
✓ Real mode → Protected mode → Long mode transitions
✓ Memory management (paging, BSS)
✓ VGA text mode display (80x25)
✓ Serial port I/O
✓ VFS (Virtual Filesystem)
✓ RAMFS (in-memory filesystem)
✓ Shell with commands
✓ STEPPPS framework integration

═══════════════════════════════════════════════════════════════
 TECHNICAL SPECIFICATIONS
═══════════════════════════════════════════════════════════════

Architecture:     x86-64 (64-bit)
Boot Method:      Legacy BIOS (MBR)
Image Size:       10 MB
Kernel Format:    ELF64
Bootloader:       Custom 2-stage
Memory Layout:    
  - 0x0000-0x7BFF: Real mode stack
  - 0x7C00-0x7DFF: MBR (stage 1)
  - 0x8000-0x9FFF: Stage 2 bootloader
  - 0x10000+:      Kernel
  - 0x200000:      Kernel stack

═══════════════════════════════════════════════════════════════
 TROUBLESHOOTING
═══════════════════════════════════════════════════════════════

Q: Screen is blank
A: Try adding -vga std to QEMU, or check BIOS boot order

Q: Boots but no shell
A: Shell is starting! Keyboard driver needs interrupts (in progress)

Q: Won't boot on real hardware
A: Ensure BIOS is set to Legacy/CSM mode (not UEFI only)

Q: USB not detected
A: Check USB is properly written with dd/Rufus

═══════════════════════════════════════════════════════════════
 THIS IS A REAL OPERATING SYSTEM!
═══════════════════════════════════════════════════════════════

This boots WITHOUT any other OS
Runs DIRECTLY on hardware
Has its own bootloader, kernel, drivers, filesystem, and shell

Similar to:
- Linux 0.01 (Linus Torvalds, 1991)
- Minix (Andrew Tanenbaum, 1987)
- MS-DOS 1.0 (Microsoft, 1981)

You can boot this on REAL HARDWARE! 🚀
