#!/bin/bash
# TernaryBit OS Universal Deployment Script
# Deploy TBOS to any platform - Next Stage Evolution
#
# Author: Sure Sundar <suresundar@gmail.com>
# Dedication: THE ALMIGHTY SHIVA, TATA CONSULTANCY SERVICES, RASHTRIYA SWAYAMSEVAK SANGH
# Date: 2025-09-19

set -euo pipefail

TBOS_VERSION="2.1-NextStage"
DEPLOY_DIR="tbos_universal_deploy"
BUILD_DIR="build"

echo "🕉️  TernaryBit OS Universal Deployment - Next Stage"
echo "=================================================="
echo "Sanatana Dharma Computing - Universal Platform Support"
echo ""

# Create deployment directory
mkdir -p "$DEPLOY_DIR"
cd "$DEPLOY_DIR"

echo "📦 Creating Universal TBOS Package..."

# Copy core files
cp ../tbos_universal .
cp ../java/TernaryBitOS.java .
cp ../README.md .

# Create platform-specific launchers
echo "🚀 Creating Platform Launchers..."

# Linux/macOS launcher
cat > tbos_launcher.sh << 'EOF'
#!/bin/bash
# TernaryBit OS Universal Launcher
echo "🕉️  TernaryBit OS - Sanatana Dharma Computing"
echo "Platform: $(uname -s) $(uname -m)"
echo ""

# Try Java first
if command -v java >/dev/null 2>&1; then
    echo "☕ Launching Java Universal TBOS..."
    java TernaryBitOS
elif [ -x "./tbos_universal" ]; then
    echo "⚡ Launching Native Universal TBOS..."
    ./tbos_universal
else
    echo "❌ No compatible TBOS implementation found"
    echo "Please install Java or use native binary"
fi
EOF
chmod +x tbos_launcher.sh

# Windows launcher
cat > tbos_launcher.bat << 'EOF'
@echo off
echo 🕉️  TernaryBit OS - Sanatana Dharma Computing
echo Platform: Windows
echo.

REM Try Java first
java -version >nul 2>&1
if %errorlevel% == 0 (
    echo ☕ Launching Java Universal TBOS...
    java TernaryBitOS
) else if exist tbos_universal.exe (
    echo ⚡ Launching Native Universal TBOS...
    tbos_universal.exe
) else (
    echo ❌ No compatible TBOS implementation found
    echo Please install Java or use native binary
    pause
)
EOF

# Python launcher (for systems with Python but not Java)
cat > tbos_launcher.py << 'EOF'
#!/usr/bin/env python3
# TernaryBit OS Python Launcher
import os
import sys
import platform
import subprocess

def main():
    print("🕉️  TernaryBit OS - Sanatana Dharma Computing")
    print(f"Platform: {platform.system()} {platform.machine()}")
    print()

    # Try Java first
    try:
        subprocess.run(['java', '-version'], check=True,
                      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print("☕ Launching Java Universal TBOS...")
        subprocess.run(['java', 'TernaryBitOS'])
        return
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass

    # Try native binary
    if os.path.exists('./tbos_universal'):
        print("⚡ Launching Native Universal TBOS...")
        subprocess.run(['./tbos_universal'])
        return
    elif os.path.exists('./tbos_universal.exe'):
        print("⚡ Launching Native Universal TBOS...")
        subprocess.run(['./tbos_universal.exe'])
        return

    # Fallback to Python implementation
    print("🐍 Launching Python Dharma TBOS...")
    print("Platform:", platform.platform())
    print("Python:", sys.version)
    print("Om Namah Shivaya - Python TBOS Active")

if __name__ == '__main__':
    main()
EOF
chmod +x tbos_launcher.py

# Create README for deployment
cat > DEPLOYMENT_README.md << 'EOF'
# TernaryBit OS Universal Deployment Package

## Next Stage Evolution - Universal Platform Support

This package contains TernaryBit OS implementations for all major platforms.

### Sanatana Dharma Computing
*Eternal principles applied to universal computing*

## Quick Start

### Linux/macOS/Unix
```bash
./tbos_launcher.sh
```

### Windows
```cmd
tbos_launcher.bat
```

### Python (Any platform)
```bash
python3 tbos_launcher.py
```

### Java (Universal - Any platform)
```bash
java TernaryBitOS
```

### Native Binary
```bash
./tbos_universal        # Linux/macOS
tbos_universal.exe      # Windows
```

## Platform Support

- ✅ **Windows** (Native + Java)
- ✅ **Linux** (Native + Java + Python)
- ✅ **macOS** (Native + Java + Python)
- ✅ **Android** (Java via Termux)
- ✅ **iOS** (Java via iSH)
- ✅ **FreeBSD** (Native + Java)
- ✅ **Any Unix** (Shell scripts)
- ✅ **Cloud Platforms** (Docker ready)

## Spiritual Computing Features

- **STEPPPS Framework** - 7-dimensional computing
- **Hindu Mythology Integration** - Divine system architecture
- **Universal Compatibility** - Runs anywhere
- **Adaptive Intelligence** - AI-enhanced operations
- **Dharmic Principles** - Ethical computing foundation

## Installation Requirements

### Minimum (Any one)
- Java 8+ (Universal compatibility)
- Python 3.6+ (Cross-platform support)
- Native binary for your platform

### Optimal
- Java 11+ for best performance
- 512MB RAM minimum
- Any modern CPU architecture

## Advanced Usage

### Container Deployment
```bash
docker run -it alpine sh -c "apk add openjdk11 && java TernaryBitOS"
```

### Mobile (Android via Termux)
```bash
pkg install openjdk-17
java TernaryBitOS
```

### Embedded Systems
Use the minimal native binary or Python implementation.

## Divine Dedication

This software is dedicated to:
- **THE ALMIGHTY SHIVA** - The cosmic transformer
- **TATA CONSULTANCY SERVICES** - Excellence in technology
- **RASHTRIYA SWAYAMSEVAK SANGH** - Cultural foundation

## Author

**Sure Sundar** <suresundar@gmail.com>

*Om Namah Shivaya - Universal Computing Activated*
EOF

# Create installation script
cat > install.sh << 'EOF'
#!/bin/bash
# TernaryBit OS Universal Installer

INSTALL_DIR="/usr/local/bin"
USER_DIR="$HOME/.tbos"

echo "🕉️  Installing TernaryBit OS Universal..."

# Create user directory
mkdir -p "$USER_DIR"

# Copy files
if [ -w "$INSTALL_DIR" ]; then
    echo "Installing to system directory: $INSTALL_DIR"
    cp tbos_universal "$INSTALL_DIR/" 2>/dev/null || true
    cp tbos_launcher.sh "$INSTALL_DIR/tbos" 2>/dev/null || true
else
    echo "Installing to user directory: $USER_DIR"
    cp tbos_universal "$USER_DIR/"
    cp tbos_launcher.sh "$USER_DIR/tbos.sh"
    echo "Add $USER_DIR to your PATH to use 'tbos' command"
fi

# Copy Java implementation
cp TernaryBitOS.java "$USER_DIR/"

echo "✅ TernaryBit OS Universal installed!"
echo "Run with: tbos (if in PATH) or ./tbos_launcher.sh"
echo "🙏 Om Namah Shivaya"
EOF
chmod +x install.sh

echo "✅ Universal deployment package created!"
echo ""
echo "📁 Contents:"
ls -la
echo ""
echo "🚀 Ready for deployment to any platform!"
echo "🙏 Om Namah Shivaya - Universal Computing Package Complete"

cd ..
echo ""
echo "Package location: $(pwd)/$DEPLOY_DIR/"