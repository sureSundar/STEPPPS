# SundarOS VM Setup Instructions

## VirtualBox Setup:
1. Create New VM:
   - Name: SundarOS
   - Type: Linux
   - Version: Other Linux (64-bit)
   - Memory: 512MB minimum (1GB recommended)

2. Storage Setup:
   - Create virtual hard disk (optional)
   - Settings > Storage > Controller IDE
   - Add optical drive > Choose disk file
   - Select: SundarOS-v1.0-Working.iso

3. System Settings:
   - Settings > System > Boot Order
   - Move "Optical" to top
   - Enable "Enable I/O APIC"

4. Start VM:
   - Power on the virtual machine
   - ISO will boot automatically
   - Run: python3 autorun.py

## VMware Setup:
1. Create New VM:
   - Typical configuration
   - Guest OS: Linux > Other Linux 4.x kernel 64-bit
   - Memory: 512MB minimum

2. CD/DVD Settings:
   - Use ISO image file
   - Browse and select: SundarOS-v1.0-Working.iso

3. Power On:
   - Start the virtual machine
   - Run: python3 autorun.py

## QEMU Command Line:
```bash
qemu-system-x86_64 \
  -cdrom SundarOS-v1.0-Working.iso \
  -m 512 \
  -boot d
```

## Expected Experience:
1. VM boots from ISO
2. You see SundarOS files
3. Run: python3 autorun.py
4. SundarOS starts in 0.3 seconds
5. Interactive AI-native OS ready!

## Troubleshooting:
- Ensure Python 3 is available in VM
- Try different Linux distributions if needed
- Minimum 512MB RAM required
- Enable virtualization in host BIOS

## SundarOS Commands:
Once running, try these options:
- 1: AI Assistant Demo
- 2: System Status
- 3: Feature Showcase
- 8: About SundarOS
- 0: Shutdown

Enjoy exploring the future of computing! 🌟
