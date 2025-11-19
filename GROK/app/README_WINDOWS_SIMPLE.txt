═══════════════════════════════════════════════════════════════════
  TBOS Universal Detector - Windows Quick Start
  🕉 Swamiye Saranam Aiyappa 🕉
═══════════════════════════════════════════════════════════════════

STEP 1: Install Compiler (ONE TIME ONLY)
─────────────────────────────────────────────────────────────────

Option A - Automatic (Easiest):
  Double-click: setup_windows.bat
  (It downloads and installs MinGW for you)

Option B - Manual:
  1. Download MSYS2: https://www.msys2.org/
  2. Install to C:\msys64
  3. Open MSYS2 terminal
  4. Run: pacman -S mingw-w64-x86_64-gcc
  5. Add C:\msys64\mingw64\bin to PATH

Option C - Visual Studio:
  1. Download VS Community: https://visualstudio.microsoft.com/
  2. Install "Desktop development with C++"
  3. Use "Developer Command Prompt"

═══════════════════════════════════════════════════════════════════

STEP 2: Build and Test (EVERY TIME)
─────────────────────────────────────────────────────────────────

Double-click: build_windows.bat

That's it!

═══════════════════════════════════════════════════════════════════

WHAT YOU'LL SEE:
─────────────────────────────────────────────────────────────────

╔══════════════════════════════════════════════════════════╗
║     TBOS Universal Hardware Detection Report            ║
╚══════════════════════════════════════════════════════════╝

=== SYSTEM INFORMATION ===
  Operating System: Windows 10.0 Build XXXXX
  Architecture: x86_64

=== CPU INFORMATION ===
  Vendor: Your CPU vendor
  Cores: Your core count
  Speed: Your CPU speed

=== MEMORY INFORMATION ===
  Total RAM: Your RAM in GB

=== DEVICE CLASSIFICATION ===
  Device Class: DESKTOP (or WORKSTATION, etc.)

═══════════════════════════════════════════════════════════════════

FILES:
─────────────────────────────────────────────────────────────────

setup_windows.bat           ← Run ONCE to install compiler
build_windows.bat           ← Run to build and test
test_windows.bat            ← Run to test again
README_WINDOWS_SIMPLE.txt   ← You are here
INSTALL_MINGW.md            ← Detailed install guide

═══════════════════════════════════════════════════════════════════

TROUBLESHOOTING:
─────────────────────────────────────────────────────────────────

Problem: "No compiler found"
Fix: Run setup_windows.bat first

Problem: "gcc is not recognized"
Fix: Restart Command Prompt after installing MinGW

Problem: Download fails
Fix: Download MinGW manually from https://www.msys2.org/

═══════════════════════════════════════════════════════════════════

🕉 Swamiye Saranam Aiyappa 🕉

Quick Start: Just run setup_windows.bat!
