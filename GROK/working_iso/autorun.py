#!/usr/bin/env python3
"""SundarOS Autorun - Start the AI-Native OS"""
import subprocess
import sys
import os

print("🌟 Starting SundarOS from ISO...")
print("Loading AI-Native Operating System...")

# Run SundarOS
try:
    subprocess.run([sys.executable, "sundarios.py"], cwd=os.path.dirname(__file__))
except KeyboardInterrupt:
    print("\n🌟 SundarOS session ended.")
except Exception as e:
    print(f"Error: {e}")
