#!/usr/bin/env python3
import subprocess
import sys
import os

print("🌟 Starting SundarOS from ISO...")
print("Loading AI-Native Operating System...")

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    subprocess.run([sys.executable, "sundarios.py"])
except Exception as e:
    print(f"Error: {e}")
