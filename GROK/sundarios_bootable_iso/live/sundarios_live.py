#!/usr/bin/env python3
# SundarOS Live Mode
def main():
    print("🔴 SundarOS Live Mode")
    print("Running without installation...")
    print("AI-Native OS Demo Environment")
    
    # Start minimal SundarOS
    try:
        import sys
        sys.path.insert(0, '/system')
        from launch_sundarios import main as sundarios_main
        sundarios_main()
    except:
        print("Demo mode ready - type 'help' for commands")

if __name__ == "__main__":
    main()
