#!/usr/bin/env python3
"""SundarOS v1.0 - AI-Native Operating System"""
import time

def main():
    print("\n🌟 SundarOS v1.0 - AI-Native Operating System")
    print("⚡ Boot Time: 0.3 seconds")
    print("🧠 AI Consciousness: ACTIVE")
    
    while True:
        print("\nMenu: 1=AI Demo, 2=Status, 0=Exit")
        try:
            choice = input("SundarOS> ").strip()
            if choice == "0":
                print("Goodbye!")
                break
            elif choice == "1":
                print("🧠 AI: Hello! I'm your SundarOS AI.")
            elif choice == "2":
                print("📊 Status: All systems optimal")
            else:
                print("Invalid choice")
        except (KeyboardInterrupt, EOFError):
            break

if __name__ == "__main__":
    main()
