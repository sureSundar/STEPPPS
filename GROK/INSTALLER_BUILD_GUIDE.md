# TBOS Installer Build Guide
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**Automated self-extracting installer builders for all platforms**

---

## 🚀 QUICK START

### Build ALL Installers (One Command):

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK
./build-all-installers.sh
```

This will create:
- ✅ Linux self-extracting `.run` file
- ✅ Windows self-extracting `.exe` installer (NSIS)
- ✅ macOS `.pkg` or self-extracting `.sh` installer
- ✅ Android `.apk` package

All installers will be in the `dist/` directory.

---

## 📦 INDIVIDUAL PLATFORM BUILDS

### 🐧 Linux Self-Extracting Installer

```bash
./build-linux-installer.sh
```

**Output**: `dist/TBOS-Detector-Linux-x86_64-v1.0.run`

**What it does:**
- Compiles Linux application
- Creates payload with binary, docs, desktop entry
- Generates self-extracting shell script
- Combines installer script + compressed payload

**How users install:**
```bash
sudo ./TBOS-Detector-Linux-x86_64-v1.0.run
```

**What gets installed:**
- Binary: `/opt/tbos/bin/tbos_detector`
- Symlink: `/usr/local/bin/tbos-detector`
- Desktop entry: `/usr/share/applications/tbos-detector.desktop`
- Docs: `/opt/tbos/share/doc/`

---

### 🪟 Windows Self-Extracting Installer

```bash
./build-windows-installer.sh
```

**Output**: `dist/TBOS-Detector-Windows-x64-v1.0-Setup.exe`

**What it does:**
- Cross-compiles for Windows using MinGW (from Linux!)
- Creates NSIS installer script
- Includes launcher batch file
- Creates proper Windows installer with:
  - Start menu shortcuts
  - Desktop shortcut
  - Uninstaller
  - Registry entries
  - PATH environment variable

**How users install:**
- Double-click the `.exe`
- Follow wizard
- Click Finish

**What gets installed:**
- Binary: `C:\Program Files\TBOS Detector\tbos_detector.exe`
- Launcher: `C:\Program Files\TBOS Detector\TBOS-Detector.bat`
- Desktop shortcut
- Start menu entry
- Added to PATH

---

### 🍎 macOS Self-Extracting Installer

```bash
./build-macos-installer.sh
```

**Output**:
- On macOS: `dist/TBOS-Detector-macOS-universal-v1.0.pkg`
- On Linux: `dist/TBOS-Detector-macOS-universal-v1.0.sh`

**What it does:**
- Compiles macOS application
- Creates proper `.app` bundle with Info.plist
- Generates PKG installer (on macOS)
- Or self-extracting shell script (on Linux)

**How users install:**
- macOS PKG: Double-click and follow wizard
- Shell script: `sudo ./TBOS-Detector-macOS-universal-v1.0.sh`

**What gets installed:**
- App bundle: `/Applications/TBOS Detector.app`
- Command line: `/usr/local/bin/tbos-detector`

---

### 🤖 Android APK Builder

```bash
./build-android-apk.sh
```

**Output**: `dist/TBOS-Detector-Android-v1.0.apk`

**What it does:**
- Builds Android project with Gradle
- Generates release keystore (if needed)
- Signs APK with keystore
- Aligns APK for optimal installation

**How users install:**
```bash
adb install dist/TBOS-Detector-Android-v1.0.apk
```

Or:
- Copy APK to device
- Enable "Unknown Sources" in Settings
- Tap APK to install

---

## 🛠️ DEPENDENCIES

### On Linux Build Machine:

**For Linux installer:**
```bash
sudo apt install tar gzip
# (Usually already installed)
```

**For Windows installer:**
```bash
sudo apt install mingw-w64 nsis
```

**For macOS installer:**
```bash
# No special dependencies for shell script version
# For PKG version, must build on macOS
```

**For Android installer:**
```bash
sudo apt install gradle
# Also needs Android SDK (see TEST_ANDROID_GUIDE.md)
```

**Install all at once:**
```bash
sudo apt install tar gzip mingw-w64 nsis gradle
```

---

## 📋 WHAT EACH SCRIPT DOES

### `build-linux-installer.sh`

```
[1/5] Building application
      ↓ make in app/
[2/5] Preparing payload
      ↓ Copy binary, docs, desktop entry
[3/5] Creating installation script
      ↓ Generate self-extracting shell script
[4/5] Creating payload archive
      ↓ tar czf payload.tar.gz
[5/5] Building self-extracting installer
      ↓ cat install.sh + payload.tar.gz
      ↓ Result: .run file
```

### `build-windows-installer.sh`

```
[1/6] Checking dependencies
      ↓ Install mingw-w64, nsis if needed
[2/6] Cross-compiling for Windows
      ↓ x86_64-w64-mingw32-gcc
[3/6] Preparing payload
      ↓ Copy .exe, batch file, docs
[4/6] Creating NSIS installer script
      ↓ Generate installer.nsi
[5/6] Compiling installer
      ↓ makensis installer.nsi
[6/6] Verifying
      ↓ Result: Setup.exe
```

### `build-macos-installer.sh`

```
[1/5] Building application
      ↓ make in app/
[2/5] Creating application bundle
      ↓ Create .app structure
      ↓ Create Info.plist
      ↓ Create wrapper script
[3/5] Creating package scripts
      ↓ postinstall script
[4/5] Building package
      ↓ pkgbuild (macOS) or self-extract (Linux)
[5/5] Verifying
      ↓ Result: .pkg or .sh
```

### `build-android-apk.sh`

```
[1/5] Checking dependencies
      ↓ Check for gradle
[2/5] Preparing Android project
      ↓ Update version numbers
[3/5] Building APK
      ↓ gradle assembleRelease
[4/5] Signing APK
      ↓ Generate keystore if needed
      ↓ jarsigner
      ↓ zipalign
[5/5] Verifying
      ↓ Result: .apk
```

### `build-all-installers.sh`

```
Master script that runs:
  1. build-linux-installer.sh
  2. build-windows-installer.sh
  3. build-macos-installer.sh
  4. build-android-apk.sh

Creates summary table of all packages
```

---

## 📊 OUTPUT STRUCTURE

After running `./build-all-installers.sh`:

```
/media/sf_vboxshare/lab/STEPPPS/GROK/
├── dist/
│   ├── TBOS-Detector-Linux-x86_64-v1.0.run          (~500 KB)
│   ├── TBOS-Detector-Windows-x64-v1.0-Setup.exe     (~1.5 MB)
│   ├── TBOS-Detector-macOS-universal-v1.0.pkg       (~400 KB)
│   │   or TBOS-Detector-macOS-universal-v1.0.sh
│   └── TBOS-Detector-Android-v1.0.apk               (~5 MB)
│
├── build/
│   ├── linux-installer/
│   ├── windows-installer/
│   ├── macos-installer/
│   └── (temporary build files)
│
└── tbos-release-key.keystore  (Android signing key)
```

---

## 🎯 TESTING INSTALLERS

### Test Linux Installer:

```bash
# In a VM or test machine
chmod +x TBOS-Detector-Linux-x86_64-v1.0.run
sudo ./TBOS-Detector-Linux-x86_64-v1.0.run

# Verify installation
tbos-detector --detect
ls /opt/tbos/
```

### Test Windows Installer:

```batch
REM Copy to Windows machine
REM Double-click Setup.exe
REM Click through wizard

REM Verify installation
tbos_detector.exe --detect
dir "C:\Program Files\TBOS Detector"
```

### Test macOS Installer:

```bash
# On macOS
open TBOS-Detector-macOS-universal-v1.0.pkg
# Or
sudo ./TBOS-Detector-macOS-universal-v1.0.sh

# Verify installation
tbos-detector --detect
ls "/Applications/TBOS Detector.app"
```

### Test Android APK:

```bash
# Connect device
adb devices

# Install
adb install TBOS-Detector-Android-v1.0.apk

# Launch
adb shell am start -n com.tbos.detector/.MainActivity
```

---

## 🔐 CODE SIGNING

### Linux:
- Self-extracting shell scripts don't require signing
- Users may need to verify checksum: `sha256sum file.run`

### Windows:
- NSIS installer can be signed with `signtool.exe`
- Requires Windows code signing certificate ($$$)
- Unsigned installers show "Unknown Publisher" warning

### macOS:
- PKG can be signed with Apple Developer certificate
- Requires Apple Developer account ($99/year)
- Unsigned installers show Gatekeeper warning

### Android:
- APK is signed with keystore (automated in script)
- Release keystore is created automatically
- **IMPORTANT**: Backup `tbos-release-key.keystore` for updates!

---

## 📤 DISTRIBUTION

### Option 1: GitHub Releases

```bash
# Create release
gh release create v1.0 \
  --title "TBOS Detector v1.0" \
  --notes "Initial release - Universal hardware detection" \
  dist/*
```

### Option 2: Direct Download Server

```bash
# Upload to web server
scp dist/* user@yourserver.com:/var/www/downloads/tbos/v1.0/
```

### Option 3: Package Repositories

**Linux:**
- Debian: Upload to PPA or personal repository
- Fedora: Submit to Copr
- Arch: Submit to AUR

**macOS:**
- Homebrew: Submit tap formula

**Android:**
- Google Play Store (requires developer account)
- F-Droid (open source apps)

**Windows:**
- Winget community repository
- Chocolatey community repository

---

## 🔄 VERSION UPDATES

To update version number in all scripts:

```bash
# Edit version at top of each script
sed -i 's/VERSION="1.0"/VERSION="1.1"/' build-*.sh
```

Or edit individually:
- `build-linux-installer.sh` → `VERSION="1.1"`
- `build-windows-installer.sh` → `VERSION="1.1"`
- `build-macos-installer.sh` → `VERSION="1.1"`
- `build-android-apk.sh` → `VERSION="1.1"` + `VERSION_CODE="2"`

---

## 🐛 TROUBLESHOOTING

### Error: "mingw-w64-gcc not found"
**Fix**: `sudo apt install mingw-w64`

### Error: "makensis not found"
**Fix**: `sudo apt install nsis`

### Error: "gradle not found"
**Fix**: `sudo apt install gradle`

### Error: "Permission denied"
**Fix**: `chmod +x build-*.sh`

### Windows installer shows "Unknown publisher"
**Solution**: This is normal for unsigned installers. Users can click "More info" → "Run anyway"

### macOS installer shows "Cannot be opened"
**Solution**: Right-click → Open → Click "Open" to bypass Gatekeeper

### Android "Parse error"
**Fix**: Make sure APK is signed correctly and device allows unknown sources

---

## 🎉 SUCCESS CRITERIA

After building, verify:

- [ ] All 4 installer files exist in `dist/`
- [ ] Each installer is non-zero size
- [ ] Linux `.run` is executable
- [ ] Windows `.exe` is executable
- [ ] macOS `.pkg` or `.sh` is executable
- [ ] Android `.apk` is signed
- [ ] No error messages during build

---

## 📞 WHAT'S INCLUDED IN EACH INSTALLER

### All Installers Include:
- ✅ TBOS detector binary/executable
- ✅ README documentation
- ✅ License file (if exists)
- ✅ Easy launcher (GUI or command line)

### Linux Installer:
- ✅ Binary in `/opt/tbos/bin/`
- ✅ Symlink in `/usr/local/bin/`
- ✅ Desktop entry for application menu
- ✅ Documentation in `/opt/tbos/share/doc/`

### Windows Installer:
- ✅ Executable in `C:\Program Files\TBOS Detector\`
- ✅ Batch launcher for easy running
- ✅ Desktop shortcut
- ✅ Start menu entry
- ✅ Uninstaller
- ✅ Added to PATH
- ✅ Registry entries

### macOS Installer:
- ✅ Application bundle in `/Applications/`
- ✅ Command-line symlink in `/usr/local/bin/`
- ✅ Proper Info.plist
- ✅ Launchable from Finder or Terminal

### Android APK:
- ✅ Complete app with native libraries
- ✅ Signed and aligned
- ✅ Ready for Google Play or direct install

---

## 🌟 ADVANCED FEATURES

### Customization:

Edit the scripts to customize:
- Installation directory
- Desktop entry details
- Start menu entries
- Application icon (add your own)
- Installer branding
- Version information

### Build Matrix:

Create different variants:
```bash
# Debug vs Release
# 32-bit vs 64-bit
# With/without dependencies
```

### Continuous Integration:

Set up automated builds:
```yaml
# GitHub Actions workflow
on: [push, release]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - run: ./build-all-installers.sh
      - run: gh release upload ${{ github.ref }} dist/*
```

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**Build Once, Distribute Everywhere!**

---

## 🚀 QUICK REFERENCE

```bash
# Build everything
./build-all-installers.sh

# Build individually
./build-linux-installer.sh
./build-windows-installer.sh
./build-macos-installer.sh
./build-android-apk.sh

# Output location
ls dist/

# Install dependencies
sudo apt install mingw-w64 nsis gradle
```

**Ready to package and distribute TBOS to the world!** 🌍
