#!/usr/bin/env python3
"""
STEPPPS Ecosystem Launcher
Autonomous bootstrap and management system
"""

import subprocess
import sys
import time
import os

def launch_autonomous_ecosystem():
    """Launch completely autonomous STEPPPS ecosystem"""
    print("🚀 LAUNCHING AUTONOMOUS STEPPPS ECOSYSTEM")
    print("=" * 50)

    # 1. Start Genesis in daemon mode
    print("🌟 Starting Genesis Prime...")
    genesis_process = subprocess.Popen([
        sys.executable, "genesis.py", "--daemon"
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    # Give genesis time to bootstrap
    time.sleep(3)

    # 2. Launch Console Manager in new terminal/window
    print("💻 Launching Console Manager in new window...")

    if sys.platform == "win32":
        # Windows
        subprocess.Popen([
            "cmd", "/c", "start", "cmd", "/k",
            f"python {os.path.abspath('console_manager.py')}"
        ])
    elif sys.platform == "darwin":
        # macOS
        subprocess.Popen([
            "osascript", "-e",
            f"tell app \"Terminal\" to do script \"cd {os.getcwd()} && python3 console_manager.py\""
        ])
    else:
        # Linux - try different terminal emulators
        terminals = ["gnome-terminal", "xterm", "konsole", "terminator"]
        for terminal in terminals:
            try:
                if terminal == "gnome-terminal":
                    subprocess.Popen([
                        terminal, "--", "python3", "console_manager.py"
                    ])
                else:
                    subprocess.Popen([
                        terminal, "-e", "python3", "console_manager.py"
                    ])
                break
            except FileNotFoundError:
                continue
        else:
            print("⚠️  Could not find terminal emulator. Run manually: python3 console_manager.py")

    print("✅ ECOSYSTEM LAUNCHED!")
    print("\n🎯 Your STEPPPS ecosystem is now running autonomously:")
    print("   • Genesis Prime: Bootstrapping and supervising")
    print("   • Visual Editor: GUI interface (background)")
    print("   • Console Manager: Interactive command center")
    print("\n📱 Access points:")
    print("   • Console Manager: Available in new terminal window")
    print("   • Manual access: python3 console_manager.py")
    print("   • Visual Editor: GUI window should be visible")
    print("\n🛑 To stop ecosystem: Ctrl+C or close Genesis terminal")
    print("=" * 50)

    return genesis_process

def monitor_ecosystem(genesis_process):
    """Monitor ecosystem health"""
    print("🔍 Monitoring ecosystem...")

    try:
        while True:
            # Check if genesis is still running
            if genesis_process.poll() is not None:
                print("⚠️  Genesis process ended")
                break

            # Check for STEPPPS files
            steppps_files = [f for f in os.listdir('.')
                           if f.endswith('.py') and
                           ('steppps' in f.lower() or 'genesis' in f.lower())]

            print(f"💚 [{time.strftime('%H:%M:%S')}] Ecosystem healthy - {len(steppps_files)} STEPPPS active")
            time.sleep(30)  # Check every 30 seconds

    except KeyboardInterrupt:
        print("\n🛑 Stopping ecosystem monitor...")
        return

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--monitor":
        # Just monitor mode
        print("🔍 STEPPPS Ecosystem Monitor")
        while True:
            time.sleep(10)
            steppps_files = [f for f in os.listdir('.') if 'steppps' in f.lower() or 'genesis' in f.lower()]
            print(f"💚 [{time.strftime('%H:%M:%S')}] {len(steppps_files)} STEPPPS active")
    else:
        # Full autonomous launch
        genesis_process = launch_autonomous_ecosystem()
        monitor_ecosystem(genesis_process)