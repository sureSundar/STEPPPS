#!/bin/bash
# SundarOS macOS Deployment Script
# Automated installation for macOS systems

set -e

echo "🍎 SundarOS macOS Deployment"
echo "============================"

# Check macOS version
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "❌ This script is for macOS only"
    exit 1
fi

echo "✅ macOS detected: $(sw_vers -productVersion)"

# Install Homebrew if not present
if ! command -v brew &> /dev/null; then
    echo "📦 Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install Python 3.9+
echo "🐍 Installing Python 3.9..."
brew install python@3.9

# Create SundarOS directory
SUNDARIOS_DIR="$HOME/SundarOS"
mkdir -p "$SUNDARIOS_DIR"
cd "$SUNDARIOS_DIR"

# Copy SundarOS files
echo "📁 Setting up SundarOS files..."
cp -r /media/sf_vboxshare/lab/STEPPPS/GROK/* .

# Install Python dependencies
echo "📦 Installing dependencies..."
pip3 install asyncio sqlite3 psutil tkinter

# Create launch script
cat > launch_sundarios_macos.py << 'EOF'
#!/usr/bin/env python3
import os
import sys
import subprocess

def main():
    print("🌟 Starting SundarOS on macOS...")
    
    # Set macOS-specific environment
    os.environ['SUNDARIOS_PLATFORM'] = 'macos'
    os.environ['SUNDARIOS_MODE'] = 'hosted'
    
    # Launch SundarOS
    subprocess.run([sys.executable, 'launch_sundarios.py'])

if __name__ == "__main__":
    main()
EOF

chmod +x launch_sundarios_macos.py

# Create macOS app bundle structure
echo "📱 Creating macOS app bundle..."
mkdir -p SundarOS.app/Contents/{MacOS,Resources}

cat > SundarOS.app/Contents/Info.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>SundarOS</string>
    <key>CFBundleIdentifier</key>
    <string>com.sundarios.os</string>
    <key>CFBundleName</key>
    <string>SundarOS</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
</dict>
</plist>
EOF

cat > SundarOS.app/Contents/MacOS/SundarOS << 'EOF'
#!/bin/bash
cd "$(dirname "$0")/../../../"
python3 launch_sundarios_macos.py
EOF

chmod +x SundarOS.app/Contents/MacOS/SundarOS

echo "✅ SundarOS installed successfully!"
echo "🚀 Launch options:"
echo "   1. Terminal: cd $SUNDARIOS_DIR && python3 launch_sundarios_macos.py"
echo "   2. App: Double-click SundarOS.app"
echo "   3. Spotlight: Search for 'SundarOS'"
