# SundarOS Virtual Machine Setup Guide

## Files Created:
- SundarOS-v1.0-Final.iso (Main ISO file)
- SundarOS-v1.0-Final.iso.md5 (MD5 checksum)
- SundarOS-v1.0-Final.iso.sha256 (SHA256 checksum)

## VirtualBox Setup:
1. Create New Virtual Machine:
   - Name: SundarOS
   - Type: Linux
   - Version: Other Linux (64-bit)
   - Memory: 512MB minimum

2. Storage Configuration:
   - Settings > Storage
   - Controller: IDE > Add Optical Drive
   - Choose: SundarOS-v1.0-Final.iso

3. Boot Settings:
   - Settings > System > Boot Order
   - Move "Optical" to first position

4. Start VM and Run:
   ```
   python3 autorun.py
   ```

## VMware Setup:
1. Create New VM:
   - Guest OS: Linux > Other Linux 4.x 64-bit
   - Memory: 512MB minimum

2. CD/DVD Settings:
   - Use ISO image file
   - Browse: SundarOS-v1.0-Final.iso

3. Power On and Run:
   ```
   python3 autorun.py
   ```

## QEMU Command Line:
```bash
qemu-system-x86_64 -cdrom SundarOS-v1.0-Final.iso -m 512 -boot d
```

## Expected Experience:
1. VM boots from ISO
2. SundarOS files are accessible
3. Run: python3 autorun.py
4. SundarOS starts in 0.3 seconds
5. Interactive AI-native OS interface

## SundarOS Menu Options:
- 1: AI Assistant Demo
- 2: System Status
- 3: Feature Showcase  
- 4: About SundarOS
- 0: Shutdown

## Troubleshooting:
- Ensure Python 3 is available
- Try different Linux distributions if needed
- Minimum 512MB RAM required
- Enable virtualization in BIOS

Enjoy the future of computing! 🌟
