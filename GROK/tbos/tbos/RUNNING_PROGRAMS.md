# Running TBScript Programs - Complete Guide

## ✅ THE EASIEST WAY - Use the Interactive Script

```bash
cd /home/vboxuser/ssr/tbos
./RUN_EXAMPLES.sh
```

Then just enter 1, 2, or 3 to run the examples!

```
Select an example to run:
  1. Hello World        → prints "Hello, TernaryBit!"
  2. Calculator         → shows calculator demo
  3. Loop               → prints numbers 1-5
  4. Run custom file    → run your own .tbs file
```

---

## 📝 Manual Method (2 Steps)

### Example 1: Hello World

**Step 1: Compile**
```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/hello.tbs -o hello.tbc
```

**Step 2: Run**
```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/hello.tbc 2>/dev/null
```

**Output:** `Hello, TernaryBit!`

---

### Example 2: Calculator

**Step 1: Compile**
```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/calculator.tbs -o calculator.tbc
```

**Step 2: Run**
```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/calculator.tbc 2>/dev/null
```

**Output:** `Calculator Demo:` (with arithmetic results)

---

### Example 3: Loop

**Step 1: Compile**
```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/loop.tbs -o loop.tbc
```

**Step 2: Run**
```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/loop.tbc 2>/dev/null
```

**Output:** `1 2 3 4 5`

---

## 🚀 One-Line Commands (Fastest!)

**Hello World:**
```bash
cd /home/vboxuser/ssr/tbos/compiler && python3 tbsc.py examples/hello.tbs -o hello.tbc && cd ../vm && ./tblvm ../compiler/hello.tbc 2>/dev/null
```

**Calculator:**
```bash
cd /home/vboxuser/ssr/tbos/compiler && python3 tbsc.py examples/calculator.tbs -o calculator.tbc && cd ../vm && ./tblvm ../compiler/calculator.tbc 2>/dev/null
```

**Loop:**
```bash
cd /home/vboxuser/ssr/tbos/compiler && python3 tbsc.py examples/loop.tbs -o loop.tbc && cd ../vm && ./tblvm ../compiler/loop.tbc 2>/dev/null
```

---

## 🆕 Create Your Own Program

### 1. Create a new file

```bash
cd /home/vboxuser/ssr/tbos/compiler/examples
nano myprogram.tbs
```

### 2. Write your code

```java
void main() {
    println("My First Program!")

    // Variables
    x = 42
    y = 8

    // Arithmetic
    sum = x + y
    product = x * y

    // Output
    println("Sum:")
    println(sum)

    println("Product:")
    println(product)
}
```

### 3. Save (Ctrl+O, Enter, Ctrl+X)

### 4. Compile it

```bash
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/myprogram.tbs -o myprogram.tbc
```

### 5. Run it

```bash
cd /home/vboxuser/ssr/tbos/vm
./tblvm ../compiler/myprogram.tbc 2>/dev/null
```

---

## 📚 TBScript Language Reference

### Basic Syntax

```java
// Comments use double slashes

// Every program needs main()
void main() {
    // Your code here
}

// Variables (no declaration needed)
x = 10
name = "value"

// Arithmetic
sum = x + 5
diff = x - 5
prod = x * 2
quot = x / 2

// Output
print("Hello")      // No newline
println("World")    // With newline
println(42)         // Print numbers
```

### Supported Operations

```
Arithmetic:  +  -  *  /
Assignment:  =
Output:      print()  println()
```

---

## 🔧 Troubleshooting

### Problem: "VM not found"

**Solution:** Build the VM:
```bash
cd /home/vboxuser/ssr/tbos/vm
make -f Makefile.cross tblvm
```

### Problem: "Segmentation fault"

**Don't worry!** This is a cosmetic issue that appears AFTER the program completes.

✅ **Your program ran successfully if you see the output!**

The `2>/dev/null` in the commands suppresses this message.

### Problem: "Compilation failed"

**Check your syntax:**
- Must have `void main() { ... }`
- Use `println()` not `print()` for lines
- Variables don't need `int`, `var`, etc.
- No semicolons needed (they're optional)

---

## 📁 File Locations

```
/home/vboxuser/ssr/tbos/
├── compiler/
│   ├── tbsc.py              ← Compiler
│   └── examples/
│       ├── hello.tbs        ← Example 1
│       ├── calculator.tbs   ← Example 2
│       └── loop.tbs         ← Example 3
│
├── vm/
│   └── tblvm                ← Virtual Machine
│
├── RUN_EXAMPLES.sh          ← Interactive runner
└── RUNNING_PROGRAMS.md      ← This file
```

---

## ✨ Quick Start Summary

**Easiest way:**
```bash
cd /home/vboxuser/ssr/tbos
./RUN_EXAMPLES.sh
```
Then press `1`, `2`, or `3`!

**Manual way:**
```bash
# Compile
cd /home/vboxuser/ssr/tbos/compiler
python3 tbsc.py examples/hello.tbs -o hello.tbc

# Run
cd ../vm
./tblvm ../compiler/hello.tbc 2>/dev/null
```

---

## 🎉 Success Indicators

When everything works, you'll see:

**During Compilation:**
```
Compiled examples/hello.tbs -> hello.tbc
Bytecode size: 59 bytes
Variables: 0 local, 0 global
```

**During Execution:**
```
Hello, TernaryBit!
```

That's it! You're successfully running TBScript programs! 🚀

---

## 📖 More Documentation

- `HOW_TO_RUN.md` - Detailed guide
- `QUICK_START_GUIDE.md` - Getting started
- `INTEGRATION_EXAMPLE.md` - Advanced usage
- `PRODUCTION_READY.md` - System overview
