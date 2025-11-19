# TernaryBit OS - Quick Start Guide

## How to Run TBScript (.tbs) Files

### Method 1: Two-Step Process (Compile then Run)

#### Step 1: Compile the .tbs file to bytecode

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/hello.tbs -o hello.tbc
```

**Output:**
```
Compiled examples/hello.tbs -> hello.tbc
Bytecode size: 59 bytes
Variables: 0 local, 0 global
```

#### Step 2: Run the bytecode on the VM

```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/hello.tbc
```

**Output:**
```
Hello, TernaryBit!
```

---

### Method 2: One-Line Automated Script

I'll create a convenience script that does both steps automatically:

```bash
cd /home/vboxuser/ssr/tbos
./run_tbscript.sh examples/hello.tbs
```

---

## Running All Available Examples

### 1. Hello World
```bash
# Compile
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/hello.tbs -o hello.tbc

# Run
cd ../vm
./tblvm ../compiler/hello.tbc
```

**Expected Output:**
```
Hello, TernaryBit!
```

---

### 2. Calculator
```bash
# Compile
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/calculator.tbs -o calculator.tbc

# Run
cd ../vm
./tblvm ../compiler/calculator.tbc
```

**Expected Output:**
```
Calculator Demo:
10
 +
5
 =
15
```

---

### 3. Loop
```bash
# Compile
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/loop.tbs -o loop.tbc

# Run
cd ../vm
./tblvm ../compiler/loop.tbc
```

**Expected Output:**
```
1
2
3
4
5
```

---

## Writing Your Own TBScript Programs

### Create a new .tbs file

```bash
cd /home/vboxuser/ssr/tbos/compiler/examples
nano myprogram.tbs
```

### Example Program:

```java
// My first TBScript program
void main() {
    println("Welcome to TernaryBit OS!")

    // Variables
    x = 42
    println("The answer is:")
    println(x)

    // Arithmetic
    y = x * 2
    println("Double that is:")
    println(y)
}
```

### Compile and Run:

```bash
# Compile
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/myprogram.tbs -o myprogram.tbc

# Run
cd ../vm
./tblvm ../compiler/myprogram.tbc
```

---

## Troubleshooting

### Problem: VM not found
**Solution:** Build the VM first:
```bash
cd /home/vboxuser/ssr/tbos/vm
make -f Makefile.cross clean
make -f Makefile.cross tblvm
```

### Problem: Python not found
**Solution:** Make sure Python 3 is installed:
```bash
python3 --version
```

### Problem: Compilation errors
**Solution:** Check your TBScript syntax:
- Every program needs `void main() { ... }`
- Use `println()` for output
- Variables don't need declaration
- No semicolons required

---

## Quick Reference

### TBScript Syntax

```java
// Comments start with //

// Main function (required)
void main() {
    // Print with newline
    println("Hello!")

    // Print without newline
    print("Value: ")

    // Variables
    x = 10
    y = 20

    // Arithmetic
    sum = x + y
    diff = x - y
    prod = x * y
    quot = x / y

    // Print numbers
    println(sum)
}
```

### Compiler Options

```bash
# Basic compilation
python3 tbsc.py input.tbs -o output.tbc

# View help
python3 tbsc.py --help
```

### VM Usage

```bash
# Run bytecode
./tblvm program.tbc

# Build VM (if needed)
make -f Makefile.cross tblvm
```

---

## Next Steps

1. **Try the examples** - Run hello.tbs, calculator.tbs, loop.tbs
2. **Write your own** - Create custom TBScript programs
3. **Compress programs** - Use PXFS for extreme compression (see INTEGRATION_EXAMPLE.md)
4. **Deploy anywhere** - Same bytecode runs on all platforms!

---

## File Locations

```
/home/vboxuser/ssr/tbos/
├── compiler/
│   ├── tbsc.py              # Compiler
│   ├── examples/
│   │   ├── hello.tbs        # Hello World
│   │   ├── calculator.tbs   # Calculator demo
│   │   └── loop.tbs         # Loop example
│   └── *.tbc                # Compiled bytecode files
│
└── vm/
    ├── tblvm                # VM executable
    └── Makefile.cross       # Build script
```

---

**Ready to start? Try this:**

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/hello.tbs -o hello.tbc
cd ../vm
./tblvm ../compiler/hello.tbc
```

You should see: `Hello, TernaryBit!`

🎉 **Success! You're now running TBScript programs!**
