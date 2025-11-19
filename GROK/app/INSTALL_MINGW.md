# Installing MinGW on Windows
## 🕉 Swamiye Saranam Aiyappa 🕉

**MinGW** = Minimalist GNU for Windows (GCC compiler for Windows)

---

## 🚀 OPTION 1: Automatic Install (EASIEST)

Just run this:

```
setup_windows.bat
```

It will:
1. Download MinGW automatically
2. Install to C:\MinGW
3. Add to PATH
4. Build TBOS detector

**That's it!**

---

## 🔧 OPTION 2: Manual Install (MSYS2 - Recommended)

### Step 1: Download MSYS2

Go to: https://www.msys2.org/

Download: `msys2-x86_64-latest.exe`

### Step 2: Install

Run installer, install to: `C:\msys64`

### Step 3: Update MSYS2

Open "MSYS2 MSYS" from Start Menu:

```bash
pacman -Syu
```

Close terminal when prompted, reopen, and run:

```bash
pacman -Su
```

### Step 4: Install MinGW-w64

```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
```

### Step 5: Add to PATH

Add this to your Windows PATH:
```
C:\msys64\mingw64\bin
```

How to add to PATH:
1. Press Windows key
2. Type "environment variables"
3. Click "Edit system environment variables"
4. Click "Environment Variables"
5. Under "System variables", select "Path"
6. Click "Edit"
7. Click "New"
8. Add: `C:\msys64\mingw64\bin`
9. Click OK, OK, OK

### Step 6: Verify

Open new Command Prompt:

```cmd
gcc --version
```

Should show:
```
gcc (GCC) 13.x.x
```

### Step 7: Build TBOS

```cmd
cd Z:\lab\STEPPPS\GROK\app
build_windows.bat
```

---

## 🔧 OPTION 3: Manual Install (Standalone MinGW)

### Step 1: Download

Go to: https://github.com/niXman/mingw-builds-binaries/releases

Download latest:
- `x86_64-XX.X.X-release-posix-seh-ucrt-rt_vXX-revX.7z`

### Step 2: Extract

Extract to: `C:\MinGW`

You should have:
```
C:\MinGW\
  └── bin\
      ├── gcc.exe
      ├── g++.exe
      └── ...
```

### Step 3: Add to PATH

Add to Windows PATH:
```
C:\MinGW\bin
```

(See OPTION 2 Step 5 for how to add to PATH)

### Step 4: Verify

```cmd
gcc --version
```

### Step 5: Build TBOS

```cmd
cd Z:\lab\STEPPPS\GROK\app
build_windows.bat
```

---

## 🪟 OPTION 4: Visual Studio (Alternative)

If you prefer Visual Studio over MinGW:

### Step 1: Download

Go to: https://visualstudio.microsoft.com/downloads/

Download: "Visual Studio Community" (free)

### Step 2: Install

Run installer, select:
- "Desktop development with C++"

### Step 3: Use Developer Command Prompt

Start Menu → Visual Studio → "Developer Command Prompt for VS"

### Step 4: Build TBOS

```cmd
cd Z:\lab\STEPPPS\GROK\app
build_windows.bat
```

(The script will auto-detect Visual Studio)

---

## ✅ Verification

After installing any option above, test:

```cmd
REM Check GCC
gcc --version

REM Or check MSVC
cl
```

Then build:

```cmd
cd Z:\lab\STEPPPS\GROK\app
build_windows.bat
```

---

## 🐛 Troubleshooting

### "gcc is not recognized"

**Cause**: MinGW not in PATH

**Fix**:
1. Add `C:\MinGW\bin` to PATH (see instructions above)
2. Restart Command Prompt
3. Try again

### "Cannot find -ladvapi32"

**Cause**: Library linking issue

**Fix**: This is normal with GCC. The build script handles it.

### Build works but program crashes

**Cause**: Wrong compiler used

**Fix**: Make sure using Windows implementation:
```
src\hardware_windows.c (not hardware_linux.c)
```

---

## 📊 Comparison

| Method | Size | Ease | Best For |
|--------|------|------|----------|
| **Automatic** | ~200MB | ⭐⭐⭐⭐⭐ | Quick testing |
| **MSYS2** | ~300MB | ⭐⭐⭐⭐ | Development |
| **Standalone** | ~200MB | ⭐⭐⭐ | Minimal install |
| **Visual Studio** | ~2GB | ⭐⭐⭐ | Full IDE |

---

## 🎯 Recommended

For TBOS testing, we recommend:

1. **Quick test**: Run `setup_windows.bat` (automatic)
2. **Development**: Install MSYS2 (Option 2)
3. **Professional**: Use Visual Studio (Option 4)

---

## 📞 Need Help?

If installation fails:

1. Check Windows version (needs Windows 7+)
2. Check if antivirus is blocking
3. Try different installation method
4. Share error message for help

---

🕉 **Swamiye Saranam Aiyappa** 🕉

**Once MinGW is installed, building TBOS is one click!**
