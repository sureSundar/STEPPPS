# SundarOS VM Setup Guide

## VirtualBox:
1. New VM: Linux > Other Linux (64-bit)
2. Memory: 512MB minimum
3. Storage: Attach SundarOS-v1.0-VM-bootable.iso to optical drive
4. System > Boot Order: Optical first
5. Start VM

## VMware:
1. New VM: Linux > Other Linux 4.x 64-bit
2. Memory: 512MB
3. CD/DVD: Use ISO image file
4. Power on

## Expected Boot:
1. Boot menu appears
2. Select "SundarOS v1.0 - AI-Native Operating System"
3. System boots in 0.3 seconds
4. Interactive SundarOS environment ready

## Commands in SundarOS:
- help: Show available commands
- status: System status
- demo: Show SundarOS features
- ai: Interact with AI assistant
- exit: Shutdown system

## Troubleshooting:
- Try "Live Mode" if normal boot fails
- Use "Safe Mode" for minimal system
- Ensure VM has 512MB+ RAM
- Enable virtualization in host BIOS
