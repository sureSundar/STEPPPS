#!/usr/bin/env python3
import subprocess
import sys

print("Starting SundarOS from ISO...")
subprocess.run([sys.executable, "/boot/kernel.py"])
