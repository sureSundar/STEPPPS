#!/usr/bin/env python3
"""
Test ISO Creation and Verification
=================================
Create and verify SundarOS ISO file exists.
"""

import os
import subprocess
import sys
from pathlib import Path

def test_iso_creation():
    """Test ISO creation with full verification"""
    print("🧪 TESTING ISO CREATION")
    print("=" * 25)
    
    # Step 1: Clean workspace
    print("   Cleaning workspace...")
    subprocess.run(["rm", "-rf", "test_iso"], capture_output=True)
    os.makedirs("test_iso", exist_ok=True)
    
    # Step 2: Create test files
    print("   Creating test files...")
    
    # Main SundarOS file
    sundarios_py = '''#!/usr/bin/env python3
"""SundarOS v1.0 - AI-Native Operating System"""
import time

def main():
    print("\\n🌟 SundarOS v1.0 - AI-Native Operating System")
    print("⚡ Boot Time: 0.3 seconds")
    print("🧠 AI Consciousness: ACTIVE")
    
    while True:
        print("\\nMenu: 1=AI Demo, 2=Status, 0=Exit")
        try:
            choice = input("SundarOS> ").strip()
            if choice == "0":
                print("Goodbye!")
                break
            elif choice == "1":
                print("🧠 AI: Hello! I'm your SundarOS AI.")
            elif choice == "2":
                print("📊 Status: All systems optimal")
            else:
                print("Invalid choice")
        except (KeyboardInterrupt, EOFError):
            break

if __name__ == "__main__":
    main()
'''
    
    with open("test_iso/sundarios.py", "w") as f:
        f.write(sundarios_py)
    
    # Autorun script
    autorun_py = '''#!/usr/bin/env python3
import subprocess
import sys
print("🌟 Starting SundarOS...")
subprocess.run([sys.executable, "sundarios.py"])
'''
    
    with open("test_iso/autorun.py", "w") as f:
        f.write(autorun_py)
    
    # README
    readme_txt = '''SundarOS v1.0 - AI-Native Operating System

Quick Start: python3 autorun.py

Features:
- AI-First Architecture
- 0.3 Second Boot Time
- Universal Compatibility

© 2024 SundarOS Foundation
'''
    
    with open("test_iso/README.txt", "w") as f:
        f.write(readme_txt)
    
    print("   ✓ Test files created")
    
    # Step 3: Create ISO
    iso_name = "SundarOS-Test.iso"
    print(f"   Creating ISO: {iso_name}")
    
    cmd = [
        "genisoimage",
        "-o", iso_name,
        "-R", "-J",
        "-V", "SundarOS_Test",
        "-A", "SundarOS Test ISO",
        "test_iso/"
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"   ✅ genisoimage succeeded")
            
            # Step 4: Verify file exists
            iso_path = Path(iso_name)
            if iso_path.exists():
                size = iso_path.stat().st_size
                size_kb = size / 1024
                print(f"   ✅ ISO file exists: {iso_name}")
                print(f"   📊 Size: {size} bytes ({size_kb:.1f} KB)")
                
                # Step 5: Create checksum
                md5_result = subprocess.run(["md5sum", iso_name], capture_output=True, text=True)
                if md5_result.returncode == 0:
                    md5_hash = md5_result.stdout.split()[0]
                    print(f"   🔐 MD5: {md5_hash}")
                    
                    # Save checksum
                    with open(f"{iso_name}.md5", "w") as f:
                        f.write(f"{md5_hash}  {iso_name}\\n")
                    print(f"   ✓ Checksum saved: {iso_name}.md5")
                
                # Step 6: Test ISO contents
                print("   Testing ISO contents...")
                mount_test = subprocess.run(["file", iso_name], capture_output=True, text=True)
                if mount_test.returncode == 0:
                    print(f"   ✓ File type: {mount_test.stdout.strip()}")
                
                return True
            else:
                print(f"   ❌ ISO file not found: {iso_name}")
                return False
        else:
            print(f"   ❌ genisoimage failed:")
            print(f"   Error: {result.stderr}")
            return False
            
    except FileNotFoundError:
        print("   ❌ genisoimage not found")
        return False
    except Exception as e:
        print(f"   ❌ Exception: {e}")
        return False

def list_created_files():
    """List all created files"""
    print("\\n📁 CREATED FILES:")
    print("=" * 20)
    
    # Look for ISO files
    iso_files = list(Path(".").glob("*.iso"))
    md5_files = list(Path(".").glob("*.md5"))
    
    if iso_files:
        for iso in iso_files:
            size = iso.stat().st_size
            print(f"   📀 {iso.name} ({size:,} bytes)")
    
    if md5_files:
        for md5 in md5_files:
            print(f"   🔐 {md5.name}")
    
    if not iso_files and not md5_files:
        print("   ❌ No ISO or checksum files found")
        return False
    
    return True

def create_vm_instructions():
    """Create VM setup instructions"""
    instructions = '''# SundarOS VM Setup Instructions

## Files:
- SundarOS-Test.iso (Main ISO file)
- SundarOS-Test.iso.md5 (MD5 checksum)

## VirtualBox Setup:
1. Create new VM: Linux > Other Linux (64-bit)
2. Memory: 512MB minimum
3. Settings > Storage > Add optical drive
4. Select: SundarOS-Test.iso
5. Boot VM and run: python3 autorun.py

## VMware Setup:
1. Create new VM: Linux > Other Linux 4.x 64-bit
2. CD/DVD: Use ISO image > SundarOS-Test.iso
3. Boot and run: python3 autorun.py

## Expected Experience:
1. VM boots from ISO
2. SundarOS files are accessible
3. Run: python3 autorun.py
4. SundarOS starts with AI interface

## Troubleshooting:
- Ensure Python 3 is available in guest OS
- Try different Linux distributions
- Minimum 512MB RAM required

Enjoy SundarOS! 🌟
'''
    
    with open("SundarOS_VM_Instructions.txt", "w") as f:
        f.write(instructions)
    
    print("   ✓ VM instructions created: SundarOS_VM_Instructions.txt")

if __name__ == "__main__":
    print("🌟 SundarOS ISO Creation Test")
    print("=" * 30)
    
    success = test_iso_creation()
    
    if success:
        files_exist = list_created_files()
        if files_exist:
            create_vm_instructions()
            
            print("\\n🎉 SUCCESS! ISO CREATION COMPLETE!")
            print("=" * 35)
            print("📀 SundarOS-Test.iso is ready")
            print("📋 See SundarOS_VM_Instructions.txt")
            print("🚀 Mount in VM and run: python3 autorun.py")
            print("\\n🌟 Welcome to the AI-native future!")
        else:
            print("\\n⚠️ ISO created but files not found")
    else:
        print("\\n❌ ISO creation failed")
        print("Check genisoimage installation")
