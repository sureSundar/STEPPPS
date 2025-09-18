#!/usr/bin/env python3
# SundarOS System Init
import sys
import os
sys.path.insert(0, '/system')

def main():
    print("🌟 SundarOS System Initialization")
    
    # Check for safe mode
    if 'safe=1' in ' '.join(sys.argv):
        print("🟡 Safe mode detected")
        # Minimal startup
        print("SundarOS Safe Mode Ready")
        return
    
    # Normal startup
    try:
        from launch_sundarios import main as sundarios_main
        sundarios_main()
    except Exception as e:
        print(f"⚠️ Startup error: {e}")
        print("Falling back to minimal mode...")

if __name__ == "__main__":
    main()
