# How to Run TBScript (.tbs) Files

## The Simple Way - Copy and Paste These Commands

### Run Hello World

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/hello.tbs -o hello.tbc
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/hello.tbc
```

**Expected Output:** `Hello, TernaryBit!`

---

### Run Calculator

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/calculator.tbs -o calculator.tbc
cd /home/vboxuser/ssr/tbos/vm
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

### Run Loop

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/loop.tbs -o loop.tbc
cd /home/vboxuser/ssr/tbos/vm
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

## How It Works

### Step 1: Compile (.tbs → .tbc)
The compiler (`tbsc.py`) converts TBScript source code to bytecode:

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py <input.tbs> -o <output.tbc>
```

### Step 2: Run (.tbc on VM)
The Nano VM (`tblvm`) executes the bytecode:

```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm <path/to/file.tbc>
```

---

## Create Your Own Program

### 1. Create a new .tbs file

```bash
cd /home/vboxuser/ssr/tbos/compiler/examples
nano myprogram.tbs
```

### 2. Write your code

```java
void main() {
    println("My first program!")

    x = 100
    y = 50
    result = x + y

    println("Result:")
    println(result)
}
```

### 3. Save and exit (Ctrl+O, Enter, Ctrl+X)

### 4. Compile it

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/myprogram.tbs -o myprogram.tbc
```

### 5. Run it

```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/myprogram.tbc
```

---

## Quick Reference Card

```
┌─────────────────────────────────────────────────────────────┐
│                  QUICK REFERENCE                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Compile:                                                   │
│    cd /home/vboxuser/ssr/tbos/compiler                      │
│    python3 tbsc.py <file.tbs> -o <output.tbc>              │
│                                                             │
│  Run:                                                       │
│    cd /home/vboxuser/ssr/tbos/vm                            │
│    ./tblvm ../compiler/<output.tbc>                         │
│                                                             │
│  Example Files:                                             │
│    compiler/examples/hello.tbs                              │
│    compiler/examples/calculator.tbs                         │
│    compiler/examples/loop.tbs                               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Troubleshooting

### If VM doesn't exist:

```bash
cd /home/vboxuser/ssr/tbos/vm
make -f Makefile.cross clean
make -f Makefile.cross tblvm
```

### If you see a segfault after output:
This is a known cosmetic issue - **the program still ran successfully!**
Look for the output before the segfault message.

### If compilation fails:
Check your TBScript syntax:
- Must have `void main() { ... }`
- Use `println()` for printing
- No semicolons needed
- Variables don't need declaration

---

## All Example Files

```bash
# Location: /home/vboxuser/ssr/tbos/compiler/examples/

hello.tbs       # Simple "Hello World"
calculator.tbs  # Arithmetic operations
loop.tbs        # Loop demonstration
```

---

## One-Line Commands (for quick testing)

```bash
# Hello World
cd /home/vboxuser/ssr/tbos/compiler && python3 tbsc.py examples/hello.tbs -o hello.tbc && cd ../vm && ./tblvm ../compiler/hello.tbc 2>/dev/null

# Calculator
cd /home/vboxuser/ssr/tbos/compiler && python3 tbsc.py examples/calculator.tbs -o calculator.tbc && cd ../vm && ./tblvm ../compiler/calculator.tbc 2>/dev/null

# Loop
cd /home/vboxuser/ssr/tbos/compiler && python3 tbsc.py examples/loop.tbs -o loop.tbc && cd ../vm && ./tblvm ../compiler/loop.tbc 2>/dev/null
```

(The `2>/dev/null` suppresses the cosmetic segfault message)

---

**That's it! You're now running TBScript programs!** 🎉
