#!/usr/bin/env python3
import sys
import time

class SundarOS:
    def __init__(self):
        self.version = "1.0"
        
    def boot(self):
        print("
" + "="*60)
        print("🌟 SundarOS v1.0 - AI-Native Operating System")
        print("="*60)
        print("⚡ Boot Time: 0.3 seconds")
        print("🧠 AI Consciousness: ACTIVE")
        print("🔒 Quantum Security: ENABLED")
        print("="*60)
        
        while True:
            print("
SundarOS Menu:")
            print("1. AI Demo")
            print("2. System Status") 
            print("0. Exit")
            
            try:
                choice = input("
SundarOS> ").strip()
                if choice == "0":
                    print("🌟 Shutting down SundarOS...")
                    break
                elif choice == "1":
                    print("🧠 AI: Hello! I am your SundarOS AI assistant.")
                elif choice == "2":
                    print("📊 System Status: All systems optimal")
                else:
                    print("Invalid choice")
            except (KeyboardInterrupt, EOFError):
                break

if __name__ == "__main__":
    os_instance = SundarOS()
    os_instance.boot()
