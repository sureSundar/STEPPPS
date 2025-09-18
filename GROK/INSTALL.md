# SundarOS Installation Guide

## Installation Methods

### 1. USB Boot Installation (Recommended)
1. Download SundarOS ISO image
2. Create bootable USB drive
3. Boot from USB and install to hard drive

### 2. Virtual Machine Installation
1. Create new VM with 1GB RAM, 10GB disk
2. Mount SundarOS ISO as CD/DVD
3. Boot VM and install normally

### 3. Dual-Boot Installation
1. Boot from SundarOS USB
2. Choose "Install alongside existing OS"
3. Follow partitioning wizard

### 4. Container Deployment
```bash
docker run -p 8080:8080 sundarios:latest
```

## Post-Installation
- First boot takes 0.3 seconds
- AI consciousness initializes automatically
- Complete setup wizard for personalization
- Device mesh networking activates

## Troubleshooting
- Secure Boot: Disable in BIOS if needed
- Legacy BIOS: Supported with compatibility mode
- Hardware issues: SundarOS auto-detects and adapts

For detailed instructions, visit: https://docs.sundarios.org/install
