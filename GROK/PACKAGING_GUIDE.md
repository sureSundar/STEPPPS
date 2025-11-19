# TBOS Packaging and Distribution Guide
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**How to package and distribute TBOS to all platforms!**

---

## 📦 OVERVIEW

This guide covers packaging TBOS for distribution on all platforms:
- Linux (`.deb`, `.rpm`, AppImage, Snap)
- Windows (`.exe`, `.msi`, portable ZIP)
- macOS (`.dmg`, `.pkg`, Homebrew)
- Android (`.apk`, Google Play)
- iOS (`.ipa`, App Store)

---

## 🐧 LINUX PACKAGING

### Method 1: Debian Package (.deb)

**For**: Ubuntu, Debian, Linux Mint, Pop!_OS, etc.

#### Step 1: Create Package Structure

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK
mkdir -p packaging/linux/deb/tbos-detector_1.0-1

cd packaging/linux/deb/tbos-detector_1.0-1

# Create directory structure
mkdir -p usr/bin
mkdir -p usr/share/tbos
mkdir -p usr/share/doc/tbos-detector
mkdir -p usr/share/applications
mkdir -p usr/share/icons/hicolor/256x256/apps
mkdir -p DEBIAN
```

#### Step 2: Create DEBIAN/control File

```bash
cat > DEBIAN/control << 'EOF'
Package: tbos-detector
Version: 1.0-1
Section: utils
Priority: optional
Architecture: amd64
Maintainer: TBOS Team <tbos@example.com>
Description: TBOS Universal Hardware Detector
 Universal hardware detection and device classification tool.
 Detects CPU, RAM, and classifies devices into 8 tiers.
 Part of the TBOS Universal Bootloader project.
 .
 Swamiye Saranam Aiyappa
Depends: libc6 (>= 2.31)
Homepage: https://github.com/yourusername/tbos
EOF
```

#### Step 3: Copy Files

```bash
# Copy binary
cp ../../../../app/tbos_detector usr/bin/
chmod 755 usr/bin/tbos_detector

# Copy documentation
cp ../../../../README.md usr/share/doc/tbos-detector/
cp ../../../../LICENSE usr/share/doc/tbos-detector/ 2>/dev/null || echo "No LICENSE file"
gzip -9 -n -c ../../../../CHANGELOG.md > usr/share/doc/tbos-detector/changelog.gz 2>/dev/null || true
```

#### Step 4: Create Desktop Entry

```bash
cat > usr/share/applications/tbos-detector.desktop << 'EOF'
[Desktop Entry]
Name=TBOS Detector
Comment=Universal Hardware Detection
Exec=tbos-detector --detect
Icon=tbos-detector
Terminal=true
Type=Application
Categories=System;Utility;
Keywords=hardware;detection;system;
EOF
```

#### Step 5: Build Package

```bash
cd ..
dpkg-deb --build tbos-detector_1.0-1
```

#### Step 6: Install and Test

```bash
sudo dpkg -i tbos-detector_1.0-1.deb
tbos-detector --detect
```

#### Step 7: Create Installation Script

```bash
cat > install-deb.sh << 'EOF'
#!/bin/bash
# TBOS Detector - Debian Package Installer

echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
echo "Installing TBOS Detector..."

sudo dpkg -i tbos-detector_1.0-1.deb

if [ $? -eq 0 ]; then
    echo "✅ TBOS Detector installed successfully!"
    echo "Run: tbos-detector --detect"
else
    echo "❌ Installation failed"
    exit 1
fi
EOF
chmod +x install-deb.sh
```

---

### Method 2: RPM Package (.rpm)

**For**: Fedora, RHEL, CentOS, openSUSE, etc.

#### Step 1: Install rpm-build

```bash
# Fedora/RHEL
sudo dnf install rpm-build rpmdevtools

# Or openSUSE
sudo zypper install rpm-build
```

#### Step 2: Create RPM Structure

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK
mkdir -p packaging/linux/rpm
cd packaging/linux/rpm

rpmdev-setuptree
# Creates: ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
```

#### Step 3: Create SPEC File

```bash
cat > ~/rpmbuild/SPECS/tbos-detector.spec << 'EOF'
Name:           tbos-detector
Version:        1.0
Release:        1%{?dist}
Summary:        TBOS Universal Hardware Detector

License:        MIT
URL:            https://github.com/yourusername/tbos
Source0:        tbos-detector-1.0.tar.gz

BuildRequires:  gcc make
Requires:       glibc

%description
Universal hardware detection and device classification tool.
Detects CPU, RAM, and classifies devices into 8 tiers.
Part of the TBOS Universal Bootloader project.

Swamiye Saranam Aiyappa

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
install -m 755 tbos_detector $RPM_BUILD_ROOT/%{_bindir}/tbos-detector

%files
%{_bindir}/tbos-detector
%doc README.md
%license LICENSE

%changelog
* Sat Nov 02 2025 TBOS Team <tbos@example.com> - 1.0-1
- Initial package release
EOF
```

#### Step 4: Create Source Tarball

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app
tar czf ~/rpmbuild/SOURCES/tbos-detector-1.0.tar.gz \
    --transform 's,^,tbos-detector-1.0/,' \
    Makefile src/ include/ README.md LICENSE
```

#### Step 5: Build RPM

```bash
rpmbuild -ba ~/rpmbuild/SPECS/tbos-detector.spec

# Package will be in:
# ~/rpmbuild/RPMS/x86_64/tbos-detector-1.0-1.x86_64.rpm
```

#### Step 6: Install and Test

```bash
sudo rpm -i ~/rpmbuild/RPMS/x86_64/tbos-detector-1.0-1.x86_64.rpm
tbos-detector --detect
```

---

### Method 3: AppImage (Universal Linux)

**For**: All Linux distributions

#### Step 1: Download appimagetool

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/linux
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
```

#### Step 2: Create AppDir Structure

```bash
mkdir -p TBOS.AppDir/usr/bin
mkdir -p TBOS.AppDir/usr/lib
mkdir -p TBOS.AppDir/usr/share/applications
mkdir -p TBOS.AppDir/usr/share/icons/hicolor/256x256/apps
```

#### Step 3: Copy Files

```bash
# Copy binary
cp ../../app/tbos_detector TBOS.AppDir/usr/bin/

# Copy dependencies (if any)
ldd ../../app/tbos_detector | grep "=> /" | awk '{print $3}' | while read lib; do
    cp "$lib" TBOS.AppDir/usr/lib/ 2>/dev/null || true
done
```

#### Step 4: Create AppRun Script

```bash
cat > TBOS.AppDir/AppRun << 'EOF'
#!/bin/bash
# TBOS AppImage Runner

SELF=$(readlink -f "$0")
HERE=${SELF%/*}

export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"

exec "${HERE}/usr/bin/tbos_detector" "$@"
EOF
chmod +x TBOS.AppDir/AppRun
```

#### Step 5: Create Desktop File

```bash
cat > TBOS.AppDir/tbos-detector.desktop << 'EOF'
[Desktop Entry]
Name=TBOS Detector
Exec=tbos_detector
Icon=tbos-detector
Type=Application
Categories=Utility;System;
EOF

cp TBOS.AppDir/tbos-detector.desktop TBOS.AppDir/usr/share/applications/
```

#### Step 6: Build AppImage

```bash
./appimagetool-x86_64.AppImage TBOS.AppDir TBOS-Detector-x86_64.AppImage
chmod +x TBOS-Detector-x86_64.AppImage
```

#### Step 7: Test

```bash
./TBOS-Detector-x86_64.AppImage --detect
```

---

### Method 4: Snap Package

**For**: Ubuntu and other Snap-enabled distributions

#### Step 1: Create snapcraft.yaml

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/linux/snap
mkdir -p snap

cat > snap/snapcraft.yaml << 'EOF'
name: tbos-detector
version: '1.0'
summary: TBOS Universal Hardware Detector
description: |
  Universal hardware detection and device classification tool.
  Detects CPU, RAM, and classifies devices into 8 tiers.

  Swamiye Saranam Aiyappa

grade: stable
confinement: strict
base: core22

apps:
  tbos-detector:
    command: bin/tbos_detector
    plugs:
      - system-observe
      - hardware-observe

parts:
  tbos:
    plugin: make
    source: ../../../app
    build-packages:
      - gcc
      - make
    stage-packages:
      - libc6
EOF
```

#### Step 2: Build Snap

```bash
sudo snap install snapcraft --classic
snapcraft
```

#### Step 3: Install and Test

```bash
sudo snap install tbos-detector_1.0_amd64.snap --dangerous
tbos-detector --detect
```

---

## 🪟 WINDOWS PACKAGING

### Method 1: Portable ZIP (Simplest)

#### Step 1: Create Package Structure

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/windows
mkdir -p TBOS-Detector-Win64-v1.0
cd TBOS-Detector-Win64-v1.0

mkdir bin
mkdir docs
```

#### Step 2: Cross-Compile for Windows (from Linux)

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app

# Install cross-compiler
sudo apt install mingw-w64

# Build for Windows
x86_64-w64-mingw32-gcc \
    -I./include \
    -o ../packaging/windows/TBOS-Detector-Win64-v1.0/bin/tbos_detector.exe \
    src/main.c \
    src/hardware_windows.c \
    src/device_classifier.c \
    src/print_utils.c \
    -ladvapi32 -static
```

#### Step 3: Add Documentation

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/windows/TBOS-Detector-Win64-v1.0

cp ../../../README.md docs/
cp ../../../app/README_WINDOWS_SIMPLE.txt docs/
```

#### Step 4: Create Run Script

```bash
cat > TBOS-Detector.bat << 'EOF'
@echo off
echo ╔══════════════════════════════════════════════════════════╗
echo ║  TBOS Universal Hardware Detector                       ║
echo ║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║
echo ╚══════════════════════════════════════════════════════════╝
echo.

bin\tbos_detector.exe --detect

echo.
pause
EOF
```

#### Step 5: Create README

```bash
cat > README.txt << 'EOF'
TBOS Universal Hardware Detector - Windows
🕉️ Swamiye Saranam Aiyappa 🕉️

HOW TO USE:
1. Double-click: TBOS-Detector.bat
2. Or open Command Prompt and run: bin\tbos_detector.exe --detect

FILES:
- bin\tbos_detector.exe  - Main program
- TBOS-Detector.bat      - Easy launcher
- docs\                  - Documentation

No installation required - just run!
EOF
```

#### Step 6: Create ZIP

```bash
cd ..
zip -r TBOS-Detector-Win64-v1.0.zip TBOS-Detector-Win64-v1.0/
```

---

### Method 2: Windows Installer (.exe using NSIS)

#### Step 1: Install NSIS (on Linux)

```bash
sudo apt install nsis
```

#### Step 2: Create NSIS Script

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/windows

cat > tbos-installer.nsi << 'EOF'
!define APP_NAME "TBOS Detector"
!define APP_VERSION "1.0"
!define APP_PUBLISHER "TBOS Project"
!define APP_URL "https://github.com/yourusername/tbos"

Name "${APP_NAME}"
OutFile "TBOS-Detector-Setup-v${APP_VERSION}.exe"
InstallDir "$PROGRAMFILES64\TBOS Detector"
RequestExecutionLevel admin

Page directory
Page instfiles

Section "Install"
    SetOutPath "$INSTDIR"

    File "TBOS-Detector-Win64-v1.0\bin\tbos_detector.exe"
    File "TBOS-Detector-Win64-v1.0\README.txt"

    CreateDirectory "$INSTDIR\docs"
    SetOutPath "$INSTDIR\docs"
    File /r "TBOS-Detector-Win64-v1.0\docs\*.*"

    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateShortcut "$DESKTOP\TBOS Detector.lnk" "$INSTDIR\tbos_detector.exe"

    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "UninstallString" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\tbos_detector.exe"
    Delete "$INSTDIR\README.txt"
    Delete "$INSTDIR\Uninstall.exe"
    Delete "$DESKTOP\TBOS Detector.lnk"
    RMDir /r "$INSTDIR\docs"
    RMDir "$INSTDIR"

    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd
EOF
```

#### Step 3: Build Installer

```bash
makensis tbos-installer.nsi
```

---

### Method 3: MSI Installer (WiX Toolset)

#### Step 1: Install WiX (on Windows or Linux with Wine)

```bash
# On Linux
sudo apt install wine
wget https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311-binaries.zip
unzip wix311-binaries.zip -d wix
```

#### Step 2: Create WiX XML

```bash
cat > tbos.wxs << 'EOF'
<?xml version='1.0' encoding='windows-1252'?>
<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>
  <Product Name='TBOS Detector'
           Id='*'
           UpgradeCode='PUT-GUID-HERE'
           Language='1033'
           Codepage='1252'
           Version='1.0.0'
           Manufacturer='TBOS Project'>

    <Package Id='*'
             Keywords='Installer'
             Description="TBOS Universal Hardware Detector"
             Comments='Swamiye Saranam Aiyappa'
             Manufacturer='TBOS Project'
             InstallerVersion='100'
             Languages='1033'
             Compressed='yes'
             SummaryCodepage='1252' />

    <Media Id='1' Cabinet='tbos.cab' EmbedCab='yes' />

    <Directory Id='TARGETDIR' Name='SourceDir'>
      <Directory Id='ProgramFilesFolder' Name='PFiles'>
        <Directory Id='INSTALLDIR' Name='TBOS Detector'>
          <Component Id='MainExecutable' Guid='PUT-GUID-HERE'>
            <File Id='TBOSDETECTOREXE' Name='tbos_detector.exe'
                  DiskId='1' Source='bin/tbos_detector.exe' />
          </Component>
        </Directory>
      </Directory>
    </Directory>

    <Feature Id='Complete' Level='1'>
      <ComponentRef Id='MainExecutable' />
    </Feature>
  </Product>
</Wix>
EOF
```

#### Step 3: Compile MSI

```bash
wine wix/candle.exe tbos.wxs
wine wix/light.exe tbos.wixobj -out TBOS-Detector-v1.0.msi
```

---

## 🍎 macOS PACKAGING

### Method 1: Application Bundle (.app)

#### Step 1: Create Bundle Structure

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/macos
mkdir -p TBOS-Detector.app/Contents/{MacOS,Resources}
```

#### Step 2: Build Binary

```bash
cd ../../app
make
cp tbos_detector ../packaging/macos/TBOS-Detector.app/Contents/MacOS/
```

#### Step 3: Create Info.plist

```bash
cat > ../packaging/macos/TBOS-Detector.app/Contents/Info.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>TBOS Detector</string>
    <key>CFBundleDisplayName</key>
    <string>TBOS Detector</string>
    <key>CFBundleIdentifier</key>
    <string>com.tbos.detector</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>TBOS</string>
    <key>CFBundleExecutable</key>
    <string>tbos_detector</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHumanReadableCopyright</key>
    <string>Swamiye Saranam Aiyappa</string>
</dict>
</plist>
EOF
```

---

### Method 2: DMG Disk Image

#### Step 1: Create DMG (on macOS)

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/packaging/macos

hdiutil create -volname "TBOS Detector" \
               -srcfolder TBOS-Detector.app \
               -ov -format UDZO \
               TBOS-Detector-v1.0.dmg
```

---

### Method 3: Homebrew Formula

#### Step 1: Create Formula

```ruby
# Formula/tbos-detector.rb
class TbosDetector < Formula
  desc "TBOS Universal Hardware Detector"
  homepage "https://github.com/yourusername/tbos"
  url "https://github.com/yourusername/tbos/archive/v1.0.tar.gz"
  sha256 "PUT-SHA256-HERE"
  license "MIT"

  def install
    cd "app" do
      system "make"
      bin.install "tbos_detector"
    end
  end

  test do
    system "#{bin}/tbos_detector", "--detect"
  end
end
```

---

## 📱 ANDROID PACKAGING

### Method 1: APK via Gradle

#### Step 1: Build APK

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app/android

# Debug APK (for testing)
./gradlew assembleDebug
# Output: app/build/outputs/apk/debug/app-debug.apk

# Release APK (for distribution)
./gradlew assembleRelease
# Output: app/build/outputs/apk/release/app-release-unsigned.apk
```

#### Step 2: Sign APK

```bash
# Generate signing key (one-time)
keytool -genkey -v -keystore tbos-release-key.keystore \
        -alias tbos-key \
        -keyalg RSA -keysize 2048 -validity 10000

# Sign APK
jarsigner -verbose -sigalg SHA256withRSA -digestalg SHA-256 \
          -keystore tbos-release-key.keystore \
          app/build/outputs/apk/release/app-release-unsigned.apk \
          tbos-key

# Align APK
zipalign -v 4 \
         app/build/outputs/apk/release/app-release-unsigned.apk \
         app/build/outputs/apk/release/TBOS-Detector-v1.0.apk
```

---

## 📱 iOS PACKAGING

### Method 1: IPA via Xcode

#### Step 1: Archive App

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app/ios

xcodebuild -workspace TBOS.xcworkspace \
           -scheme TBOS \
           -configuration Release \
           -archivePath build/TBOS.xcarchive \
           archive
```

#### Step 2: Export IPA

```bash
xcodebuild -exportArchive \
           -archivePath build/TBOS.xcarchive \
           -exportPath build/ \
           -exportOptionsPlist ExportOptions.plist
```

---

## 📦 COMPLETE PACKAGING SCRIPT

Create an all-in-one packaging script:

```bash
cat > /media/sf_vboxshare/lab/STEPPPS/GROK/package-all.sh << 'EOF'
#!/bin/bash
# TBOS Universal Packaging Script
# 🕉️ Swamiye Saranam Aiyappa 🕉️

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  TBOS Universal Packager                                ║"
echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

VERSION="1.0"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGING_DIR="$ROOT_DIR/packaging"
DIST_DIR="$ROOT_DIR/dist"

mkdir -p "$DIST_DIR"

echo "📦 Packaging TBOS v$VERSION for all platforms..."
echo ""

# Build application first
echo "🔨 Building application..."
cd "$ROOT_DIR/app"
make clean
make

# Linux packages
echo "🐧 Creating Linux packages..."
# ... (add packaging commands)

# Windows packages
echo "🪟 Creating Windows packages..."
# ... (add packaging commands)

# macOS packages
echo "🍎 Creating macOS packages..."
# ... (add packaging commands)

echo ""
echo "✅ All packages created in: $DIST_DIR"
echo ""
echo "🕉️ Swamiye Saranam Aiyappa 🕉️"
EOF

chmod +x package-all.sh
```

---

## 📤 DISTRIBUTION METHODS

### GitHub Releases

```bash
# Create release
gh release create v1.0 \
  --title "TBOS v1.0" \
  --notes "Initial release" \
  dist/*
```

### Download Server

```bash
# Upload to server
scp dist/* user@yourserver.com:/var/www/downloads/tbos/v1.0/
```

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**Package once, distribute everywhere!**
