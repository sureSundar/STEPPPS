#!/usr/bin/env python3
"""
TernaryBit OS - Conscious Shell Interactive Demo
Simulates the conscious shell with karma tracking and consciousness evolution
"""

import sys
import os
from datetime import datetime

class ConsciousShell:
    def __init__(self):
        self.karma = 100
        self.consciousness = "AWAKENING"
        self.commands_executed = 0
        self.cwd = "/"
        self.history = []
        self.files = {
            "/": ["bin", "home", "tmp", "dev"],
            "/home": ["user"],
            "/bin": ["ls", "cat", "echo"],
            "/tmp": [],
            "/dev": ["null"]
        }

    def get_consciousness_level(self):
        levels = ["NONE", "AWAKENING", "AWARE", "COMPASSIONATE", "ENLIGHTENED"]
        level_idx = min(self.commands_executed // 20, 4)
        return levels[level_idx]

    def update_karma(self, amount, reason=""):
        self.karma += amount
        if amount > 0:
            symbol = "+"
        else:
            symbol = ""
        if reason:
            print(f"   Karma {symbol}{amount}: {reason}")

    def cmd_help(self, args):
        print("\n╔══════════════════════════════════════════════════════════════╗")
        print("║   TernaryBit OS - Available Commands                        ║")
        print("╚══════════════════════════════════════════════════════════════╝\n")
        print("📁 File System Commands:")
        print("  ls [path]              List directory contents")
        print("  cd <path>              Change directory")
        print("  pwd                    Print working directory")
        print("  mkdir <path>           Create directory")
        print("  cat <path>             Display file contents")
        print()
        print("🕉️  Consciousness Commands:")
        print("  karma                  Show karma status")
        print("  consciousness          Show consciousness level")
        print("  history                Show command history")
        print()
        print("🛠️  System Commands:")
        print("  echo <text>            Display text")
        print("  clear                  Clear screen")
        print("  help                   Show this help")
        print("  exit                   Exit shell")
        print()
        print("💡 Tips:")
        print("  - Commands cost and reward karma")
        print("  - Consciousness evolves with use")
        print("  - Practice mindful computing!")
        print()
        self.update_karma(2, "seeking knowledge")
        return 0

    def cmd_pwd(self, args):
        print(self.cwd)
        self.update_karma(1, "basic command")
        return 0

    def cmd_ls(self, args):
        path = args[0] if args else self.cwd
        if path not in self.files:
            print(f"❌ Directory not found: {path}")
            return -1

        print(f"\nListing directory: {path}\n")
        for item in self.files[path]:
            print(f"  {item}")
        print(f"\nTotal: {len(self.files[path])} entries")
        self.update_karma(1, "basic command")
        return 0

    def cmd_cd(self, args):
        if not args:
            print("Usage: cd <path>")
            return -1

        path = args[0]
        if path not in self.files:
            print(f"❌ Directory not found: {path}")
            return -1

        self.cwd = path
        print(f"Changed directory to: {path}")
        self.update_karma(1, "navigation")
        return 0

    def cmd_echo(self, args):
        print(" ".join(args))
        self.update_karma(1, "communication")
        return 0

    def cmd_karma(self, args):
        level = "Excellent" if self.karma > 200 else "Good" if self.karma > 100 else "Learning"

        print("\n╔════════════════════════════════════════════════════════╗")
        print("║      Your Karma Status                                 ║")
        print("╠════════════════════════════════════════════════════════╣")
        print(f"║ Current Karma:         {self.karma:<30} ║")
        print(f"║ Karma Level:           {level:<30} ║")
        print("╠════════════════════════════════════════════════════════╣")

        if self.karma < 50:
            guidance = "Practice mindful actions"
        elif self.karma < 150:
            guidance = "Continue learning with awareness"
        elif self.karma < 300:
            guidance = "Share knowledge with others"
        else:
            guidance = "Guide others to enlightenment"

        print(f"║ Guidance:              {guidance:<30} ║")
        print("╚════════════════════════════════════════════════════════╝\n")
        self.update_karma(5, "self-reflection")
        return 0

    def cmd_consciousness(self, args):
        current = self.get_consciousness_level()
        levels = ["NONE", "AWAKENING", "AWARE", "COMPASSIONATE", "ENLIGHTENED"]

        print("\n╔════════════════════════════════════════════════════════╗")
        print("║      Your Consciousness Level                          ║")
        print("╠════════════════════════════════════════════════════════╣")

        for i, level in enumerate(levels):
            if level == current:
                print(f"║ ➤ {level:<15} [{i}] - Current level ║")
            elif levels.index(current) > i:
                print(f"║ ✓ {level:<15} [{i}] - Achieved      ║")
            else:
                print(f"║   {level:<15} [{i}] - Future        ║")

        next_level_in = 20 - (self.commands_executed % 20)
        print("╠════════════════════════════════════════════════════════╣")
        print(f"║ Commands Executed:     {self.commands_executed:<30} ║")
        print(f"║ Next Level In:         {next_level_in:<30} ║")
        print("╚════════════════════════════════════════════════════════╝\n")
        self.update_karma(5, "mindfulness")
        return 0

    def cmd_history(self, args):
        print("\n╔════════════════════════════════════════════════════════╗")
        print("║      Command History                                   ║")
        print("╚════════════════════════════════════════════════════════╝\n")

        for i, entry in enumerate(self.history[-20:], 1):  # Last 20
            print(f"  {i}. {entry['cmd']}")
            print(f"     Karma: +{entry['karma']}")

        print(f"\nTotal commands in history: {len(self.history)}\n")
        self.update_karma(2, "reviewing actions")
        return 0

    def cmd_mkdir(self, args):
        if not args:
            print("Usage: mkdir <path>")
            return -1

        path = args[0]
        if path in self.files:
            print(f"❌ Directory already exists: {path}")
            return -1

        # Check karma for creation actions
        if self.karma < 10:
            print("❌ Insufficient karma to create directories")
            self.update_karma(-5, "attempting action without preparation")
            return -1

        self.files[path] = []
        print(f"✓ Created directory: {path}")
        self.update_karma(3, "creation")
        return 0

    def cmd_clear(self, args):
        os.system('clear' if os.name == 'posix' else 'cls')
        return 0

    def execute(self, cmdline):
        cmdline = cmdline.strip()
        if not cmdline:
            return 0

        parts = cmdline.split()
        cmd = parts[0]
        args = parts[1:]

        # Track in history
        self.commands_executed += 1

        # Update consciousness
        old_consciousness = self.consciousness
        self.consciousness = self.get_consciousness_level()
        if old_consciousness != self.consciousness:
            print(f"\n✨ Consciousness evolved to: {self.consciousness} ✨\n")

        # Execute command
        commands = {
            'help': self.cmd_help,
            'pwd': self.cmd_pwd,
            'ls': self.cmd_ls,
            'cd': self.cmd_cd,
            'echo': self.cmd_echo,
            'karma': self.cmd_karma,
            'consciousness': self.cmd_consciousness,
            'history': self.cmd_history,
            'mkdir': self.cmd_mkdir,
            'clear': self.cmd_clear,
        }

        if cmd in commands:
            karma_before = self.karma
            result = commands[cmd](args)
            karma_gained = self.karma - karma_before

            # Add to history
            self.history.append({
                'cmd': cmdline,
                'karma': karma_gained,
                'result': result
            })

            return result
        else:
            print(f"❌ Command not found: {cmd}")
            print("   Type 'help' for available commands")
            self.update_karma(-2, "invalid command")
            return -1

    def run(self):
        print("\n╔══════════════════════════════════════════════════════════════╗")
        print("║   TernaryBit OS - Conscious Shell (Interactive Demo)        ║")
        print("║   सॉफ्टवेर (Soft-Aware)                                      ║")
        print("╚══════════════════════════════════════════════════════════════╝\n")

        print("🕉️  Shell initialized successfully!")
        print(f"    Starting karma: {self.karma}")
        print(f"    Consciousness: {self.consciousness}\n")

        print("Type 'help' for available commands.")
        print("Type 'exit' to quit.\n")

        print("💡 Features:")
        print("   • Commands earn/cost karma")
        print("   • Consciousness evolves with use")
        print("   • History tracks all actions")
        print("   • Mindful computing rewarded\n")

        while True:
            try:
                cmdline = input("tbos> ")

                if cmdline.strip() in ['exit', 'quit']:
                    print("\n═══════════════════════════════════════════════════")
                    print("  Graceful Shutdown")
                    print("═══════════════════════════════════════════════════\n")
                    print("Preserving consciousness state...")
                    print(f"  Final karma: {self.karma}")
                    print(f"  Commands executed: {self.commands_executed}")
                    print(f"  Consciousness level: {self.consciousness}")
                    print("\n॥ तत् सत् ॥ (That is Truth)\n")
                    print("🕉️  May you compute with consciousness! 🕉️\n")
                    break

                self.execute(cmdline)
                print()  # Blank line after each command

            except EOFError:
                print("\n")
                break
            except KeyboardInterrupt:
                print("\n\nUse 'exit' to quit gracefully.\n")
                continue

if __name__ == "__main__":
    shell = ConsciousShell()
    shell.run()
