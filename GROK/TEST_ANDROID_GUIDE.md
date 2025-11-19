# TBOS Android Testing Guide
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**How to test TBOS hardware detection on Android devices!**

---

## 📱 ANDROID APPLICATION MODE

Since Android devices use ARM processors (not x86), we test the **Application Mode** of TBOS, not the bootloader mode.

---

## 🚀 METHOD 1: Using Android Studio Emulator (EASIEST)

### Step 1: Install Android Studio

Download from: https://developer.android.com/studio

### Step 2: Create Android Project

1. Open Android Studio
2. File → New → Project from Existing Code
3. Navigate to: `/media/sf_vboxshare/lab/STEPPPS/GROK/app/android`
4. Select "Android" as project type

### Step 3: Configure NDK

1. Tools → SDK Manager → SDK Tools
2. Check "NDK (Side by side)"
3. Check "CMake"
4. Click "Apply" to install

### Step 4: Build Project

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app/android
./gradlew build
```

### Step 5: Run in Emulator

1. Click "Run" (green play button)
2. Select or create emulator device
3. Wait for app to launch

### Step 6: See Results

You'll see:
```
╔══════════════════════════════════════════════╗
║  TBOS Universal Hardware Detection          ║
║     Swamiye Saranam Aiyappa                 ║
╚══════════════════════════════════════════════╝

=== DEVICE INFORMATION ===
  Device: Samsung Galaxy S21 (or emulator name)
  Android Version: 13

=== CPU INFORMATION ===
  Processor: ARM Cortex-A78
  Cores: 8
  Architecture: aarch64

=== MEMORY INFORMATION ===
  Total RAM: 8.00 GB

=== DEVICE CLASSIFICATION ===
  Device Class: DESKTOP (4MB-16GB)
```

---

## 🔧 METHOD 2: Command Line Build

### Step 1: Set up NDK

```bash
export ANDROID_NDK_HOME=$HOME/Android/Sdk/ndk/25.1.8937393
export PATH=$PATH:$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
```

### Step 2: Build with NDK

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app

# For ARM64 (most modern Android devices)
aarch64-linux-android30-clang \
    -I./include \
    -o tbos_detector_android \
    src/main.c \
    src/hardware_android.c \
    src/device_classifier.c \
    src/print_utils.c \
    -llog

# For ARMv7 (older Android devices)
armv7a-linux-androideabi30-clang \
    -I./include \
    -o tbos_detector_android_arm32 \
    src/main.c \
    src/hardware_android.c \
    src/device_classifier.c \
    src/print_utils.c \
    -llog
```

### Step 3: Push to Device

```bash
# Connect Android device via USB
# Enable USB debugging in Developer Options

adb push tbos_detector_android /data/local/tmp/
adb shell chmod +x /data/local/tmp/tbos_detector_android
adb shell /data/local/tmp/tbos_detector_android --detect
```

---

## 📲 METHOD 3: Using Real Android Device

### Step 1: Enable Developer Mode

1. Settings → About Phone
2. Tap "Build Number" 7 times
3. Go back → Developer Options
4. Enable "USB Debugging"

### Step 2: Connect Device

```bash
# Connect via USB cable
adb devices
# Should show your device
```

### Step 3: Install APK

```bash
cd /media/sf_vboxshare/lab/STEPPPS/GROK/app/android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

### Step 4: Run App

1. Find "TBOS Detector" in app drawer
2. Tap to launch
3. See hardware detection results

---

## 🧪 METHOD 4: Termux (No Root Required)

### Step 1: Install Termux

Download from: https://f-droid.org/en/packages/com.termux/

### Step 2: Install Compiler in Termux

```bash
# Open Termux app
pkg update
pkg install clang
```

### Step 3: Copy Source Code

```bash
# Copy files to Termux storage
cd ~
mkdir tbos
cd tbos

# Copy from your Linux host via USB or shared folder
# Or clone from git if hosted online
```

### Step 4: Build in Termux

```bash
clang \
    -I./include \
    -o tbos_detector \
    src/main.c \
    src/hardware_android.c \
    src/device_classifier.c \
    src/print_utils.c

./tbos_detector --detect
```

---

## 📊 EXPECTED RESULTS BY DEVICE TYPE

### Budget Smartphone (2GB RAM):
```
Device Class: MOBILE (64KB-4MB)
Class ID: 2
```

### Mid-Range Phone (4-6GB RAM):
```
Device Class: DESKTOP (4MB-16GB)
Class ID: 3
```

### Flagship Phone (8-12GB RAM):
```
Device Class: DESKTOP (4MB-16GB)
Class ID: 3
```

### Gaming Phone (16GB+ RAM):
```
Device Class: WORKSTATION (16GB-32GB)
Class ID: 4
```

### Android Tablet (High-End):
```
Device Class: DESKTOP or WORKSTATION
Class ID: 3 or 4
```

---

## 🎯 TESTING MATRIX

| Device Type | RAM | Expected Class | Status |
|-------------|-----|----------------|--------|
| **Emulator (AVD)** | 2GB | MOBILE | ⏳ Ready to test |
| **Emulator (AVD)** | 8GB | DESKTOP | ⏳ Ready to test |
| **Samsung Galaxy S21** | 8GB | DESKTOP | ⏳ Ready to test |
| **Google Pixel 7** | 8GB | DESKTOP | ⏳ Ready to test |
| **OnePlus 11** | 16GB | WORKSTATION | ⏳ Ready to test |
| **Xiaomi 13 Ultra** | 12GB | DESKTOP | ⏳ Ready to test |

---

## 🐛 TROUBLESHOOTING

### Error: "adb not found"

**Fix**: Add Android SDK platform-tools to PATH:
```bash
export PATH=$PATH:$HOME/Android/Sdk/platform-tools
```

### Error: "device unauthorized"

**Fix**: Check your phone screen for USB debugging authorization prompt and tap "Allow"

### Error: "INSTALL_FAILED_UPDATE_INCOMPATIBLE"

**Fix**: Uninstall old version first:
```bash
adb uninstall com.tbos.detector
```

### Error: "Permission denied" when reading /proc

**Fix**: Some values in `/proc` require root. The app gracefully handles missing permissions and shows available data.

---

## 📸 TAKING SCREENSHOTS

### From Android Studio:

1. Run app in emulator
2. Click camera icon in emulator toolbar
3. Screenshot saved to `~/Pictures`

### From adb:

```bash
adb shell screencap -p /sdcard/tbos_test.png
adb pull /sdcard/tbos_test.png
```

---

## 🔍 COMPARING RESULTS

### Test Different Android Versions:

Create multiple AVD emulators:

```bash
# Android 11 with 2GB RAM
# Android 13 with 8GB RAM
# Android 14 with 16GB RAM
```

**Run TBOS on each and compare classifications!**

---

## 🌟 ADVANCED: Testing on ChromeOS

ChromeOS runs Android apps! You can test TBOS on:
- Chromebooks with Android support
- Chrome OS Flex

Same APK works on both Android phones and ChromeOS!

---

## 🎉 SUCCESS CRITERIA

✅ App builds without errors
✅ App runs on emulator
✅ App runs on real device
✅ CPU vendor detected correctly
✅ Core count detected correctly
✅ RAM amount detected correctly
✅ Device classified correctly
✅ UI displays results clearly

---

## 📞 NEXT STEPS

After successful Android testing:

1. ✅ Test application mode
2. ⏳ Add JSON export for server communication
3. ⏳ Connect to TBOS server
4. ⏳ Receive OS recommendations
5. ⏳ (Future) Root mode for deeper hardware access

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**TBOS runs everywhere - even in your pocket!**

---

**Quick Start**:
1. Install Android Studio
2. Open project from `app/android`
3. Click Run
4. See your Android hardware detected!
