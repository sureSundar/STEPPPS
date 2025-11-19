═══════════════════════════════════════════════════════════════════
  TBOS Universal Hardware Detector - Windows Testing
  🕉 Swamiye Saranam Aiyappa 🕉
═══════════════════════════════════════════════════════════════════

QUICK START (3 STEPS):
─────────────────────────────────────────────────────────────────

1. Open File Explorer
2. Navigate to this folder in your shared drive:
   Z:\lab\STEPPPS\GROK\app  (or \\vboxsvr\vboxshare\lab\STEPPPS\GROK\app)
3. Double-click: build_windows.bat

That's it! The program will:
✓ Detect your compiler (MinGW or Visual Studio)
✓ Build the detector
✓ Run the test
✓ Show your hardware info

═══════════════════════════════════════════════════════════════════

IF YOU DON'T HAVE A COMPILER:
─────────────────────────────────────────────────────────────────

Install MinGW (easiest):
1. Download: https://github.com/niXman/mingw-builds-binaries/releases
2. Extract to C:\MinGW
3. Add to PATH: C:\MinGW\bin
4. Try again!

OR

Install Visual Studio Community (free):
1. Download: https://visualstudio.microsoft.com/downloads/
2. Install "Desktop development with C++"
3. Use "Developer Command Prompt for VS"
4. Navigate to this folder and run: build_windows.bat

═══════════════════════════════════════════════════════════════════

WHAT YOU SHOULD SEE:
─────────────────────────────────────────────────────────────────

After running build_windows.bat, you'll see:

╔══════════════════════════════════════════════════════════╗
║     TBOS Universal Hardware Detection Report            ║
║         🕉 Swamiye Saranam Aiyappa 🕉                    ║
╚══════════════════════════════════════════════════════════╝

=== SYSTEM INFORMATION ===
  Operating System: Windows 10.0 Build XXXXX
  Architecture: x86_64

=== CPU INFORMATION ===
  Vendor: GenuineIntel (or AuthenticAMD)
  Cores: X
  Speed: XXXX MHz

=== MEMORY INFORMATION ===
  Total RAM: XXXX MB
  Total RAM: XX.XX GB

=== DEVICE CLASSIFICATION ===
  Device Class: DESKTOP (or WORKSTATION, etc.)

=== RECOMMENDED OS ===
  Recommended: <OS based on your hardware>

═══════════════════════════════════════════════════════════════════

FILES IN THIS FOLDER:
─────────────────────────────────────────────────────────────────

build_windows.bat    ← Double-click this to build and test
test_windows.bat     ← Run after building to test again
README_WINDOWS.txt   ← You are here
README.md            ← Full documentation

src/                 ← Source code
  main.c
  hardware_windows.c
  device_classifier.c
  print_utils.c

include/             ← Headers
  tbos_common.h

═══════════════════════════════════════════════════════════════════

ADVANCED USAGE:
─────────────────────────────────────────────────────────────────

After building, you can run the detector manually:

Full report:
  tbos_detector.exe --detect

JSON output (for server):
  tbos_detector.exe --json

Save to file:
  tbos_detector.exe --json > my_hardware.json

═══════════════════════════════════════════════════════════════════

TROUBLESHOOTING:
─────────────────────────────────────────────────────────────────

Problem: "gcc is not recognized"
Solution: Install MinGW or add it to PATH

Problem: "cl is not recognized"
Solution: Use "Developer Command Prompt for VS"

Problem: Build fails
Solution: Make sure you have src\hardware_windows.c file

Problem: Program crashes
Solution: Send error message for help

═══════════════════════════════════════════════════════════════════

NEXT STEPS AFTER TESTING:
─────────────────────────────────────────────────────────────────

1. Share your results (CPU, RAM, Classification)
2. Test on other Windows PCs if available
3. Compare with Linux results
4. Test the bootloader version (native)

═══════════════════════════════════════════════════════════════════

QUESTIONS?
─────────────────────────────────────────────────────────────────

Check these files for more info:
- README.md (full documentation)
- WINDOWS_TEST_INSTRUCTIONS.md (detailed instructions)
- PLATFORM_SUPPORT.md (all platforms)
- COMPLETE_STATUS.md (project status)

═══════════════════════════════════════════════════════════════════

🕉 Swamiye Saranam Aiyappa 🕉

Universal Detector: ONE tool for ALL devices!
From Calculator to Supercomputer

═══════════════════════════════════════════════════════════════════
