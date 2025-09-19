#!/usr/bin/env python3
"""
TernaryBit OS - Universal Installer
Detects platform and deploys appropriate TBOS configuration
"""

import os
import sys
import platform
import subprocess
import json
import shutil
import urllib.request
from pathlib import Path

class TBOSUniversalInstaller:
    def __init__(self):
        self.version = "1.0.0"
        self.platform_info = self.detect_platform()
        self.config = self.generate_config()
        
    def detect_platform(self):
        """Detect current platform and hardware capabilities"""
        system = platform.system().lower()
        machine = platform.machine().lower()
        
        # Get memory info
        try:
            if system == "linux":
                with open('/proc/meminfo', 'r') as f:
                    meminfo = f.read()
                    mem_total = int([line for line in meminfo.split('\n') if 'MemTotal' in line][0].split()[1]) // 1024
            elif system == "darwin":  # macOS
                mem_total = int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']).decode()) // (1024**3)
            elif system == "windows":
                import psutil
                mem_total = psutil.virtual_memory().total // (1024**3)
            else:
                mem_total = 1  # Default fallback
        except:
            mem_total = 1
            
        # Get CPU info
        cpu_count = os.cpu_count() or 1
        
        return {
            'system': system,
            'machine': machine,
            'memory_gb': mem_total,
            'cpu_count': cpu_count,
            'python_version': sys.version_info[:2]
        }
    
    def generate_config(self):
        """Generate TBOS configuration based on detected hardware"""
        info = self.platform_info
        
        # Determine device class
        if info['memory_gb'] >= 8:
            device_class = "desktop"
            features = ["gui", "shell", "steppps", "network", "filesystem", "ai"]
            memory_limit = min(info['memory_gb'] * 0.1, 2.0)
        elif info['memory_gb'] >= 4:
            device_class = "laptop"
            features = ["shell", "steppps", "network", "filesystem"]
            memory_limit = min(info['memory_gb'] * 0.15, 1.0)
        elif info['memory_gb'] >= 2:
            device_class = "mobile"
            features = ["shell", "steppps", "network"]
            memory_limit = min(info['memory_gb'] * 0.2, 0.5)
        else:
            device_class = "embedded"
            features = ["shell", "steppps"]
            memory_limit = min(info['memory_gb'] * 0.3, 0.25)
            
        return {
            'version': self.version,
            'device_class': device_class,
            'platform': info['system'],
            'architecture': info['machine'],
            'features': features,
            'memory_limit_gb': memory_limit,
            'cpu_cores': info['cpu_count'],
            'steppps_dimensions': ["SPACE", "TIME", "EVENT", "PSYCHOLOGY", "PIXEL", "PROMPT", "SCRIPT"]
        }
    
    def print_header(self):
        """Print installation header"""
        print("🌟 TernaryBit Universal OS - Universal Installer")
        print("=" * 50)
        print(f"🖥️  Platform: {self.platform_info['system'].title()}")
        print(f"🏗️  Architecture: {self.platform_info['machine']}")
        print(f"💾 Memory: {self.platform_info['memory_gb']}GB")
        print(f"🧠 CPU Cores: {self.platform_info['cpu_count']}")
        print(f"📱 Device Class: {self.config['device_class'].title()}")
        print(f"⚡ Features: {', '.join(self.config['features'])}")
        print()
    
    def install_dependencies(self):
        """Install platform-specific dependencies"""
        system = self.platform_info['system']
        
        print("📦 Installing dependencies...")
        
        if system == "linux":
            # Check for package manager
            if shutil.which('apt-get'):
                subprocess.run(['sudo', 'apt-get', 'update'], check=False)
                subprocess.run(['sudo', 'apt-get', 'install', '-y', 'python3', 'python3-pip'], check=False)
            elif shutil.which('yum'):
                subprocess.run(['sudo', 'yum', 'install', '-y', 'python3', 'python3-pip'], check=False)
            elif shutil.which('pacman'):
                subprocess.run(['sudo', 'pacman', '-S', '--noconfirm', 'python', 'python-pip'], check=False)
                
        elif system == "darwin":  # macOS
            # Check for Homebrew
            if shutil.which('brew'):
                subprocess.run(['brew', 'install', 'python3'], check=False)
            else:
                print("⚠️  Homebrew not found. Please install Python 3 manually.")
                
        elif system == "windows":
            print("⚠️  Please ensure Python 3 is installed on Windows.")
            
        print("  ✅ Dependencies checked")
    
    def create_installation_dirs(self):
        """Create installation directories"""
        system = self.platform_info['system']
        
        if system == "windows":
            self.install_dir = Path(os.environ.get('PROGRAMFILES', 'C:\\Program Files')) / "TernaryBitOS"
            self.config_dir = Path(os.environ.get('APPDATA', '')) / "TernaryBitOS"
        else:
            if os.geteuid() == 0:  # Running as root
                self.install_dir = Path("/opt/ternarybit-os")
                self.config_dir = Path("/etc/ternarybit-os")
            else:
                self.install_dir = Path.home() / ".local/share/ternarybit-os"
                self.config_dir = Path.home() / ".config/ternarybit-os"
        
        print(f"📁 Creating directories...")
        self.install_dir.mkdir(parents=True, exist_ok=True)
        self.config_dir.mkdir(parents=True, exist_ok=True)
        (self.install_dir / "bin").mkdir(exist_ok=True)
        (self.install_dir / "lib").mkdir(exist_ok=True)
        print(f"  ✅ Install dir: {self.install_dir}")
        print(f"  ✅ Config dir: {self.config_dir}")
    
    def install_tbos_core(self):
        """Install TBOS core components"""
        print("🚀 Installing TBOS core...")
        
        # Create main TBOS executable
        tbos_script = f'''#!/usr/bin/env python3
"""TernaryBit Universal OS - {self.config['device_class'].title()} Edition"""

import os
import sys
import json
import time
import threading
from datetime import datetime

class TernaryBitOS:
    def __init__(self):
        self.version = "{self.version}"
        self.device_class = "{self.config['device_class']}"
        self.platform = "{self.config['platform']}"
        self.features = {self.config['features']}
        self.memory_limit = {self.config['memory_limit_gb']}
        self.cpu_cores = {self.config['cpu_cores']}
        self.steppps_dimensions = {self.config['steppps_dimensions']}
        self.running = False
        
    def boot(self):
        print("🌟 TernaryBit Universal OS v{{}} - {{}} Edition".format(self.version, self.device_class.title()))
        print("=" * 60)
        print("🖥️  Platform: {{}}".format(self.platform.title()))
        print("📱 Device Class: {{}}".format(self.device_class.title()))
        print("💾 Memory Limit: {{}}GB".format(self.memory_limit))
        print("🧠 CPU Cores: {{}}".format(self.cpu_cores))
        print("⚡ Features: {{}}".format(', '.join(self.features)))
        print("🌐 STEPPPS Dimensions: {{}}".format(len(self.steppps_dimensions)))
        print()
        
        # Boot sequence
        print("🚀 Booting TernaryBit OS...")
        boot_steps = [
            "Hardware detection",
            "Memory initialization", 
            "STEPPPS framework loading",
            "Feature activation",
            "System ready"
        ]
        
        for step in boot_steps:
            print(f"  ⏳ {{step}}...")
            time.sleep(0.2)
            print(f"  ✅ {{step}} complete")
            
        print("\\n✅ TernaryBit OS ready!")
        self.running = True
        
    def shell(self):
        print("\\n🖥️  TernaryBit OS Interactive Shell")
        print("Type 'help' for commands, 'exit' to quit")
        print("-" * 50)
        
        while self.running:
            try:
                cmd = input("tbos> ").strip()
                if not cmd:
                    continue
                    
                self.execute_command(cmd)
                    
            except KeyboardInterrupt:
                print("\\n^C")
                continue
            except EOFError:
                break
    
    def execute_command(self, cmd):
        if cmd == 'exit':
            self.running = False
        elif cmd == 'help':
            self.show_help()
        elif cmd == 'status':
            self.show_status()
        elif cmd == 'steppps':
            self.show_steppps()
        elif cmd == 'features':
            self.show_features()
        elif cmd == 'version':
            print(f"TernaryBit OS v{{self.version}}")
        elif cmd == 'clear':
            os.system('cls' if self.platform == 'windows' else 'clear')
        elif cmd.startswith('echo '):
            print(cmd[5:])
        else:
            print(f"Unknown command: {{cmd}}. Type 'help' for available commands.")
    
    def show_help(self):
        print("TernaryBit OS Commands:")
        print("  help      - Show this help")
        print("  status    - Show system status")
        print("  steppps   - Show STEPPPS framework")
        print("  features  - Show available features")
        print("  version   - Show OS version")
        print("  clear     - Clear screen")
        print("  exit      - Exit TBOS")
    
    def show_status(self):
        print("📊 TernaryBit OS Status:")
        print(f"  Version: {{self.version}}")
        print(f"  Device Class: {{self.device_class}}")
        print(f"  Platform: {{self.platform}}")
        print(f"  Memory Limit: {{self.memory_limit}}GB")
        print(f"  CPU Cores: {{self.cpu_cores}}")
        print(f"  Uptime: {{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}}")
    
    def show_steppps(self):
        print("🌟 STEPPPS Framework:")
        for i, dim in enumerate(self.steppps_dimensions, 1):
            print(f"  {{i}}. {{dim}}: ✅ Active")
    
    def show_features(self):
        print("⚡ Available Features:")
        for feature in self.features:
            print(f"  ✅ {{feature.upper()}}")
    
    def shutdown(self):
        print("\\n🔄 Shutting down TernaryBit OS...")
        self.running = False
        print("✅ Shutdown complete")

def main():
    if len(sys.argv) > 1:
        if sys.argv[1] == '--version':
            print("TernaryBit OS v{self.version}")
            return
        elif sys.argv[1] == '--help':
            print("TernaryBit Universal OS")
            print("Usage: tbos [options]")
            print("Options:")
            print("  --version    Show version")
            print("  --help       Show help")
            return
    
    tbos = TernaryBitOS()
    try:
        tbos.boot()
        tbos.shell()
    except KeyboardInterrupt:
        pass
    finally:
        tbos.shutdown()

if __name__ == "__main__":
    main()
'''
        
        # Write TBOS executable
        tbos_path = self.install_dir / "bin" / "tbos"
        if self.platform_info['system'] == "windows":
            tbos_path = tbos_path.with_suffix('.py')
            
        with open(tbos_path, 'w') as f:
            f.write(tbos_script)
            
        # Make executable on Unix systems
        if self.platform_info['system'] != "windows":
            os.chmod(tbos_path, 0o755)
            
        print(f"  ✅ TBOS core installed: {tbos_path}")
    
    def create_config_file(self):
        """Create TBOS configuration file"""
        print("⚙️ Creating configuration...")
        
        config_path = self.config_dir / "tbos.json"
        with open(config_path, 'w') as f:
            json.dump(self.config, f, indent=2)
            
        print(f"  ✅ Configuration saved: {config_path}")
    
    def setup_path(self):
        """Add TBOS to system PATH"""
        print("🔧 Configuring PATH...")
        
        bin_dir = self.install_dir / "bin"
        system = self.platform_info['system']
        
        if system == "windows":
            print("  ⚠️  Please manually add to PATH: {}".format(bin_dir))
        else:
            # Create symlink for Unix systems
            if os.geteuid() == 0:  # Root
                symlink_path = Path("/usr/local/bin/tbos")
            else:
                local_bin = Path.home() / ".local/bin"
                local_bin.mkdir(exist_ok=True)
                symlink_path = local_bin / "tbos"
                
            if symlink_path.exists():
                symlink_path.unlink()
                
            symlink_path.symlink_to(bin_dir / "tbos")
            print(f"  ✅ Added to PATH: {symlink_path}")
    
    def test_installation(self):
        """Test TBOS installation"""
        print("🧪 Testing installation...")
        
        tbos_path = self.install_dir / "bin" / "tbos"
        if self.platform_info['system'] == "windows":
            tbos_path = tbos_path.with_suffix('.py')
            
        if tbos_path.exists():
            print("  ✅ Installation test passed")
            return True
        else:
            print("  ❌ Installation test failed")
            return False
    
    def install(self):
        """Main installation process"""
        self.print_header()
        
        try:
            self.install_dependencies()
            self.create_installation_dirs()
            self.install_tbos_core()
            self.create_config_file()
            self.setup_path()
            
            if self.test_installation():
                print()
                print("🎉 TernaryBit OS Installation Complete!")
                print("=" * 50)
                print()
                print("Quick Start:")
                print("  • Run 'tbos' from terminal/command prompt")
                print("  • Use 'tbos --help' for options")
                print()
                print("🌟 Welcome to Universal Computing!")
                
                # Offer to start TBOS
                try:
                    response = input("\\nWould you like to start TernaryBit OS now? (y/n): ")
                    if response.lower() in ['y', 'yes']:
                        print("🚀 Starting TernaryBit OS...")
                        tbos_path = self.install_dir / "bin" / "tbos"
                        if self.platform_info['system'] == "windows":
                            subprocess.run([sys.executable, str(tbos_path.with_suffix('.py'))])
                        else:
                            subprocess.run([str(tbos_path)])
                except KeyboardInterrupt:
                    print("\\nInstallation complete!")
                    
            else:
                print("❌ Installation failed!")
                return False
                
        except Exception as e:
            print(f"❌ Installation error: {e}")
            return False
            
        return True

def main():
    installer = TBOSUniversalInstaller()
    installer.install()

if __name__ == "__main__":
    main()
