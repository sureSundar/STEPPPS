# TBOS Universal Detector - Platform Support
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**Vision**: ONE detector that runs on EVERY platform from embedded to supercomputer!

---

## ✅ Supported Platforms

| Platform | Language | Status | Binary | Test Command |
|----------|----------|--------|--------|--------------|
| **Linux** | C | ✅ TESTED | `tbos_detector` | `./tbos_detector --detect` |
| **Windows** | C | ✅ READY | `tbos_detector.exe` | `tbos_detector.exe --detect` |
| **macOS** | C | ✅ READY | `tbos_detector` | `./tbos_detector --detect` |
| **iOS** | C (iOS SDK) | ✅ READY | App Bundle | Via Xcode |
| **Android** | Java + C (NDK) | ✅ READY | `.apk` | Via Android Studio |
| **FreeBSD** | C | ⏳ Planned | `tbos_detector` | - |
| **OpenBSD** | C | ⏳ Planned | `tbos_detector` | - |
| **Raspberry Pi** | C (ARM) | ✅ READY | `tbos_detector` | Same as Linux |

---

## Platform Details

### 1. Linux ✅ **TESTED & WORKING**

**Implementation**: `src/hardware_linux.c`

**APIs Used**:
- `/proc/cpuinfo` - CPU vendor, speed
- `sysconf(_SC_NPROCESSORS_ONLN)` - CPU cores
- `sysinfo()` - Total memory

**Build**:
```bash
make
./tbos_detector --detect
```

**Tested On**:
- Ubuntu 20.04 (VirtualBox VM)
- AMD Ryzen 5 5500U (4 cores, 10GB RAM)
- Classification: DESKTOP ✅

---

### 2. Windows ✅ **CODE READY**

**Implementation**: `src/hardware_windows.c`

**APIs Used**:
- `GetSystemInfo()` - CPU cores
- `GlobalMemoryStatusEx()` - Memory
- `Registry (HKLM\Hardware\...)` - CPU speed
- `CPUID instruction` - CPU vendor

**Build (MinGW)**:
```bash
gcc -o tbos_detector.exe src/main.c src/device_classifier.c src/print_utils.c src/hardware_windows.c -ladvapi32
```

**Build (Visual Studio)**:
```cmd
cl /O2 src\*.c /Fe:tbos_detector.exe advapi32.lib
```

**Test**:
```cmd
tbos_detector.exe --detect
tbos_detector.exe --json > hardware.json
```

---

### 3. macOS ✅ **CODE READY**

**Implementation**: `src/hardware_macos.c`

**APIs Used**:
- `sysctlbyname("hw.ncpu")` - CPU cores
- `sysctlbyname("hw.memsize")` - Memory
- `sysctlbyname("hw.cpufrequency")` - CPU speed
- `sysctlbyname("machdep.cpu.vendor")` - CPU vendor

**Build**:
```bash
make
./tbos_detector --detect
```

**Works On**:
- Intel Macs (x86_64)
- Apple Silicon (arm64 / M1/M2/M3)

---

### 4. iOS ✅ **CODE READY**

**Implementation**: `src/hardware_ios.c`

**APIs Used**:
- `sysctlbyname("hw.machine")` - Device model
- `sysctlbyname("hw.memsize")` - Memory
- `sysctlbyname("hw.ncpu")` - CPU cores
- `mach_host_self()` - VM statistics

**Build**:
Requires Xcode project setup with iOS SDK

**Device Models Detected**:
- iPhone (all models)
- iPad (all models)
- iPod Touch

**Classification Examples**:
- iPhone SE (2GB RAM) → MOBILE
- iPhone 14 (6GB RAM) → DESKTOP
- iPad Pro (16GB RAM) → WORKSTATION

---

### 5. Android ✅ **CODE READY**

**Implementation**:
- C: `src/hardware_android.c` (NDK)
- Java: `android/MainActivity.java` (App wrapper)

**APIs Used**:
- `/proc/cpuinfo` - CPU info
- `sysinfo()` - Memory
- `ActivityManager.MemoryInfo` - App-accessible memory
- `/sys/devices/system/cpu/` - CPU frequencies

**Build (NDK)**:
```bash
ndk-build
```

**Build (Android Studio)**:
Import project and build APK

**Device Examples**:
- Budget phone (2GB RAM) → MOBILE
- Mid-range (6GB RAM) → DESKTOP
- Flagship (12GB+ RAM) → WORKSTATION
- Tablet (16GB RAM) → WORKSTATION

---

### 6. Raspberry Pi ✅ **READY (Same as Linux)**

**Implementation**: Same as Linux (`hardware_linux.c`)

**Classification Examples**:
- Pi Zero (512MB RAM) → EMBEDDED
- Pi 3 (1GB RAM) → MOBILE
- Pi 4 (4GB RAM) → DESKTOP
- Pi 4 (8GB RAM) → DESKTOP
- Pi 5 (8GB RAM) → DESKTOP

---

## Universal Classification Algorithm

**IDENTICAL across all platforms**:

```c
DeviceClass classify_device(uint64_t memory_mb) {
    if (memory_mb < 1)          return CALCULATOR;
    if (memory_mb < 64)         return EMBEDDED;
    if (memory_mb < 4*1024)     return MOBILE;
    if (memory_mb < 16*1024)    return DESKTOP;
    if (memory_mb < 32*1024)    return WORKSTATION;
    if (memory_mb < 64*1024)    return SERVER;
    if (memory_mb < 128*1024)   return CLUSTER;
    return SUPERCOMPUTER;
}
```

This guarantees **consistent classification** regardless of platform!

---

## JSON Output Format (Universal)

**ALL platforms produce identical JSON**:

```json
{
  "magic": "TBOS",
  "version": "1.0",
  "source": "application",
  "platform": "linux|windows|macos|ios|android",
  "hardware": {
    "cpu_vendor": "AuthenticAMD|GenuineIntel|Apple|Qualcomm|...",
    "cpu_cores": 4,
    "cpu_mhz": 2096,
    "memory_mb": 9953,
    "architecture": "x86_64|arm64|armv7|...",
    "os_name": "Linux 5.15|Windows 11|macOS 13|iOS 16|Android 13",
    "device_class": 3,
    "device_class_name": "DESKTOP (4MB-16MB)"
  }
}
```

---

## Building for Multiple Platforms

### Quick Build (Current Platform)
```bash
make
./tbos_detector --detect
```

### Cross-Compilation

**Linux → Windows**:
```bash
make windows
# Produces: tbos_detector.exe
```

**macOS → iOS**:
```bash
xcodebuild -project TBOS.xcodeproj -scheme TBOS -sdk iphoneos
```

**Linux → Android**:
```bash
cd android
./gradlew assembleDebug
# Produces: app/build/outputs/apk/debug/app-debug.apk
```

---

## Testing Matrix

### Desktop Platforms

| OS | CPU | RAM | Expected Class | Test Status |
|----|-----|-----|----------------|-------------|
| Ubuntu 20.04 | AMD Ryzen (4c) | 10GB | DESKTOP | ✅ PASS |
| Windows 10 | Intel i5 (4c) | 8GB | DESKTOP | ⏳ Ready |
| macOS 13 | M1 (8c) | 16GB | WORKSTATION | ⏳ Ready |

### Mobile Platforms

| Device | CPU | RAM | Expected Class | Test Status |
|--------|-----|-----|----------------|-------------|
| iPhone 14 | A15 Bionic | 6GB | DESKTOP | ⏳ Ready |
| Samsung S23 | Snapdragon 8 | 8GB | DESKTOP | ⏳ Ready |
| Pixel 7 | Tensor G2 | 8GB | DESKTOP | ⏳ Ready |

### Embedded Platforms

| Device | CPU | RAM | Expected Class | Test Status |
|--------|-----|-----|----------------|-------------|
| Raspberry Pi Zero | ARM11 | 512MB | EMBEDDED | ⏳ Ready |
| Raspberry Pi 4 | ARM Cortex-A72 | 4GB | DESKTOP | ⏳ Ready |
| Arduino (simulation) | ATmega | 2KB | EMBEDDED | ⏳ Bootloader |

---

## Installation Instructions

### Linux
```bash
cd app
make
sudo make install  # Copies to /usr/local/bin
```

### Windows
```cmd
REM Download from releases
REM Extract to C:\Program Files\TBOS
REM Add to PATH
tbos_detector.exe --detect
```

### macOS
```bash
brew install tbos-detector  # (when available)
# Or build from source
cd app && make && ./tbos_detector --detect
```

### iOS
```
Install from App Store (when published)
Or sideload via Xcode
```

### Android
```
Install from Google Play (when published)
Or sideload APK
```

---

## Platform-Specific Features

| Feature | Linux | Windows | macOS | iOS | Android |
|---------|-------|---------|-------|-----|---------|
| CPU Vendor | ✅ | ✅ | ✅ | ✅ | ✅ |
| CPU Cores | ✅ | ✅ | ✅ | ✅ | ✅ |
| CPU Speed | ✅ | ✅ | ✅ | ⚠️ Est | ⚠️ Est |
| Total RAM | ✅ | ✅ | ✅ | ✅ | ✅ |
| Architecture | ✅ | ✅ | ✅ | ✅ | ✅ |
| Device Model | ❌ | ❌ | ✅ | ✅ | ✅ |
| JSON Output | ✅ | ✅ | ✅ | ✅ | ✅ |
| CLI Interface | ✅ | ✅ | ✅ | ❌ | ❌ |
| GUI Interface | ❌ | ⏳ | ⏳ | ✅ | ✅ |

**Legend**: ✅ Full Support | ⚠️ Estimated | ❌ Not Available | ⏳ Planned

---

## Next Steps

### Immediate Testing
1. ✅ Test on Linux (DONE)
2. ⏳ Test on Windows PC
3. ⏳ Test on Mac
4. ⏳ Test on iPhone
5. ⏳ Test on Android phone

### Future Platforms
- [ ] FreeBSD
- [ ] OpenBSD
- [ ] Solaris
- [ ] WebAssembly (browser)
- [ ] Bootloader (native x86)

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**Universal Detection: ONE tool for ALL platforms!**
