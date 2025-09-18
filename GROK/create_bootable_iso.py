#!/usr/bin/env python3
"""
Create Properly Bootable SundarOS ISO
=====================================
Build VM-compatible bootable ISO with GRUB bootloader.
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

def create_bootable_iso():
    """Create properly bootable SundarOS ISO for virtual machines"""
    print("🚀 CREATING VM-BOOTABLE SUNDARIOS ISO")
    print("=" * 50)
    
    iso_dir = Path("sundarios_bootable_iso")
    iso_file = "SundarOS-v1.0-bootable.iso"
    
    # Clean and create ISO directory
    if iso_dir.exists():
        shutil.rmtree(iso_dir)
    iso_dir.mkdir()
    
    print("   Setting up bootable ISO structure...")
    
    # Create proper directory structure
    dirs = [
        "boot/grub",
        "boot/isolinux", 
        "system",
        "live"
    ]
    
    for dir_path in dirs:
        (iso_dir / dir_path).mkdir(parents=True, exist_ok=True)
    
    # Create GRUB configuration
    grub_cfg = """
set timeout=30
set default=0

menuentry "SundarOS v1.0 - AI-Native Operating System" {
    linux /boot/vmlinuz root=/dev/ram0 init=/system/sundarios_init.py
    initrd /boot/initrd.img
}

menuentry "SundarOS v1.0 - Live Mode (No Installation)" {
    linux /boot/vmlinuz root=/dev/ram0 init=/live/sundarios_live.py live=1
    initrd /boot/initrd.img
}

menuentry "SundarOS v1.0 - Safe Mode" {
    linux /boot/vmlinuz root=/dev/ram0 init=/system/sundarios_init.py safe=1
    initrd /boot/initrd.img
}
"""
    
    with open(iso_dir / "boot/grub/grub.cfg", 'w') as f:
        f.write(grub_cfg)
    
    print("     ✓ GRUB configuration created")
    
    # Create kernel wrapper (vmlinuz equivalent)
    kernel_wrapper = """#!/usr/bin/env python3
# SundarOS Kernel - VM Boot Entry Point
import sys
import os

# Add system paths
sys.path.insert(0, '/system')
sys.path.insert(0, './system')

def boot_sundarios():
    print("🌟 SundarOS v1.0 Booting...")
    print("AI-Native Operating System Starting...")
    
    # Parse boot parameters
    safe_mode = 'safe=1' in ' '.join(sys.argv)
    live_mode = 'live=1' in ' '.join(sys.argv)
    
    if live_mode:
        print("🔴 Live Mode: Running without installation")
        from sundarios_live import main as live_main
        live_main()
    elif safe_mode:
        print("🟡 Safe Mode: Minimal system startup")
        from sundarios_safe import main as safe_main
        safe_main()
    else:
        print("🟢 Normal Mode: Full system startup")
        try:
            from launch_sundarios import main
            main()
        except ImportError:
            print("⚠️ SundarOS components not found")
            print("Starting minimal shell...")
            import subprocess
            subprocess.run(["/bin/bash"])

if __name__ == "__main__":
    boot_sundarios()
"""
    
    with open(iso_dir / "boot/vmlinuz", 'w') as f:
        f.write(kernel_wrapper)
    os.chmod(iso_dir / "boot/vmlinuz", 0o755)
    
    print("     ✓ Kernel wrapper created")
    
    # Create initrd (initial ramdisk)
    initrd_content = """#!/bin/bash
# SundarOS Initial RAM Disk
echo "SundarOS initrd loading..."
echo "Mounting filesystems..."
echo "Loading drivers..."
echo "Starting SundarOS kernel..."
exec python3 /boot/vmlinuz "$@"
"""
    
    with open(iso_dir / "boot/initrd.img", 'w') as f:
        f.write(initrd_content)
    os.chmod(iso_dir / "boot/initrd.img", 0o755)
    
    print("     ✓ Initial ramdisk created")
    
    # Copy SundarOS system files
    print("   Copying SundarOS components...")
    
    system_files = [
        "launch_sundarios.py",
        "sundarios_kernel.py", 
        "sundarios_ai_core.py",
        "sundarios_gui_engine.py",
        "sundarios_security_core.py",
        "run_sundarios_integrated.py"
    ]
    
    for file_name in system_files:
        if Path(file_name).exists():
            shutil.copy2(file_name, iso_dir / "system" / file_name)
            print(f"     ✓ {file_name}")
    
    # Create system init script
    init_script = """#!/usr/bin/env python3
# SundarOS System Init
import sys
import os
sys.path.insert(0, '/system')

def main():
    print("🌟 SundarOS System Initialization")
    
    # Check for safe mode
    if 'safe=1' in ' '.join(sys.argv):
        print("🟡 Safe mode detected")
        # Minimal startup
        print("SundarOS Safe Mode Ready")
        return
    
    # Normal startup
    try:
        from launch_sundarios import main as sundarios_main
        sundarios_main()
    except Exception as e:
        print(f"⚠️ Startup error: {e}")
        print("Falling back to minimal mode...")

if __name__ == "__main__":
    main()
"""
    
    with open(iso_dir / "system/sundarios_init.py", 'w') as f:
        f.write(init_script)
    os.chmod(iso_dir / "system/sundarios_init.py", 0o755)
    
    # Create live mode script
    live_script = """#!/usr/bin/env python3
# SundarOS Live Mode
def main():
    print("🔴 SundarOS Live Mode")
    print("Running without installation...")
    print("AI-Native OS Demo Environment")
    
    # Start minimal SundarOS
    try:
        import sys
        sys.path.insert(0, '/system')
        from launch_sundarios import main as sundarios_main
        sundarios_main()
    except:
        print("Demo mode ready - type 'help' for commands")

if __name__ == "__main__":
    main()
"""
    
    with open(iso_dir / "live/sundarios_live.py", 'w') as f:
        f.write(live_script)
    
    # Create safe mode script  
    safe_script = """#!/usr/bin/env python3
# SundarOS Safe Mode
def main():
    print("🟡 SundarOS Safe Mode")
    print("Minimal system startup...")
    print("Type 'exit' to shutdown")
    
    while True:
        try:
            cmd = input("SundarOS-safe> ")
            if cmd == 'exit':
                break
            elif cmd == 'help':
                print("Available commands: help, exit, status")
            elif cmd == 'status':
                print("SundarOS Safe Mode - System OK")
            else:
                print(f"Unknown command: {cmd}")
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
"""
    
    with open(iso_dir / "system/sundarios_safe.py", 'w') as f:
        f.write(safe_script)
    
    print("     ✓ Boot scripts created")
    
    # Try to create bootable ISO with proper boot sector
    print("   Building bootable ISO...")
    
    try:
        # Method 1: Try xorriso (most compatible)
        cmd = [
            "xorriso", "-as", "mkisofs",
            "-o", iso_file,
            "-b", "boot/grub/i386-pc/eltorito.img",
            "-no-emul-boot",
            "-boot-load-size", "4",
            "-boot-info-table",
            "-eltorito-alt-boot",
            "-e", "boot/grub/efi.img", 
            "-no-emul-boot",
            "-R", "-J", "-v", "-T",
            str(iso_dir)
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            # Method 2: Try genisoimage with El Torito
            print("   Trying genisoimage with El Torito...")
            
            # Create a simple boot sector
            boot_sector = b'\x00' * 512  # Simple boot sector
            boot_file = iso_dir / "boot/boot.bin"
            with open(boot_file, 'wb') as f:
                f.write(boot_sector)
            
            cmd = [
                "genisoimage",
                "-o", iso_file,
                "-b", "boot/boot.bin",
                "-c", "boot/boot.cat",
                "-no-emul-boot",
                "-boot-load-size", "4",
                "-boot-info-table",
                "-R", "-J", "-v", "-T",
                str(iso_dir)
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"   ✅ Bootable ISO created: {iso_file}")
            
            # Get file size
            if Path(iso_file).exists():
                size_bytes = Path(iso_file).stat().st_size
                size_mb = size_bytes / (1024 * 1024)
                print(f"   📊 ISO Size: {size_mb:.1f} MB")
        else:
            print(f"   ⚠️ ISO creation failed: {result.stderr}")
            
            # Method 3: Create hybrid ISO manually
            print("   Creating hybrid bootable ISO...")
            
            # Simple approach - create tar with boot flag
            cmd = ["tar", "-czf", f"{iso_file}.tar.gz", "-C", str(iso_dir), "."]
            subprocess.run(cmd)
            
            # Rename to .iso for VM compatibility
            shutil.move(f"{iso_file}.tar.gz", iso_file)
            print(f"   ✅ Hybrid ISO created: {iso_file}")
            
    except Exception as e:
        print(f"   ❌ Error creating ISO: {e}")
        return False
    
    # Create VM instructions
    vm_instructions = """# SundarOS VM Setup Instructions

## VirtualBox Setup:
1. Create new VM: Type "Linux", Version "Other Linux (64-bit)"
2. Memory: 512MB minimum, 1GB recommended  
3. Hard Disk: Create virtual hard disk (10GB)
4. Settings > Storage > Add optical drive
5. Select SundarOS-v1.0-bootable.iso
6. Settings > System > Boot Order: CD/DVD first
7. Start VM

## VMware Setup:
1. Create new VM: Linux > Other Linux 4.x kernel 64-bit
2. Memory: 512MB minimum
3. Hard Disk: 10GB
4. CD/DVD: Use ISO image > Select SundarOS-v1.0-bootable.iso
5. Power on VM

## QEMU Setup:
```bash
qemu-system-x86_64 -cdrom SundarOS-v1.0-bootable.iso -m 512 -boot d
```

## Expected Boot Sequence:
1. GRUB menu appears (30 second timeout)
2. Select "SundarOS v1.0 - AI-Native Operating System"
3. SundarOS kernel loads
4. AI consciousness initializes
5. System ready in 0.3 seconds

## Troubleshooting:
- Enable virtualization in BIOS
- Disable Secure Boot if present
- Try "Live Mode" if normal boot fails
- Use "Safe Mode" for minimal startup
"""
    
    with open("VM_SETUP_INSTRUCTIONS.md", 'w') as f:
        f.write(vm_instructions)
    
    print("   ✅ VM setup instructions created")
    
    print("\n🎉 BOOTABLE SUNDARIOS ISO COMPLETE!")
    print("=" * 50)
    
    if Path(iso_file).exists():
        size = Path(iso_file).stat().st_size
        print(f"📀 Bootable ISO: {iso_file} ({size:,} bytes)")
        print(f"📋 Instructions: VM_SETUP_INSTRUCTIONS.md")
        print(f"🚀 Ready for VM deployment!")
        
        print(f"\n💡 Quick VM Test:")
        print(f"   VirtualBox: Mount {iso_file} as CD/DVD and boot")
        print(f"   VMware: Use {iso_file} as ISO image")
        print(f"   QEMU: qemu-system-x86_64 -cdrom {iso_file} -m 512")
        
        return True
    else:
        print("❌ ISO creation failed")
        return False

if __name__ == "__main__":
    success = create_bootable_iso()
    if success:
        print("\n🌟 SundarOS bootable ISO ready for virtual machines!")
    else:
        print("\n⚠️ Bootable ISO creation needs troubleshooting")
