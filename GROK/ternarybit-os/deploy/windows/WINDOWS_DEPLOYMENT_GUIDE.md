# TernaryBit OS - Windows 11 Deployment Guide

## Quick Start

### Step 1: Get the Script
Copy `tbos-windows-installer.ps1` to your Windows 11 ThinkPad

### Step 2: Run PowerShell as Admin
```
Right-click Start Menu → Windows PowerShell (Admin)
```

### Step 3: Execute
```powershell
# Allow script execution
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

# Run installer
.\tbos-windows-installer.ps1
```

## Installation Commands

### Basic Installation
```powershell
.\tbos-windows-installer.ps1
```

### Silent Installation
```powershell
.\tbos-windows-installer.ps1 -Silent
```

### Custom Location
```powershell
.\tbos-windows-installer.ps1 -InstallPath "D:\TernaryBitOS"
```

### All Options
```powershell
.\tbos-windows-installer.ps1 -Silent -InstallPath "C:\TBOS" -CreateDesktopShortcut:$false
```

## Alternative Methods

### Method 1: Bypass Execution Policy
```powershell
powershell -ExecutionPolicy Bypass -File ".\tbos-windows-installer.ps1"
```

### Method 2: Direct Download & Execute
```powershell
# If script is hosted online
iex ((New-Object System.Net.WebClient).DownloadString('https://your-server/tbos-installer.ps1'))
```

### Method 3: Command Prompt
```cmd
powershell.exe -ExecutionPolicy Bypass -File "tbos-windows-installer.ps1"
```

## Expected Installation

### System Detection
- Windows version compatibility check
- RAM and CPU detection
- Architecture identification (x64/ARM)

### TBOS Configuration
- **High-end ThinkPad**: Full desktop mode with GUI
- **Standard ThinkPad**: Desktop mode with shell
- **Low-spec systems**: Minimal mode

### Installation Locations
- **Admin install**: `C:\Program Files\TernaryBitOS`
- **User install**: `%APPDATA%\TernaryBitOS`
- **Config**: `%APPDATA%\TernaryBitOS\config`

### Features Installed
- `tbos.exe` - Main TBOS executable
- `tbos-shell.exe` - Interactive shell
- `tbos-gui.exe` - GUI interface (if supported)
- Configuration files
- Desktop shortcut
- PATH integration

## Post-Installation

### Verify Installation
```powershell
# Check version
tbos --version

# Start TBOS
tbos

# View help
tbos --help
```

### TBOS Commands
```
tbos> help      # Show commands
tbos> status    # System status
tbos> steppps   # STEPPPS framework
tbos> features  # Available features
tbos> exit      # Exit TBOS
```

## Troubleshooting

### Execution Policy Issues
```powershell
# Temporary bypass
Set-ExecutionPolicy -ExecutionPolicy Unrestricted -Scope Process

# Permanent user-level
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Permission Issues
- Run PowerShell as Administrator
- Or install to user directory (automatic fallback)

### PATH Issues
```powershell
# Manual PATH addition
$env:PATH += ";C:\Program Files\TernaryBitOS"

# Permanent PATH (requires admin)
[Environment]::SetEnvironmentVariable("PATH", $env:PATH + ";C:\Program Files\TernaryBitOS", "Machine")
```

### Antivirus Warnings
- Windows Defender may flag new executables
- Add TernaryBitOS folder to exclusions if needed

## Uninstallation

### Remove TBOS
```powershell
# Remove installation directory
Remove-Item -Recurse -Force "C:\Program Files\TernaryBitOS"

# Remove from PATH (manual)
# Edit System Environment Variables → PATH
```

### Clean Registry (if needed)
```powershell
# Remove any registry entries (none created by default)
```

## Advanced Options

### Service Installation
```powershell
# Install as Windows service (optional)
& "C:\Program Files\TernaryBitOS\install-service.ps1"
```

### Scheduled Task
```powershell
# Auto-start TBOS on login
schtasks /create /tn "TernaryBitOS" /tr "C:\Program Files\TernaryBitOS\tbos.exe" /sc onlogon
```

## Integration with Other Devices

Once installed on Windows, TBOS can communicate with:
- Ubuntu TBOS (in VirtualBox)
- Mobile TBOS instances
- Other Windows TBOS installations
- Cross-platform STEPPPS object sharing

## Success Indicators

✅ PowerShell script runs without errors
✅ TBOS executable created and functional
✅ Desktop shortcut works
✅ `tbos` command available in PATH
✅ TBOS interactive shell starts
✅ STEPPPS framework initializes
✅ System status shows correct hardware detection
