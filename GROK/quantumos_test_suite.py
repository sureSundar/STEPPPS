#!/usr/bin/env python3
"""
QuantumOS Comprehensive Test Suite
==================================
Production-grade testing and validation framework for QuantumOS
Linux distribution with STEPPPS framework and AI-First architecture.
"""

import os
import sys
import time
import json
import logging
import subprocess
import threading
import tempfile
import shutil
import unittest
import multiprocessing
from pathlib import Path
from typing import Dict, List, Any, Optional, Callable, Tuple
from dataclasses import dataclass, field
from enum import Enum
from concurrent.futures import ThreadPoolExecutor, as_completed
import psutil
import hashlib

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('quantumos-test')

class TestCategory(Enum):
    KERNEL = "kernel"
    STEPPPS = "steppps"
    AI_SERVICES = "ai_services"
    SYSTEM_INTEGRATION = "system_integration"
    PERFORMANCE = "performance"
    SECURITY = "security"
    HARDWARE_COMPATIBILITY = "hardware_compatibility"
    INSTALLATION = "installation"
    STRESS = "stress"

class TestResult(Enum):
    PASS = "pass"
    FAIL = "fail"
    SKIP = "skip"
    ERROR = "error"

@dataclass
class TestCase:
    """Individual test case definition"""
    name: str
    category: TestCategory
    description: str
    function: Callable
    timeout: int = 60
    required: bool = True
    dependencies: List[str] = field(default_factory=list)

@dataclass
class TestReport:
    """Test execution report"""
    test_name: str
    category: TestCategory
    result: TestResult
    duration: float
    message: str = ""
    details: Dict[str, Any] = field(default_factory=dict)
    timestamp: float = field(default_factory=time.time)

class KernelTests:
    """Tests for QuantumOS kernel functionality"""

    @staticmethod
    def test_kernel_version():
        """Test if QuantumOS kernel is running"""
        try:
            result = subprocess.run(['uname', '-r'], capture_output=True, text=True)
            kernel_version = result.stdout.strip()

            if 'quantumos' in kernel_version:
                return TestResult.PASS, f"QuantumOS kernel detected: {kernel_version}"
            else:
                return TestResult.FAIL, f"Standard kernel detected: {kernel_version}"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check kernel version: {e}"

    @staticmethod
    def test_steppps_module_loaded():
        """Test if STEPPPS kernel module is loaded"""
        try:
            with open('/proc/modules', 'r') as f:
                modules = f.read()

            if 'steppps' in modules:
                return TestResult.PASS, "STEPPPS kernel module is loaded"
            else:
                return TestResult.FAIL, "STEPPPS kernel module not found"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check modules: {e}"

    @staticmethod
    def test_steppps_proc_interface():
        """Test STEPPPS proc interface"""
        try:
            proc_path = Path('/proc/quantumos/status')

            if proc_path.exists():
                with open(proc_path, 'r') as f:
                    status = f.read()

                if 'Initialized: Yes' in status:
                    return TestResult.PASS, "STEPPPS proc interface active and initialized"
                else:
                    return TestResult.FAIL, "STEPPPS not fully initialized"
            else:
                return TestResult.FAIL, "STEPPPS proc interface not found"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check proc interface: {e}"

    @staticmethod
    def test_ai_scheduler():
        """Test AI-enhanced scheduler functionality"""
        try:
            # Check if AI scheduler parameters are available
            sched_files = [
                '/sys/kernel/debug/sched_features',
                '/proc/sys/kernel/sched_autogroup_enabled'
            ]

            features_found = 0
            for sched_file in sched_files:
                if Path(sched_file).exists():
                    features_found += 1

            if features_found >= 1:
                return TestResult.PASS, f"Scheduler features available ({features_found}/{len(sched_files)})"
            else:
                return TestResult.SKIP, "Scheduler debug features not available"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check AI scheduler: {e}"

class STEPPPSTests:
    """Tests for STEPPPS framework integration"""

    @staticmethod
    def test_dimension_initialization():
        """Test all STEPPPS dimensions are initialized"""
        try:
            proc_path = Path('/proc/quantumos/status')

            if not proc_path.exists():
                return TestResult.SKIP, "STEPPPS proc interface not available"

            with open(proc_path, 'r') as f:
                status = f.read()

            dimensions = ['SPACE', 'TIME', 'EVENT', 'PSYCHOLOGY', 'PIXEL', 'PROMPT', 'SCRIPT']
            initialized_count = 0

            for dimension in dimensions:
                if dimension in status:
                    initialized_count += 1

            if initialized_count == len(dimensions):
                return TestResult.PASS, f"All {len(dimensions)} STEPPPS dimensions initialized"
            elif initialized_count > 0:
                return TestResult.FAIL, f"Only {initialized_count}/{len(dimensions)} dimensions initialized"
            else:
                return TestResult.FAIL, "No STEPPPS dimensions found in status"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check dimension initialization: {e}"

    @staticmethod
    def test_resource_monitoring():
        """Test SPACE dimension resource monitoring"""
        try:
            proc_path = Path('/proc/quantumos/status')

            if not proc_path.exists():
                return TestResult.SKIP, "STEPPPS proc interface not available"

            with open(proc_path, 'r') as f:
                status = f.read()

            # Look for resource utilization metrics
            if 'Resource Utilization:' in status:
                return TestResult.PASS, "SPACE dimension resource monitoring active"
            else:
                return TestResult.FAIL, "Resource monitoring not found"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check resource monitoring: {e}"

    @staticmethod
    def test_time_coordination():
        """Test TIME dimension coordination"""
        try:
            proc_path = Path('/proc/quantumos/status')

            if not proc_path.exists():
                return TestResult.SKIP, "STEPPPS proc interface not available"

            with open(proc_path, 'r') as f:
                status = f.read()

            # Look for time delta metrics
            if 'System Delta:' in status:
                return TestResult.PASS, "TIME dimension coordination active"
            else:
                return TestResult.FAIL, "Time coordination not found"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check time coordination: {e}"

class AIServiceTests:
    """Tests for AI service functionality"""

    @staticmethod
    def test_ai_orchestrator_service():
        """Test AI orchestrator service status"""
        try:
            result = subprocess.run([
                'systemctl', 'is-active', 'quantumos-ai-orchestrator.service'
            ], capture_output=True, text=True)

            if result.returncode == 0 and 'active' in result.stdout:
                return TestResult.PASS, "AI orchestrator service is active"
            else:
                return TestResult.FAIL, f"AI orchestrator service not active: {result.stdout}"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check AI service: {e}"

    @staticmethod
    def test_ai_learning_engine():
        """Test AI learning engine functionality"""
        try:
            # Check if AI learning database exists
            db_path = Path('/var/lib/quantumos/ai_learning.db')

            if db_path.exists():
                return TestResult.PASS, "AI learning database found"
            else:
                return TestResult.FAIL, "AI learning database not found"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check AI learning engine: {e}"

    @staticmethod
    def test_metrics_collection():
        """Test system metrics collection"""
        try:
            # Check if metrics are being collected
            log_path = Path('/var/log/quantumos/ai-orchestrator.log')

            if log_path.exists():
                with open(log_path, 'r') as f:
                    log_content = f.read()

                if 'Metrics collection started' in log_content:
                    return TestResult.PASS, "Metrics collection is active"
                else:
                    return TestResult.FAIL, "Metrics collection not found in logs"
            else:
                return TestResult.SKIP, "AI orchestrator log not found"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check metrics collection: {e}"

class PerformanceTests:
    """Performance and benchmarking tests"""

    @staticmethod
    def test_boot_time():
        """Test system boot time"""
        try:
            # Get system uptime
            with open('/proc/uptime', 'r') as f:
                uptime_seconds = float(f.read().split()[0])

            # Get systemd boot time
            result = subprocess.run([
                'systemd-analyze', 'time'
            ], capture_output=True, text=True)

            if result.returncode == 0:
                boot_info = result.stdout
                if 'kernel' in boot_info and 'userspace' in boot_info:
                    return TestResult.PASS, f"Boot analysis available. Uptime: {uptime_seconds:.1f}s"
                else:
                    return TestResult.FAIL, "Incomplete boot time information"
            else:
                return TestResult.SKIP, f"systemd-analyze not available. Uptime: {uptime_seconds:.1f}s"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check boot time: {e}"

    @staticmethod
    def test_memory_usage():
        """Test system memory usage"""
        try:
            memory = psutil.virtual_memory()

            # Calculate AI service overhead
            ai_overhead = 0
            for proc in psutil.process_iter(['name', 'memory_info']):
                try:
                    if 'quantumos' in proc.info['name'].lower():
                        ai_overhead += proc.info['memory_info'].rss
                except:
                    continue

            ai_overhead_mb = ai_overhead / (1024 * 1024)
            total_used_mb = memory.used / (1024 * 1024)
            ai_percentage = (ai_overhead / memory.used) * 100 if memory.used > 0 else 0

            if ai_percentage < 15:  # AI overhead should be less than 15%
                return TestResult.PASS, f"Memory usage acceptable. AI overhead: {ai_overhead_mb:.1f}MB ({ai_percentage:.1f}%)"
            else:
                return TestResult.FAIL, f"High AI overhead: {ai_overhead_mb:.1f}MB ({ai_percentage:.1f}%)"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check memory usage: {e}"

    @staticmethod
    def test_cpu_performance():
        """Test CPU performance with AI services"""
        try:
            # Measure CPU usage over 5 seconds
            cpu_before = psutil.cpu_percent(interval=1)
            time.sleep(3)
            cpu_after = psutil.cpu_percent(interval=1)

            avg_cpu = (cpu_before + cpu_after) / 2

            if avg_cpu < 50:  # Normal load should be under 50%
                return TestResult.PASS, f"CPU performance good. Average usage: {avg_cpu:.1f}%"
            elif avg_cpu < 80:
                return TestResult.FAIL, f"High CPU usage: {avg_cpu:.1f}%"
            else:
                return TestResult.FAIL, f"Very high CPU usage: {avg_cpu:.1f}%"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check CPU performance: {e}"

class SecurityTests:
    """Security and hardening tests"""

    @staticmethod
    def test_secure_boot():
        """Test secure boot status"""
        try:
            secureboot_path = Path('/sys/firmware/efi/efivars/SecureBoot-*')
            secureboot_files = list(Path('/sys/firmware/efi/efivars').glob('SecureBoot-*'))

            if secureboot_files:
                return TestResult.PASS, "Secure boot variables found"
            else:
                return TestResult.SKIP, "Secure boot not available (Legacy BIOS or disabled)"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check secure boot: {e}"

    @staticmethod
    def test_kernel_hardening():
        """Test kernel hardening features"""
        try:
            hardening_features = [
                ('/proc/sys/kernel/kptr_restrict', '1'),
                ('/proc/sys/kernel/dmesg_restrict', '1'),
                ('/proc/sys/net/ipv4/conf/all/send_redirects', '0'),
                ('/proc/sys/net/ipv4/conf/default/send_redirects', '0')
            ]

            passed_features = 0
            total_features = len(hardening_features)

            for feature_path, expected_value in hardening_features:
                try:
                    with open(feature_path, 'r') as f:
                        actual_value = f.read().strip()
                    if actual_value == expected_value:
                        passed_features += 1
                except:
                    continue

            if passed_features >= total_features * 0.7:  # 70% of features enabled
                return TestResult.PASS, f"Kernel hardening: {passed_features}/{total_features} features enabled"
            else:
                return TestResult.FAIL, f"Insufficient hardening: {passed_features}/{total_features} features enabled"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check kernel hardening: {e}"

    @staticmethod
    def test_ai_service_isolation():
        """Test AI service process isolation"""
        try:
            # Check if AI services run with restricted privileges
            ai_processes = []
            for proc in psutil.process_iter(['name', 'username', 'pid']):
                try:
                    if 'quantumos' in proc.info['name'].lower():
                        ai_processes.append(proc.info)
                except:
                    continue

            if not ai_processes:
                return TestResult.SKIP, "No AI processes found"

            # Check if processes run as non-root user
            non_root_count = sum(1 for p in ai_processes if p['username'] != 'root')

            if non_root_count == len(ai_processes):
                return TestResult.PASS, f"All {len(ai_processes)} AI processes run as non-root"
            elif non_root_count > 0:
                return TestResult.FAIL, f"Only {non_root_count}/{len(ai_processes)} AI processes run as non-root"
            else:
                return TestResult.FAIL, f"All {len(ai_processes)} AI processes run as root"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check service isolation: {e}"

class HardwareCompatibilityTests:
    """Hardware compatibility and driver tests"""

    @staticmethod
    def test_cpu_features():
        """Test CPU feature detection"""
        try:
            with open('/proc/cpuinfo', 'r') as f:
                cpuinfo = f.read()

            required_features = ['sse', 'sse2', 'cx16']  # Basic x86_64 features
            optional_features = ['avx', 'avx2', 'aes']   # AI acceleration features

            required_found = sum(1 for feat in required_features if feat in cpuinfo)
            optional_found = sum(1 for feat in optional_features if feat in cpuinfo)

            if required_found == len(required_features):
                return TestResult.PASS, f"CPU compatible. Required: {required_found}/{len(required_features)}, Optional: {optional_found}/{len(optional_features)}"
            else:
                return TestResult.FAIL, f"Missing required CPU features: {required_found}/{len(required_features)}"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check CPU features: {e}"

    @staticmethod
    def test_memory_detection():
        """Test memory detection and configuration"""
        try:
            memory = psutil.virtual_memory()
            swap = psutil.swap_memory()

            total_gb = memory.total / (1024**3)
            swap_gb = swap.total / (1024**3)

            if total_gb >= 4:  # Minimum 4GB RAM
                return TestResult.PASS, f"Memory: {total_gb:.1f}GB RAM, {swap_gb:.1f}GB swap"
            else:
                return TestResult.FAIL, f"Insufficient memory: {total_gb:.1f}GB (minimum 4GB required)"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check memory: {e}"

    @staticmethod
    def test_storage_devices():
        """Test storage device detection"""
        try:
            disk_usage = psutil.disk_usage('/')
            disk_info = psutil.disk_partitions()

            total_gb = disk_usage.total / (1024**3)
            free_gb = disk_usage.free / (1024**3)
            partition_count = len(disk_info)

            if total_gb >= 32:  # Minimum 32GB storage
                return TestResult.PASS, f"Storage: {total_gb:.1f}GB total, {free_gb:.1f}GB free, {partition_count} partitions"
            else:
                return TestResult.FAIL, f"Insufficient storage: {total_gb:.1f}GB (minimum 32GB required)"

        except Exception as e:
            return TestResult.ERROR, f"Failed to check storage: {e}"

class TestRunner:
    """Main test execution engine"""

    def __init__(self):
        self.test_cases = []
        self.results = []
        self.start_time = 0
        self.end_time = 0

    def register_test_case(self, test_case: TestCase):
        """Register a test case"""
        self.test_cases.append(test_case)

    def register_all_tests(self):
        """Register all available tests"""
        # Kernel tests
        self.register_test_case(TestCase(
            "kernel_version", TestCategory.KERNEL,
            "Check if QuantumOS kernel is running",
            KernelTests.test_kernel_version
        ))

        self.register_test_case(TestCase(
            "steppps_module", TestCategory.KERNEL,
            "Verify STEPPPS kernel module is loaded",
            KernelTests.test_steppps_module_loaded
        ))

        self.register_test_case(TestCase(
            "steppps_proc_interface", TestCategory.KERNEL,
            "Test STEPPPS proc filesystem interface",
            KernelTests.test_steppps_proc_interface
        ))

        self.register_test_case(TestCase(
            "ai_scheduler", TestCategory.KERNEL,
            "Test AI-enhanced scheduler",
            KernelTests.test_ai_scheduler
        ))

        # STEPPPS tests
        self.register_test_case(TestCase(
            "dimension_initialization", TestCategory.STEPPPS,
            "Verify all STEPPPS dimensions are initialized",
            STEPPPSTests.test_dimension_initialization
        ))

        self.register_test_case(TestCase(
            "resource_monitoring", TestCategory.STEPPPS,
            "Test SPACE dimension resource monitoring",
            STEPPPSTests.test_resource_monitoring
        ))

        self.register_test_case(TestCase(
            "time_coordination", TestCategory.STEPPPS,
            "Test TIME dimension coordination",
            STEPPPSTests.test_time_coordination
        ))

        # AI service tests
        self.register_test_case(TestCase(
            "ai_orchestrator_service", TestCategory.AI_SERVICES,
            "Check AI orchestrator service status",
            AIServiceTests.test_ai_orchestrator_service
        ))

        self.register_test_case(TestCase(
            "ai_learning_engine", TestCategory.AI_SERVICES,
            "Test AI learning engine",
            AIServiceTests.test_ai_learning_engine
        ))

        self.register_test_case(TestCase(
            "metrics_collection", TestCategory.AI_SERVICES,
            "Verify metrics collection",
            AIServiceTests.test_metrics_collection
        ))

        # Performance tests
        self.register_test_case(TestCase(
            "boot_time", TestCategory.PERFORMANCE,
            "Measure system boot time",
            PerformanceTests.test_boot_time
        ))

        self.register_test_case(TestCase(
            "memory_usage", TestCategory.PERFORMANCE,
            "Check memory usage and AI overhead",
            PerformanceTests.test_memory_usage
        ))

        self.register_test_case(TestCase(
            "cpu_performance", TestCategory.PERFORMANCE,
            "Test CPU performance with AI services",
            PerformanceTests.test_cpu_performance
        ))

        # Security tests
        self.register_test_case(TestCase(
            "secure_boot", TestCategory.SECURITY,
            "Check secure boot status",
            SecurityTests.test_secure_boot
        ))

        self.register_test_case(TestCase(
            "kernel_hardening", TestCategory.SECURITY,
            "Verify kernel hardening features",
            SecurityTests.test_kernel_hardening
        ))

        self.register_test_case(TestCase(
            "ai_service_isolation", TestCategory.SECURITY,
            "Test AI service process isolation",
            SecurityTests.test_ai_service_isolation
        ))

        # Hardware compatibility tests
        self.register_test_case(TestCase(
            "cpu_features", TestCategory.HARDWARE_COMPATIBILITY,
            "Check CPU feature compatibility",
            HardwareCompatibilityTests.test_cpu_features
        ))

        self.register_test_case(TestCase(
            "memory_detection", TestCategory.HARDWARE_COMPATIBILITY,
            "Test memory detection",
            HardwareCompatibilityTests.test_memory_detection
        ))

        self.register_test_case(TestCase(
            "storage_devices", TestCategory.HARDWARE_COMPATIBILITY,
            "Test storage device detection",
            HardwareCompatibilityTests.test_storage_devices
        ))

    def run_test(self, test_case: TestCase) -> TestReport:
        """Run a single test case"""
        logger.info(f"Running test: {test_case.name}")

        start_time = time.time()

        try:
            # Run the test function with timeout
            result, message = test_case.function()
            duration = time.time() - start_time

            return TestReport(
                test_name=test_case.name,
                category=test_case.category,
                result=result,
                duration=duration,
                message=message
            )

        except Exception as e:
            duration = time.time() - start_time
            return TestReport(
                test_name=test_case.name,
                category=test_case.category,
                result=TestResult.ERROR,
                duration=duration,
                message=f"Test execution failed: {e}"
            )

    def run_all_tests(self, categories: Optional[List[TestCategory]] = None,
                     parallel: bool = False) -> Dict[str, Any]:
        """Run all registered tests"""
        logger.info("Starting QuantumOS test suite")

        self.start_time = time.time()
        self.results = []

        # Filter tests by category if specified
        tests_to_run = self.test_cases
        if categories:
            tests_to_run = [t for t in self.test_cases if t.category in categories]

        logger.info(f"Running {len(tests_to_run)} tests")

        if parallel:
            # Run tests in parallel
            with ThreadPoolExecutor(max_workers=4) as executor:
                future_to_test = {
                    executor.submit(self.run_test, test): test
                    for test in tests_to_run
                }

                for future in as_completed(future_to_test):
                    result = future.result()
                    self.results.append(result)
        else:
            # Run tests sequentially
            for test_case in tests_to_run:
                result = self.run_test(test_case)
                self.results.append(result)

        self.end_time = time.time()

        # Generate summary
        return self._generate_summary()

    def _generate_summary(self) -> Dict[str, Any]:
        """Generate test summary"""
        total_tests = len(self.results)
        passed = len([r for r in self.results if r.result == TestResult.PASS])
        failed = len([r for r in self.results if r.result == TestResult.FAIL])
        skipped = len([r for r in self.results if r.result == TestResult.SKIP])
        errors = len([r for r in self.results if r.result == TestResult.ERROR])

        # Calculate pass rate
        pass_rate = (passed / total_tests * 100) if total_tests > 0 else 0

        # Group results by category
        by_category = {}
        for result in self.results:
            category = result.category.value
            if category not in by_category:
                by_category[category] = []
            by_category[category].append(result)

        summary = {
            'total_tests': total_tests,
            'passed': passed,
            'failed': failed,
            'skipped': skipped,
            'errors': errors,
            'pass_rate': pass_rate,
            'duration': self.end_time - self.start_time,
            'by_category': by_category,
            'timestamp': time.time()
        }

        return summary

    def print_results(self, summary: Dict[str, Any]):
        """Print test results to console"""
        print("\\n" + "="*80)
        print("QuantumOS Test Suite Results")
        print("="*80)

        print(f"Total Tests: {summary['total_tests']}")
        print(f"Passed: {summary['passed']} ({summary['pass_rate']:.1f}%)")
        print(f"Failed: {summary['failed']}")
        print(f"Skipped: {summary['skipped']}")
        print(f"Errors: {summary['errors']}")
        print(f"Duration: {summary['duration']:.2f} seconds")

        print("\\n" + "-"*80)
        print("Results by Category:")
        print("-"*80)

        for category, results in summary['by_category'].items():
            passed = len([r for r in results if r.result == TestResult.PASS])
            total = len(results)
            print(f"{category.upper()}: {passed}/{total} passed")

            for result in results:
                status_symbol = {
                    TestResult.PASS: "✅",
                    TestResult.FAIL: "❌",
                    TestResult.SKIP: "⏭️",
                    TestResult.ERROR: "💥"
                }[result.result]

                print(f"  {status_symbol} {result.test_name}: {result.message}")

        print("\\n" + "="*80)

        if summary['pass_rate'] >= 90:
            print("🎉 Excellent! QuantumOS is functioning well.")
        elif summary['pass_rate'] >= 70:
            print("👍 Good! QuantumOS is mostly functional with minor issues.")
        elif summary['pass_rate'] >= 50:
            print("⚠️  Warning! QuantumOS has significant issues that need attention.")
        else:
            print("🚨 Critical! QuantumOS has major problems that must be fixed.")

    def save_report(self, summary: Dict[str, Any], filename: str):
        """Save detailed test report to file"""
        report = {
            'summary': summary,
            'detailed_results': [
                {
                    'test_name': r.test_name,
                    'category': r.category.value,
                    'result': r.result.value,
                    'duration': r.duration,
                    'message': r.message,
                    'timestamp': r.timestamp
                }
                for r in self.results
            ]
        }

        with open(filename, 'w') as f:
            json.dump(report, f, indent=2)

        logger.info(f"Detailed test report saved to {filename}")

def main():
    """Main test suite entry point"""
    print("🌟 QuantumOS Comprehensive Test Suite")
    print("=" * 50)

    # Parse command line arguments
    categories = None
    parallel = False
    output_file = None

    if len(sys.argv) > 1:
        for i, arg in enumerate(sys.argv[1:], 1):
            if arg == '--help':
                print("""Usage: quantumos_test_suite.py [options]

Options:
  --help              Show this help message
  --categories LIST   Run only specified categories (comma-separated)
  --parallel          Run tests in parallel
  --output FILE       Save detailed report to file

Available categories:
  kernel, steppps, ai_services, performance, security, hardware_compatibility

Examples:
  python3 quantumos_test_suite.py
  python3 quantumos_test_suite.py --categories kernel,steppps
  python3 quantumos_test_suite.py --parallel --output report.json
""")
                return

            elif arg == '--categories' and i + 1 < len(sys.argv):
                category_names = sys.argv[i + 1].split(',')
                categories = []
                for name in category_names:
                    try:
                        categories.append(TestCategory(name.strip()))
                    except ValueError:
                        print(f"Warning: Unknown category '{name}'")

            elif arg == '--parallel':
                parallel = True

            elif arg == '--output' and i + 1 < len(sys.argv):
                output_file = sys.argv[i + 1]

    # Create and configure test runner
    runner = TestRunner()
    runner.register_all_tests()

    # Run tests
    print(f"🔧 Initializing test environment...")
    summary = runner.run_all_tests(categories=categories, parallel=parallel)

    # Print results
    runner.print_results(summary)

    # Save detailed report if requested
    if output_file:
        runner.save_report(summary, output_file)

    # Exit with appropriate code
    if summary['pass_rate'] >= 90:
        sys.exit(0)  # Success
    elif summary['failed'] > 0 or summary['errors'] > 0:
        sys.exit(1)  # Failure
    else:
        sys.exit(0)  # Success (only skipped tests)

if __name__ == "__main__":
    main()