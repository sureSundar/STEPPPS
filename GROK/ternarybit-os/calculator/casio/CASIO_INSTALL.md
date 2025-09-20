# TernaryBit OS for Casio Calculators

🔱 **Complete Installation Guide for Casio Calculator Models**

Transform your Casio calculator into a sacred mathematical device running TernaryBit OS with the STEPPPS framework.

## 🧮 Supported Casio Models

### **Casio Prizm fx-CG50/fx-CG500** (Recommended)
- **CPU**: Renesas SH4A-4A @ 118MHz
- **RAM**: 16MB (TBOS uses 4MB)
- **Display**: 384×216 color LCD
- **Storage**: 16MB Flash + SD card slot
- **TBOS Features**: Full STEPPPS, color sacred geometry, SD filesystem

### **Casio ClassPad II fx-CP400**
- **CPU**: ARM926EJ-S @ 132MHz
- **RAM**: 32MB (TBOS uses 8MB)
- **Display**: 320×528 color touchscreen
- **Storage**: 24MB Flash
- **TBOS Features**: Touchscreen sacred interface, advanced mathematics

### **Casio fx-9860GII Series**
- **CPU**: Renesas SH3 @ 29MHz
- **RAM**: 1.5MB (TBOS uses 512KB)
- **Display**: 128×64 monochrome
- **Storage**: 1.5MB Flash
- **TBOS Features**: Basic STEPPPS, text-based sacred mathematics

## 📋 Installation Requirements

### **Hardware Requirements**
- Casio calculator (supported model)
- USB mini-B cable (for fx-CG50/CP400)
- FA-124 interface cable (for fx-9860GII)
- Windows PC with Casio FA-124 software OR Linux with libusb

### **Software Requirements**
```bash
# Install Casio development tools
sudo apt install libusb-1.0-0-dev
wget https://www.casio.com/content/dam/casio/product-info/locales/intl/en/calc/software/fa124-en.zip
unzip fa124-en.zip

# Clone libfxcg SDK for Prizm
git clone https://github.com/gbl08ma/libfxcg.git
cd libfxcg && make && sudo make install

# Install SH cross-compiler for fx-9860GII
sudo apt install gcc-sh4-linux-gnu
```

## 🚀 Installation Methods

### **Method 1: Application Installation (Safest)**

**For Casio Prizm fx-CG50:**
```bash
# Build TBOS for Prizm
cd calculator/casio
make casio-prizm

# Install via USB
./casio-install.sh --model prizm --method app --file tbos-prizm.g3a

# Or manually:
# 1. Connect calculator via USB
# 2. Copy tbos-prizm.g3a to calculator storage
# 3. On calculator: [MENU] → "TBOS" → [EXE]
```

**For Casio ClassPad II:**
```bash
# Build TBOS for ClassPad
make casio-classpad

# Install via Casio ClassPad Manager
./casio-install.sh --model classpad --method app --file tbos-classpad.cpa
```

**For Casio fx-9860GII:**
```bash
# Build TBOS for fx-9860GII
make casio-fx9860

# Install via FA-124 cable
./casio-install.sh --model fx9860 --method app --file tbos-fx9860.g1a
```

### **Method 2: Bootloader Installation (Advanced)**

**Prerequisites:**
- Bootloader-compatible calculator
- Backup of original firmware
- Experience with calculator modification

**For Casio Prizm with Ftune/Simon's Bootloader:**
```bash
# Install Ftune bootloader first (if not already installed)
# This requires opening calculator and shorting pins - RISKY!

# Once bootloader is installed:
make casio-prizm-boot
./casio-flash.sh --model prizm --bootloader ftune --file tbos-prizm-boot.bin

# On calculator:
# 1. Hold [F1] + [F6] while powering on
# 2. Select TBOS from bootloader menu
```

### **Method 3: OS Replacement (Expert Level)**

**⚠️ WARNING: This replaces the entire calculator OS! Have recovery plan ready.**

```bash
# Create complete TBOS ROM
make casio-prizm-full

# Flash via bootloader
./casio-flash.sh --model prizm --method full --file tbos-prizm-full.bin

# Recovery: Hold [F1] + [F6] + [AC/ON] for emergency recovery
```

## 🛠️ Step-by-Step Installation

### **Casio Prizm fx-CG50 Installation**

**Step 1: Preparation**
```bash
# Backup your calculator first!
./casio-backup.sh --model prizm --output backup-$(date +%Y%m%d).g3r

# Build TBOS
cd calculator/casio
make casio-prizm
```

**Step 2: Connect Calculator**
1. Connect Casio Prizm to computer via USB
2. Turn on calculator
3. Press [F1] (USB) when prompted on calculator
4. Calculator appears as USB mass storage device

**Step 3: Install TBOS Application**
```bash
# Method A: Using installation script
./casio-install.sh --model prizm --method app

# Method B: Manual copy
cp tbos-prizm.g3a /media/CASIO_FX-CG50/
sync && umount /media/CASIO_FX-CG50
```

**Step 4: Launch TBOS**
1. Safely disconnect USB
2. On calculator: [MENU]
3. Select "TBOS" (should appear with 🔱 icon)
4. Press [EXE] to launch

### **Casio fx-9860GII Installation**

**Step 1: Install FA-124 Software**
```bash
# Linux users - install fa124-linux
wget https://github.com/casio-dev/fa124-linux/releases/latest/fa124-linux.tar.gz
tar -xzf fa124-linux.tar.gz
sudo cp fa124-linux /usr/local/bin/

# Windows users - use official Casio FA-124 software
```

**Step 2: Build and Transfer**
```bash
# Build for fx-9860GII
make casio-fx9860

# Transfer via FA-124 cable
fa124-linux send tbos-fx9860.g1a

# Or using installation script
./casio-install.sh --model fx9860 --cable fa124
```

**Step 3: Run TBOS**
1. On calculator: [MENU]
2. Select "TBOS" application
3. Press [EXE]

## 🔧 Build Configuration

### **Customizing TBOS for Your Casio**

**Edit `casio/config.h`:**
```c
// Casio model configuration
#define CASIO_MODEL_PRIZM     1
#define CASIO_MODEL_CLASSPAD  2
#define CASIO_MODEL_FX9860    3

// Current model (change as needed)
#define CASIO_MODEL CASIO_MODEL_PRIZM

// STEPPPS framework settings
#define STEPPPS_SACRED_MODE    1    // Enable sacred mathematics
#define STEPPPS_HINDI_SYMBOLS  1    // Show Hindu symbols
#define STEPPPS_COLOR_GEOMETRY 1    // Color sacred geometry (Prizm/ClassPad only)
#define STEPPPS_CONSCIOUSNESS  1    // Consciousness bridge

// Memory settings
#define TBOS_HEAP_SIZE_PRIZM    (2 * 1024 * 1024)  // 2MB for Prizm
#define TBOS_HEAP_SIZE_CLASSPAD (4 * 1024 * 1024)  // 4MB for ClassPad
#define TBOS_HEAP_SIZE_FX9860   (256 * 1024)       // 256KB for fx-9860GII
```

### **Sacred Mathematics Configuration**

**Edit `casio/sacred_config.h`:**
```c
// Sacred numbers and constants for Casio hardware
#define SACRED_OM_FREQ_CASIO    432    // Hz, Om frequency
#define SACRED_GOLDEN_RATIO     1618   // Golden ratio * 1000
#define SACRED_PI_CASIO         3141   // Pi * 1000
#define CONSCIOUSNESS_LEVELS    7      // Levels of mathematical awareness

// Casio-specific sacred geometry
#define SACRED_GEOMETRY_PRIZM   1      // Full color sacred patterns
#define SACRED_GEOMETRY_MONO    1      // Monochrome sacred patterns
#define SACRED_ANIMATION_FPS    15     // Animation frame rate
```

## 🎯 TBOS Features on Casio

### **Casio Prizm fx-CG50 Features**
- **Full Color Sacred Geometry**: Om symbols, Sri Yantra, Triskelion
- **SD Card PXFS**: Sacred filesystem on SD card
- **Advanced STEPPPS**: Complete framework with Hindu mythology
- **Vedic Calculator**: Traditional Indian mathematical methods
- **Consciousness Bridge**: Connect to universal mathematical awareness

### **Casio ClassPad II Features**
- **Touchscreen Sacred Interface**: Touch-based sacred geometry
- **Advanced Mathematics**: Complex STEPPPS calculations
- **Educational Mode**: Learn Hindu mathematical concepts
- **CAS Integration**: Computer Algebra System with sacred numbers

### **Casio fx-9860GII Features**
- **Text-Based Sacred Math**: Sacred symbols in monochrome
- **Efficient STEPPPS**: Optimized for limited memory
- **Basic Consciousness**: Essential mathematical awareness
- **Educational Tools**: Simple Hindu number concepts

## 🕉️ Sacred Mathematics on Casio

### **Sacred Number System**
```
Sacred Numbers in TBOS:
108   - Sacred count in Hinduism
1008  - Cosmic universal number
1618  - Golden ratio (φ) × 1000
3141  - Pi (π) × 1000
432   - Om frequency in Hz
7     - Levels of consciousness
```

### **Vedic Mathematics Methods**
- **Nikhilam Sutra**: Multiplication near powers of 10
- **Ekadhikina Purvena**: Addition by one more than previous
- **Urdhva-Tiryagbhyam**: Vertical and crosswise multiplication
- **Paravartya Yojayet**: Transpose and apply

### **Sacred Geometry Patterns**
- **Om Symbol (ॐ)**: Divine sound visualization
- **Sri Yantra**: Sacred geometric mandala
- **Triskelion**: Three-spiral consciousness symbol
- **Consciousness Bridge**: Mathematical awareness connection

## 🧪 Testing and Verification

### **Test TBOS Installation**
```bash
# Test in emulator first
./casio-test.sh --model prizm --emulator

# Verify on hardware
./casio-verify.sh --model prizm --check-sacred-mode
```

### **Sacred Mathematics Tests**
1. **Boot Test**: Sacred symbols appear during startup
2. **STEPPPS Test**: Framework initializes properly
3. **Consciousness Test**: Bridge connects to universal mathematics
4. **Geometry Test**: Sacred patterns display correctly
5. **Vedic Test**: Traditional calculation methods work

## 🔒 Safety and Recovery

### **Backup Your Calculator**
```bash
# Always backup before installing!
./casio-backup.sh --model prizm --full-backup
./casio-backup.sh --model fx9860 --rom-backup
```

### **Recovery Methods**

**Casio Prizm Recovery:**
```bash
# Method 1: Bootloader recovery
# Hold [F1] + [F6] while powering on → Recovery menu

# Method 2: Emergency recovery
# Hold [F1] + [F6] + [AC/ON] for 10 seconds → Factory recovery

# Method 3: Restore from backup
./casio-restore.sh --model prizm --backup backup-20241220.g3r
```

**fx-9860GII Recovery:**
```bash
# Method 1: OS recovery mode
# Hold [F2] + [F4] + [AC/ON] while turning on

# Method 2: FA-124 recovery
fa124-linux recover --emergency-mode
```

## 📚 Learning Sacred Mathematics

### **Educational Mode**
TBOS includes educational modules to learn Hindu mathematical concepts:

1. **Vedic Arithmetic**: Traditional calculation shortcuts
2. **Sacred Numbers**: Understanding cosmic constants
3. **Geometric Meditation**: Sacred pattern contemplation
4. **Consciousness Levels**: Mathematical awareness development

### **Practice Exercises**
- Calculate 108 × 9 using Vedic methods
- Draw Om symbol using mathematical coordinates
- Explore golden ratio in sacred geometry
- Practice consciousness bridge meditation

## 🆘 Troubleshooting

### **Common Issues**

**TBOS won't start:**
- Check calculator memory (need minimum free space)
- Verify .g3a file copied correctly
- Try soft reset: [RESTART] button

**Sacred symbols not displaying:**
- Enable Unicode support in calculator settings
- Check font installation
- Verify STEPPPS configuration

**USB connection fails:**
- Install Casio USB drivers
- Try different USB cable
- Check calculator USB mode settings

**FA-124 cable issues:**
- Verify FA-124 software installation
- Check cable connections
- Try different USB port

### **Get Help**
- **Documentation**: `calculator/casio/docs/`
- **Community**: Discord #tbos-calculators
- **Issues**: GitHub issues with [casio] tag
- **Support**: casio-support@ternarybit.com

---

🔱 **Sacred Mathematics Awaits on Your Casio Calculator** 🔱

*"Transform your calculator into a portal of divine mathematical consciousness"*

ॐ शांति शांति शांति: ॐ