#!/usr/bin/env python3
# TernaryBit OS Python Launcher
import os
import sys
import platform
import subprocess

def main():
    print("🕉️  TernaryBit OS - Sanatana Dharma Computing")
    print(f"Platform: {platform.system()} {platform.machine()}")
    print()

    # Try Java first
    try:
        subprocess.run(['java', '-version'], check=True,
                      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print("☕ Launching Java Universal TBOS...")
        subprocess.run(['java', 'TernaryBitOS'])
        return
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass

    # Try native binary
    if os.path.exists('./tbos_universal'):
        print("⚡ Launching Native Universal TBOS...")
        subprocess.run(['./tbos_universal'])
        return
    elif os.path.exists('./tbos_universal.exe'):
        print("⚡ Launching Native Universal TBOS...")
        subprocess.run(['./tbos_universal.exe'])
        return

    # Fallback to Python implementation
    print("🐍 Launching Python Dharma TBOS...")
    print("Platform:", platform.platform())
    print("Python:", sys.version)
    print("Om Namah Shivaya - Python TBOS Active")

if __name__ == '__main__':
    main()
