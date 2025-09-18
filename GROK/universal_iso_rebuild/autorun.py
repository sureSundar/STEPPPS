#!/usr/bin/env python3
import subprocess
import sys
import os
from pathlib import Path

def main():
    print("🌟 SundarOS Universal - Complete System")
    print("=" * 40)
    
    # Check components
    components = {
        "STEPPPS Mini OS": "steppps_minios.py",
        "SundarOS Launcher": "launch_sundarios.py", 
        "AI Core": "sundarios_ai_core.py"
    }
    
    available = {}
    for name, filename in components.items():
        if Path(filename).exists():
            available[name] = filename
            print(f"   ✅ {name}: Available")
        else:
            print(f"   ❌ {name}: Missing")
    
    print()
    
    while True:
        print("Launch Options:")
        print("1. 🚀 Full STEPPPS Mini OS")
        print("2. 🌟 SundarOS Launcher") 
        print("3. �� AI Core Demo")
        print("0. Exit")
        
        try:
            choice = input("
Select (0-3): ").strip()
            
            if choice == "0":
                break
            elif choice == "1" and "STEPPPS Mini OS" in available:
                subprocess.run([sys.executable, available["STEPPPS Mini OS"]])
            elif choice == "2" and "SundarOS Launcher" in available:
                subprocess.run([sys.executable, available["SundarOS Launcher"]])
            elif choice == "3" and "AI Core" in available:
                subprocess.run([sys.executable, available["AI Core"]])
            else:
                print("Invalid option or component unavailable")
                
        except (KeyboardInterrupt, EOFError):
            break

if __name__ == "__main__":
    main()
