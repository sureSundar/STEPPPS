#!/usr/bin/env python3
# SundarOS Safe Mode
def main():
    print("🟡 SundarOS Safe Mode")
    print("Minimal system startup...")
    print("Type 'exit' to shutdown")
    
    while True:
        try:
            cmd = input("SundarOS-safe> ")
            if cmd == 'exit':
                break
            elif cmd == 'help':
                print("Available commands: help, exit, status")
            elif cmd == 'status':
                print("SundarOS Safe Mode - System OK")
            else:
                print(f"Unknown command: {cmd}")
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
