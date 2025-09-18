#!/usr/bin/env python3
"""
SundarOS Simple Launcher
========================
Easy way to experience all SundarOS components
"""

import os
import sys
import subprocess

def main():
    print("🌟 SundarOS - The World's Most Advanced Operating System")
    print("=" * 60)
    print("Choose your experience:")
    print()
    print("1. 💻 Core SundarOS (Interactive OS)")
    print("2. 🧠 AI Intelligence Demo")
    print("3. 🎨 Revolutionary GUI")
    print("4. 🛡️  Enterprise Security")
    print("5. ☁️  Cloud Computing")
    print("6. 🚀 Developer Platform")
    print("7. 🎯 Run All Demos")
    print("0. Exit")
    print()
    
    while True:
        try:
            choice = input("SundarOS> ").strip()
            
            if choice == "0":
                print("👋 Thank you for using SundarOS!")
                break
            elif choice == "1":
                print("🚀 Launching SundarOS Core...")
                subprocess.run([sys.executable, "steppps_minios.py"])
            elif choice == "2":
                print("🧠 Starting AI Intelligence Demo...")
                subprocess.run([sys.executable, "sundarios_ai_core.py"])
            elif choice == "3":
                print("🎨 Launching Revolutionary GUI...")
                subprocess.run([sys.executable, "sundarios_gui_engine.py"])
            elif choice == "4":
                print("🛡️  Starting Security Demo...")
                subprocess.run([sys.executable, "sundarios_security_core.py"])
            elif choice == "5":
                print("☁️  Launching Cloud Platform...")
                subprocess.run([sys.executable, "sundarios_cloud_core.py"])
            elif choice == "6":
                print("🚀 Starting Developer Platform...")
                subprocess.run([sys.executable, "sundarios_developer_platform.py"])
            elif choice == "7":
                print("🎯 Running All SundarOS Demos...")
                demos = [
                    "sundarios_ai_core.py",
                    "sundarios_security_core.py", 
                    "sundarios_cloud_core.py",
                    "sundarios_developer_platform.py"
                ]
                for demo in demos:
                    print(f"\n▶️  Running {demo}...")
                    subprocess.run([sys.executable, demo])
                    print("✅ Demo completed\n")
            else:
                print("❓ Invalid choice. Please select 0-7.")
                
        except KeyboardInterrupt:
            print("\n👋 Goodbye!")
            break
        except Exception as e:
            print(f"❌ Error: {e}")

if __name__ == "__main__":
    main()
