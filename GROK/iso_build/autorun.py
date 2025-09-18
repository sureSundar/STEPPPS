#!/usr/bin/env python3
import subprocess
import sys
print("🌟 Starting SundarOS...")
try:
    subprocess.run([sys.executable, "sundarios.py"])
except Exception as e:
    print(f"Error: {e}")
