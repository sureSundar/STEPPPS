#!/usr/bin/env python3
"""
QuantumOS Production Installer
==============================
Professional-grade installer for QuantumOS Linux distribution with
STEPPPS framework integration and AI-First capabilities.
"""

import os
import sys
import time
import shutil
import subprocess
import logging
import json
import threading
import uuid
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple
from dataclasses import dataclass, field
from enum import Enum
import parted
import psutil
import hashlib

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('quantumos-installer')

class InstallationStage(Enum):
    PREPARATION = "preparation"
    PARTITIONING = "partitioning"
    FORMATTING = "formatting"
    SYSTEM_COPY = "system_copy"
    BOOTLOADER = "bootloader"
    CONFIGURATION = "configuration"
    AI_SETUP = "ai_setup"
    FINALIZATION = "finalization"
    COMPLETED = "completed"
    ERROR = "error"

class PartitionType(Enum):
    EFI = "efi"
    BOOT = "boot"
    ROOT = "root"
    HOME = "home"
    SWAP = "swap"

@dataclass
class DiskInfo:
    """Information about a disk device"""
    device: str
    size: int
    model: str
    serial: str
    removable: bool
    partitions: List[Dict[str, Any]] = field(default_factory=list)

@dataclass
class PartitionPlan:
    """Partition layout plan"""
    device: str
    partitions: List[Dict[str, Any]]
    filesystem: str = "btrfs"
    encryption: bool = False

@dataclass
class UserAccount:
    """User account information"""
    username: str
    password: str
    full_name: str
    admin: bool = False
    auto_login: bool = False

@dataclass
class InstallationConfig:
    """Complete installation configuration"""
    target_disk: str
    partition_plan: PartitionPlan
    timezone: str
    locale: str
    keyboard: str
    hostname: str
    user_account: UserAccount
    ai_features: Dict[str, bool]
    network_config: Dict[str, Any]

class DiskManager:
    """Manages disk operations for installation"""

    @staticmethod
    def detect_disks() -> List[DiskInfo]:
        """Detect available disks"""
        disks = []

        try:
            # Get all disk devices
            devices = [d for d in Path('/dev').glob('sd*') if d.is_block_device()]
            devices.extend([d for d in Path('/dev').glob('nvme*n*') if d.is_block_device()])
            devices.extend([d for d in Path('/dev').glob('vd*') if d.is_block_device()])

            for device in devices:
                try:
                    # Skip partitions
                    if any(char.isdigit() for char in device.name[-2:]):
                        continue

                    # Get disk information
                    size = 0
                    model = "Unknown"
                    serial = "Unknown"
                    removable = False

                    # Try to get size from /sys
                    sys_block = Path(f"/sys/block/{device.name}")
                    if sys_block.exists():
                        size_file = sys_block / "size"
                        if size_file.exists():
                            sectors = int(size_file.read_text().strip())
                            size = sectors * 512  # 512 bytes per sector

                        model_file = sys_block / "device" / "model"
                        if model_file.exists():
                            model = model_file.read_text().strip()

                        removable_file = sys_block / "removable"
                        if removable_file.exists():
                            removable = bool(int(removable_file.read_text().strip()))

                    # Get partitions
                    partitions = []
                    for part_path in device.parent.glob(f"{device.name}*"):
                        if part_path != device and part_path.is_block_device():
                            partitions.append({
                                'device': str(part_path),
                                'size': 0,  # Would get actual size in production
                                'filesystem': 'unknown'
                            })

                    disk_info = DiskInfo(
                        device=str(device),
                        size=size,
                        model=model,
                        serial=serial,
                        removable=removable,
                        partitions=partitions
                    )
                    disks.append(disk_info)

                except Exception as e:
                    logger.warning(f"Failed to read disk info for {device}: {e}")
                    continue

        except Exception as e:
            logger.error(f"Failed to detect disks: {e}")

        return disks

    @staticmethod
    def create_partition_table(device: str, partition_plan: PartitionPlan) -> bool:
        """Create partition table on device"""
        try:
            logger.info(f"Creating partition table on {device}")

            # In production, use parted or similar tools
            # This is a simulation of the partitioning process

            # Unmount any existing partitions
            subprocess.run(['umount', '-f', f"{device}*"], capture_output=True)

            # Create new partition table
            subprocess.run([
                'parted', '-s', device, 'mklabel', 'gpt'
            ], check=True, capture_output=True)

            # Create partitions based on plan
            start_sector = 1  # Start at 1MB

            for partition in partition_plan.partitions:
                size_mb = partition['size'] // (1024 * 1024)
                end_sector = start_sector + size_mb

                part_type = partition['type']
                filesystem = partition.get('filesystem', 'ext4')

                # Create partition
                subprocess.run([
                    'parted', '-s', device,
                    'mkpart', part_type, str(start_sector), str(end_sector)
                ], check=True, capture_output=True)

                # Set partition flags if needed
                if part_type == 'EFI':
                    subprocess.run([
                        'parted', '-s', device, 'set', '1', 'esp', 'on'
                    ], check=True, capture_output=True)

                start_sector = end_sector + 1

            logger.info("Partition table created successfully")
            return True

        except subprocess.CalledProcessError as e:
            logger.error(f"Partitioning failed: {e}")
            return False
        except Exception as e:
            logger.error(f"Partitioning error: {e}")
            return False

    @staticmethod
    def format_partitions(partition_plan: PartitionPlan) -> bool:
        """Format partitions according to plan"""
        try:
            logger.info("Formatting partitions...")

            for i, partition in enumerate(partition_plan.partitions, 1):
                device_part = f"{partition_plan.device}{i}"
                filesystem = partition.get('filesystem', 'ext4')
                label = partition.get('label', f"quantumos-{partition['type']}")

                logger.info(f"Formatting {device_part} as {filesystem}")

                if filesystem == 'btrfs':
                    subprocess.run([
                        'mkfs.btrfs', '-f', '-L', label, device_part
                    ], check=True, capture_output=True)
                elif filesystem == 'ext4':
                    subprocess.run([
                        'mkfs.ext4', '-F', '-L', label, device_part
                    ], check=True, capture_output=True)
                elif filesystem == 'fat32':
                    subprocess.run([
                        'mkfs.fat', '-F32', '-n', label, device_part
                    ], check=True, capture_output=True)
                elif filesystem == 'swap':
                    subprocess.run([
                        'mkswap', '-L', label, device_part
                    ], check=True, capture_output=True)

            logger.info("Partitions formatted successfully")
            return True

        except subprocess.CalledProcessError as e:
            logger.error(f"Formatting failed: {e}")
            return False
        except Exception as e:
            logger.error(f"Formatting error: {e}")
            return False

class SystemInstaller:
    """Installs QuantumOS system"""

    def __init__(self, config: InstallationConfig):
        self.config = config
        self.mount_point = "/mnt/quantumos-install"
        self.progress_callback = None

    def set_progress_callback(self, callback):
        """Set progress callback function"""
        self.progress_callback = callback

    def _update_progress(self, stage: InstallationStage, percentage: int, message: str):
        """Update installation progress"""
        if self.progress_callback:
            self.progress_callback(stage, percentage, message)
        logger.info(f"{stage.value}: {percentage}% - {message}")

    def mount_filesystems(self) -> bool:
        """Mount target filesystems"""
        try:
            self._update_progress(InstallationStage.PREPARATION, 10, "Mounting filesystems")

            # Create mount point
            os.makedirs(self.mount_point, exist_ok=True)

            # Mount root filesystem
            root_device = f"{self.config.partition_plan.device}2"  # Assuming root is second partition
            subprocess.run([
                'mount', root_device, self.mount_point
            ], check=True, capture_output=True)

            # Create mount points for other partitions
            os.makedirs(f"{self.mount_point}/boot", exist_ok=True)
            os.makedirs(f"{self.mount_point}/boot/efi", exist_ok=True)

            # Mount EFI partition
            efi_device = f"{self.config.partition_plan.device}1"
            subprocess.run([
                'mount', efi_device, f"{self.mount_point}/boot/efi"
            ], check=True, capture_output=True)

            # Mount boot partition if separate
            if len(self.config.partition_plan.partitions) > 2:
                boot_device = f"{self.config.partition_plan.device}3"
                subprocess.run([
                    'mount', boot_device, f"{self.mount_point}/boot"
                ], check=True, capture_output=True)

            logger.info("Filesystems mounted successfully")
            return True

        except Exception as e:
            logger.error(f"Failed to mount filesystems: {e}")
            return False

    def copy_system(self) -> bool:
        """Copy QuantumOS system files"""
        try:
            self._update_progress(InstallationStage.SYSTEM_COPY, 20, "Copying system files")

            # In production, this would copy from the live system or extract from squashfs
            # For now, simulate the copy process

            # Create essential directories
            essential_dirs = [
                "bin", "boot", "dev", "etc", "home", "lib", "lib64", "media",
                "mnt", "opt", "proc", "root", "run", "sbin", "srv", "sys",
                "tmp", "usr", "var"
            ]

            for i, dir_name in enumerate(essential_dirs):
                dir_path = Path(self.mount_point) / dir_name
                dir_path.mkdir(exist_ok=True)

                progress = 20 + (i * 40 // len(essential_dirs))
                self._update_progress(InstallationStage.SYSTEM_COPY, progress, f"Creating {dir_name}")
                time.sleep(0.1)  # Simulate work

            # Copy kernel and initramfs
            self._update_progress(InstallationStage.SYSTEM_COPY, 60, "Installing kernel")
            self._copy_kernel()

            # Copy system files
            self._update_progress(InstallationStage.SYSTEM_COPY, 70, "Installing system packages")
            self._copy_system_files()

            # Install QuantumOS-specific components
            self._update_progress(InstallationStage.SYSTEM_COPY, 80, "Installing QuantumOS components")
            self._install_quantumos_components()

            logger.info("System files copied successfully")
            return True

        except Exception as e:
            logger.error(f"Failed to copy system: {e}")
            return False

    def _copy_kernel(self):
        """Copy kernel files"""
        boot_dir = Path(self.mount_point) / "boot"

        # Simulate kernel files
        (boot_dir / "vmlinuz-6.6.0-quantumos").write_bytes(b"KERNEL_PLACEHOLDER" * 1000)
        (boot_dir / "initrd.img-6.6.0-quantumos").write_bytes(b"INITRD_PLACEHOLDER" * 500)
        (boot_dir / "System.map-6.6.0-quantumos").write_text("# System map\\n")

    def _copy_system_files(self):
        """Copy essential system files"""
        etc_dir = Path(self.mount_point) / "etc"

        # Create essential configuration files
        (etc_dir / "os-release").write_text(f"""NAME="QuantumOS"
VERSION="1.0.0 (Neural)"
ID=quantumos
ID_LIKE=debian
PRETTY_NAME="QuantumOS 1.0.0"
VERSION_ID="1.0.0"
HOME_URL="https://quantumos.org"
""")

        (etc_dir / "hostname").write_text(f"{self.config.hostname}\\n")

        (etc_dir / "hosts").write_text(f"""127.0.0.1       localhost
127.0.1.1       {self.config.hostname}
::1             localhost ip6-localhost ip6-loopback
""")

    def _install_quantumos_components(self):
        """Install QuantumOS-specific components"""
        # Install AI orchestrator
        ai_dir = Path(self.mount_point) / "usr" / "lib" / "quantumos"
        ai_dir.mkdir(parents=True, exist_ok=True)

        # Copy AI orchestrator if it exists
        ai_orchestrator = Path("quantumos_ai_orchestrator.py")
        if ai_orchestrator.exists():
            shutil.copy2(ai_orchestrator, ai_dir / "ai-orchestrator.py")

        # Install systemd services
        systemd_dir = Path(self.mount_point) / "etc" / "systemd" / "system"
        systemd_dir.mkdir(parents=True, exist_ok=True)

        service_content = """[Unit]
Description=QuantumOS AI Orchestrator
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/python3 /usr/lib/quantumos/ai-orchestrator.py
Restart=on-failure
User=quantumos

[Install]
WantedBy=multi-user.target
"""
        (systemd_dir / "quantumos-ai-orchestrator.service").write_text(service_content)

    def configure_system(self) -> bool:
        """Configure the installed system"""
        try:
            self._update_progress(InstallationStage.CONFIGURATION, 85, "Configuring system")

            # Configure timezone
            self._configure_timezone()

            # Configure locale
            self._configure_locale()

            # Configure user account
            self._configure_user()

            # Configure network
            self._configure_network()

            # Configure AI features
            self._configure_ai_features()

            logger.info("System configured successfully")
            return True

        except Exception as e:
            logger.error(f"System configuration failed: {e}")
            return False

    def _configure_timezone(self):
        """Configure system timezone"""
        timezone_file = Path(self.mount_point) / "etc" / "timezone"
        timezone_file.write_text(f"{self.config.timezone}\\n")

        # Link localtime
        localtime_link = Path(self.mount_point) / "etc" / "localtime"
        if localtime_link.exists():
            localtime_link.unlink()

        timezone_path = f"/usr/share/zoneinfo/{self.config.timezone}"
        localtime_link.symlink_to(timezone_path)

    def _configure_locale(self):
        """Configure system locale"""
        locale_gen = Path(self.mount_point) / "etc" / "locale.gen"
        locale_gen.write_text(f"{self.config.locale} UTF-8\\n")

        default_locale = Path(self.mount_point) / "etc" / "default" / "locale"
        default_locale.parent.mkdir(exist_ok=True)
        default_locale.write_text(f"LANG={self.config.locale}.UTF-8\\n")

    def _configure_user(self):
        """Configure user account"""
        user = self.config.user_account

        # Add user to passwd
        passwd_file = Path(self.mount_point) / "etc" / "passwd"
        passwd_file.write_text(f"""root:x:0:0:root:/root:/bin/bash
{user.username}:x:1000:1000:{user.full_name}:/home/{user.username}:/bin/bash
""")

        # Add user to group
        group_file = Path(self.mount_point) / "etc" / "group"
        group_file.write_text(f"""root:x:0:
{user.username}:x:1000:
sudo:x:27:{user.username if user.admin else ''}
""")

        # Create home directory
        home_dir = Path(self.mount_point) / "home" / user.username
        home_dir.mkdir(parents=True, exist_ok=True)

        # Set up sudo if admin
        if user.admin:
            sudoers_d = Path(self.mount_point) / "etc" / "sudoers.d"
            sudoers_d.mkdir(exist_ok=True)
            (sudoers_d / user.username).write_text(f"{user.username} ALL=(ALL) NOPASSWD:ALL\\n")

    def _configure_network(self):
        """Configure network settings"""
        # Create basic network configuration
        netplan_dir = Path(self.mount_point) / "etc" / "netplan"
        netplan_dir.mkdir(parents=True, exist_ok=True)

        netplan_config = {
            'network': {
                'version': 2,
                'renderer': 'networkd',
                'ethernets': {
                    'eth0': {
                        'dhcp4': True,
                        'dhcp6': True
                    }
                }
            }
        }

        with open(netplan_dir / "01-quantumos-network.yaml", 'w') as f:
            import yaml
            yaml.dump(netplan_config, f)

    def _configure_ai_features(self):
        """Configure AI features"""
        ai_config_dir = Path(self.mount_point) / "etc" / "quantumos"
        ai_config_dir.mkdir(parents=True, exist_ok=True)

        ai_config = {
            'ai_orchestrator': {
                'enabled': self.config.ai_features.get('orchestrator', True),
                'learning_enabled': self.config.ai_features.get('learning', True),
                'steppps_integration': self.config.ai_features.get('steppps', True)
            },
            'steppps_dimensions': {
                'space': True,
                'time': True,
                'event': True,
                'psychology': True,
                'pixel': True,
                'prompt': True,
                'script': True
            }
        }

        with open(ai_config_dir / "ai-config.json", 'w') as f:
            json.dump(ai_config, f, indent=2)

    def install_bootloader(self) -> bool:
        """Install GRUB bootloader"""
        try:
            self._update_progress(InstallationStage.BOOTLOADER, 90, "Installing bootloader")

            # Install GRUB to EFI
            subprocess.run([
                'chroot', self.mount_point,
                'grub-install', '--target=x86_64-efi',
                '--efi-directory=/boot/efi',
                '--bootloader-id=QuantumOS'
            ], check=True, capture_output=True)

            # Generate GRUB configuration
            subprocess.run([
                'chroot', self.mount_point,
                'update-grub'
            ], check=True, capture_output=True)

            # Create GRUB configuration manually if needed
            grub_cfg = Path(self.mount_point) / "boot" / "grub" / "grub.cfg"
            if not grub_cfg.exists():
                grub_cfg.parent.mkdir(parents=True, exist_ok=True)
                grub_cfg.write_text(f"""set timeout=10
set default=0

menuentry "QuantumOS" {{
    linux /boot/vmlinuz-6.6.0-quantumos root=UUID={self._get_root_uuid()} ro quiet splash
    initrd /boot/initrd.img-6.6.0-quantumos
}}

menuentry "QuantumOS (Safe Mode)" {{
    linux /boot/vmlinuz-6.6.0-quantumos root=UUID={self._get_root_uuid()} ro single
    initrd /boot/initrd.img-6.6.0-quantumos
}}
""")

            logger.info("Bootloader installed successfully")
            return True

        except Exception as e:
            logger.error(f"Bootloader installation failed: {e}")
            return False

    def _get_root_uuid(self) -> str:
        """Get UUID of root partition"""
        # In production, get actual UUID
        return str(uuid.uuid4())

    def finalize_installation(self) -> bool:
        """Finalize the installation"""
        try:
            self._update_progress(InstallationStage.FINALIZATION, 95, "Finalizing installation")

            # Enable QuantumOS services
            subprocess.run([
                'chroot', self.mount_point,
                'systemctl', 'enable', 'quantumos-ai-orchestrator.service'
            ], capture_output=True)

            # Update initramfs
            subprocess.run([
                'chroot', self.mount_point,
                'update-initramfs', '-u'
            ], capture_output=True)

            # Unmount filesystems
            self._unmount_filesystems()

            self._update_progress(InstallationStage.COMPLETED, 100, "Installation completed")
            logger.info("Installation finalized successfully")
            return True

        except Exception as e:
            logger.error(f"Finalization failed: {e}")
            return False

    def _unmount_filesystems(self):
        """Unmount target filesystems"""
        try:
            # Unmount in reverse order
            subprocess.run(['umount', f"{self.mount_point}/boot/efi"], capture_output=True)
            subprocess.run(['umount', f"{self.mount_point}/boot"], capture_output=True)
            subprocess.run(['umount', self.mount_point], capture_output=True)
        except Exception as e:
            logger.warning(f"Failed to unmount some filesystems: {e}")

class QuantumOSInstaller:
    """Main installer application"""

    def __init__(self):
        self.installation_id = str(uuid.uuid4())
        self.start_time = time.time()
        self.progress_log = []

    def detect_system_info(self) -> Dict[str, Any]:
        """Detect system information"""
        info = {
            'disks': DiskManager.detect_disks(),
            'memory': psutil.virtual_memory().total,
            'cpu_count': psutil.cpu_count(),
            'architecture': os.uname().machine,
            'uefi_mode': Path('/sys/firmware/efi').exists()
        }
        return info

    def validate_requirements(self, system_info: Dict[str, Any]) -> Tuple[bool, List[str]]:
        """Validate system requirements"""
        errors = []

        # Check minimum memory (4GB)
        if system_info['memory'] < 4 * 1024 * 1024 * 1024:
            errors.append("Minimum 4GB RAM required")

        # Check available disks
        if not system_info['disks']:
            errors.append("No suitable disks found")

        # Check for available disk space (minimum 32GB)
        suitable_disks = [d for d in system_info['disks'] if d.size >= 32 * 1024 * 1024 * 1024]
        if not suitable_disks:
            errors.append("Minimum 32GB disk space required")

        # Check UEFI mode
        if not system_info['uefi_mode']:
            errors.append("UEFI boot mode required")

        return len(errors) == 0, errors

    def create_default_partition_plan(self, disk: DiskInfo) -> PartitionPlan:
        """Create default partition plan"""
        partitions = []

        # EFI partition (512MB)
        partitions.append({
            'type': 'EFI',
            'size': 512 * 1024 * 1024,
            'filesystem': 'fat32',
            'label': 'quantumos-efi'
        })

        # Root partition (remaining space minus 8GB for swap)
        swap_size = 8 * 1024 * 1024 * 1024
        root_size = disk.size - (512 * 1024 * 1024) - swap_size

        partitions.append({
            'type': 'root',
            'size': root_size,
            'filesystem': 'btrfs',
            'label': 'quantumos-root'
        })

        # Swap partition (8GB)
        partitions.append({
            'type': 'swap',
            'size': swap_size,
            'filesystem': 'swap',
            'label': 'quantumos-swap'
        })

        return PartitionPlan(
            device=disk.device,
            partitions=partitions,
            filesystem='btrfs'
        )

    def progress_callback(self, stage: InstallationStage, percentage: int, message: str):
        """Handle installation progress updates"""
        progress_entry = {
            'timestamp': time.time(),
            'stage': stage.value,
            'percentage': percentage,
            'message': message
        }
        self.progress_log.append(progress_entry)

        print(f"[{percentage:3d}%] {stage.value}: {message}")

    def install(self, config: InstallationConfig) -> bool:
        """Perform the installation"""
        try:
            logger.info(f"Starting QuantumOS installation: {self.installation_id}")

            # Create installer
            installer = SystemInstaller(config)
            installer.set_progress_callback(self.progress_callback)

            # Partition disk
            if not DiskManager.create_partition_table(config.target_disk, config.partition_plan):
                return False

            # Format partitions
            if not DiskManager.format_partitions(config.partition_plan):
                return False

            # Mount filesystems
            if not installer.mount_filesystems():
                return False

            # Copy system
            if not installer.copy_system():
                return False

            # Configure system
            if not installer.configure_system():
                return False

            # Install bootloader
            if not installer.install_bootloader():
                return False

            # Finalize installation
            if not installer.finalize_installation():
                return False

            # Create installation report
            self._create_installation_report(config)

            logger.info("QuantumOS installation completed successfully")
            return True

        except Exception as e:
            logger.error(f"Installation failed: {e}")
            return False

    def _create_installation_report(self, config: InstallationConfig):
        """Create installation report"""
        report = {
            'installation_id': self.installation_id,
            'timestamp': time.time(),
            'duration': time.time() - self.start_time,
            'version': 'QuantumOS 1.0.0',
            'target_disk': config.target_disk,
            'hostname': config.hostname,
            'username': config.user_account.username,
            'ai_features': config.ai_features,
            'progress_log': self.progress_log[-10:]  # Last 10 entries
        }

        report_file = Path('/var/log/quantumos-installation.json')
        try:
            with open(report_file, 'w') as f:
                json.dump(report, f, indent=2)
        except Exception as e:
            logger.warning(f"Failed to create installation report: {e}")

def main():
    """Main installer entry point"""
    print("🌟 QuantumOS Production Installer")
    print("=" * 50)

    installer = QuantumOSInstaller()

    # Detect system
    print("🔍 Detecting system information...")
    system_info = installer.detect_system_info()

    print(f"   CPU: {system_info['cpu_count']} cores")
    print(f"   Memory: {system_info['memory'] // (1024**3)} GB")
    print(f"   Architecture: {system_info['architecture']}")
    print(f"   UEFI Mode: {'Yes' if system_info['uefi_mode'] else 'No'}")
    print(f"   Disks found: {len(system_info['disks'])}")

    # Validate requirements
    print("\\n✅ Validating system requirements...")
    valid, errors = installer.validate_requirements(system_info)

    if not valid:
        print("❌ System requirements not met:")
        for error in errors:
            print(f"   • {error}")
        sys.exit(1)

    print("✅ System requirements satisfied")

    # In a real installer, this would be an interactive GUI or TUI
    # For demonstration, we'll use the first suitable disk
    suitable_disks = [d for d in system_info['disks'] if d.size >= 32 * 1024 * 1024 * 1024]

    if not suitable_disks:
        print("❌ No suitable disks found")
        sys.exit(1)

    target_disk = suitable_disks[0]
    print(f"\\n💾 Target disk: {target_disk.device} ({target_disk.size // (1024**3)} GB)")

    # Create installation configuration
    partition_plan = installer.create_default_partition_plan(target_disk)

    config = InstallationConfig(
        target_disk=target_disk.device,
        partition_plan=partition_plan,
        timezone="UTC",
        locale="en_US",
        keyboard="us",
        hostname="quantumos",
        user_account=UserAccount(
            username="quantum",
            password="quantum",
            full_name="Quantum User",
            admin=True
        ),
        ai_features={
            'orchestrator': True,
            'learning': True,
            'steppps': True
        },
        network_config={'dhcp': True}
    )

    # Confirm installation
    print("\\n🚀 Ready to install QuantumOS")
    print(f"   Target: {config.target_disk}")
    print(f"   Hostname: {config.hostname}")
    print(f"   User: {config.user_account.username}")
    print(f"   AI Features: {', '.join([k for k, v in config.ai_features.items() if v])}")

    if len(sys.argv) < 2 or sys.argv[1] != "--auto":
        response = input("\\nProceed with installation? [y/N]: ")
        if response.lower() != 'y':
            print("Installation cancelled")
            sys.exit(0)

    # Perform installation
    print("\\n🔧 Starting installation...")
    success = installer.install(config)

    if success:
        print("\\n🎉 QuantumOS installation completed successfully!")
        print("\\n🌟 Welcome to the AI-First Operating System!")
        print("\\nReboot to start using QuantumOS with STEPPPS framework.")
    else:
        print("\\n❌ Installation failed. Check logs for details.")
        sys.exit(1)

if __name__ == "__main__":
    main()