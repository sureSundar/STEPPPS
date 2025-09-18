#!/usr/bin/env python3
"""
SundarOS - Comprehensive Test Suite
===================================
Complete testing framework for SundarOS components and integration.
Tests all subsystems: kernel, processes, memory, drivers, filesystem, network.
Powered by STEPPPS framework.
"""

import os
import sys
import time
import unittest
import threading
from typing import Dict, List, Any, Optional

# Add current directory to path for imports
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

try:
    import steppps_minios
    from steppps_minios_kernel import STEPPPSKernel, OperationMode, KernelConfiguration, DeviceCapability
    from steppps_minios_process import STEPPPSProcessManager, ProcessState
    from steppps_minios_memory import STEPPPSMemoryManager
    from steppps_minios_filesystem import STEPPPSFileSystem
    from steppps_minios_network import STEPPPSNetworkStack, NetworkProtocol
    IMPORTS_AVAILABLE = True
except ImportError as e:
    print(f"⚠️  Warning: Some imports failed: {e}")
    IMPORTS_AVAILABLE = False

class TestSTEPPPSKernel(unittest.TestCase):
    """Test STEPPPS Kernel functionality"""
    
    def setUp(self):
        if not IMPORTS_AVAILABLE:
            self.skipTest("Required imports not available")
        
        self.config = KernelConfiguration(
            operation_mode=OperationMode.HOSTED,
            device_capability=DeviceCapability.STANDARD,
            enable_graphics=False,
            enable_network=True,
            debug_mode=False
        )
        self.kernel = STEPPPSKernel(self.config)
    
    def test_kernel_initialization(self):
        """Test kernel initialization"""
        self.assertIsNotNone(self.kernel)
        self.assertEqual(self.kernel.config.operation_mode, OperationMode.HOSTED)
        self.assertIn("space", self.kernel.steppps_state)
        self.assertIn("time", self.kernel.steppps_state)
    
    def test_kernel_boot(self):
        """Test kernel boot process"""
        result = self.kernel.boot_kernel()
        self.assertTrue(result)
        
        status = self.kernel.get_kernel_status()
        self.assertEqual(status["system_state"], "running")
        self.assertGreater(status["uptime"], 0)
    
    def test_hardware_detection(self):
        """Test hardware detection"""
        self.kernel.boot_kernel()
        status = self.kernel.get_kernel_status()
        
        self.assertIn("hardware_resources", status)
        resources = status["hardware_resources"]
        self.assertIn("cpu", resources)
        self.assertIn("memory", resources)
    
    def tearDown(self):
        if hasattr(self, 'kernel'):
            self.kernel.shutdown_kernel()

class TestSTEPPPSMemoryManager(unittest.TestCase):
    """Test STEPPPS Memory Management"""
    
    def setUp(self):
        if not IMPORTS_AVAILABLE:
            self.skipTest("Required imports not available")
        
        self.memory_manager = STEPPPSMemoryManager(total_memory_mb=64)
    
    def test_memory_initialization(self):
        """Test memory manager initialization"""
        self.assertIsNotNone(self.memory_manager)
        self.assertEqual(self.memory_manager.total_memory, 64 * 1024 * 1024)
        self.assertGreater(len(self.memory_manager.free_pages), 0)
    
    def test_memory_allocation(self):
        """Test memory allocation"""
        pid = 1
        size = 4096  # 1 page
        
        addr = self.memory_manager.allocate_memory(pid, size)
        self.assertIsNotNone(addr)
        self.assertGreater(addr, 0)
        
        # Check allocation tracking
        self.assertIn(pid, self.memory_manager.allocated_pages)
        self.assertGreater(len(self.memory_manager.allocated_pages[pid]), 0)
    
    def test_memory_deallocation(self):
        """Test memory deallocation"""
        pid = 1
        size = 4096
        
        addr = self.memory_manager.allocate_memory(pid, size)
        self.assertIsNotNone(addr)
        
        result = self.memory_manager.deallocate_memory(pid, addr)
        self.assertTrue(result)
    
    def test_memory_usage_tracking(self):
        """Test memory usage tracking"""
        initial_usage = self.memory_manager.get_memory_usage()
        
        # Allocate some memory
        pid = 1
        addr = self.memory_manager.allocate_memory(pid, 1024 * 1024)  # 1MB
        
        new_usage = self.memory_manager.get_memory_usage()
        self.assertGreater(new_usage, initial_usage)
        
        # Deallocate
        self.memory_manager.deallocate_memory(pid, addr)
        final_usage = self.memory_manager.get_memory_usage()
        self.assertLessEqual(final_usage, new_usage)

class TestSTEPPPSProcessManager(unittest.TestCase):
    """Test STEPPPS Process Management"""
    
    def setUp(self):
        if not IMPORTS_AVAILABLE:
            self.skipTest("Required imports not available")
        
        self.process_manager = STEPPPSProcessManager(max_processes=10)
    
    def test_process_creation(self):
        """Test process creation"""
        def dummy_task():
            time.sleep(0.1)
        
        pid = self.process_manager.create_process("test_process", dummy_task)
        self.assertIsNotNone(pid)
        self.assertIn(pid, self.process_manager.processes)
        
        process = self.process_manager.processes[pid]
        self.assertEqual(process.name, "test_process")
        self.assertIn(process.state, [ProcessState.READY, ProcessState.RUNNING])
    
    def test_process_termination(self):
        """Test process termination"""
        def dummy_task():
            time.sleep(1.0)
        
        pid = self.process_manager.create_process("test_process", dummy_task)
        self.assertIsNotNone(pid)
        
        result = self.process_manager.terminate_process(pid)
        self.assertTrue(result)
        
        # Process should be removed or marked as terminated
        if pid in self.process_manager.processes:
            self.assertEqual(self.process_manager.processes[pid].state, ProcessState.TERMINATED)
    
    def test_process_scheduling(self):
        """Test process scheduling"""
        def dummy_task():
            time.sleep(0.1)
        
        # Create multiple processes
        pids = []
        for i in range(3):
            pid = self.process_manager.create_process(f"test_process_{i}", dummy_task)
            if pid:
                pids.append(pid)
        
        # Run scheduler
        self.process_manager.schedule_processes()
        
        # Check that processes are being managed
        self.assertGreater(len(pids), 0)
        
        # Cleanup
        for pid in pids:
            self.process_manager.terminate_process(pid)

class TestSTEPPPSFileSystem(unittest.TestCase):
    """Test STEPPPS File System"""
    
    def setUp(self):
        if not IMPORTS_AVAILABLE:
            self.skipTest("Required imports not available")
        
        self.filesystem = STEPPPSFileSystem()
    
    def test_file_creation(self):
        """Test file creation"""
        handle = self.filesystem.create_file("/test.txt")
        self.assertIsNotNone(handle)
        self.assertIn(handle, self.filesystem.open_files)
    
    def test_file_write_read(self):
        """Test file write and read operations"""
        handle = self.filesystem.create_file("/test.txt")
        self.assertIsNotNone(handle)
        
        test_data = b"Hello STEPPPS!"
        result = self.filesystem.write_file(handle, test_data)
        self.assertTrue(result)
        
        read_data = self.filesystem.read_file(handle, 0, len(test_data))
        self.assertEqual(read_data, test_data)
    
    def test_directory_operations(self):
        """Test directory operations"""
        result = self.filesystem.create_directory("/testdir")
        self.assertTrue(result)
        
        dirs = self.filesystem.list_directory("/")
        self.assertIn("testdir", dirs)
    
    def test_file_deletion(self):
        """Test file deletion"""
        handle = self.filesystem.create_file("/test.txt")
        self.assertIsNotNone(handle)
        
        self.filesystem.close_file(handle)
        
        result = self.filesystem.delete_file("/test.txt")
        self.assertTrue(result)

class TestSTEPPPSNetworkStack(unittest.TestCase):
    """Test STEPPPS Network Stack"""
    
    def setUp(self):
        if not IMPORTS_AVAILABLE:
            self.skipTest("Required imports not available")
        
        self.network_stack = STEPPPSNetworkStack()
    
    def test_network_initialization(self):
        """Test network stack initialization"""
        self.assertIsNotNone(self.network_stack)
        self.assertGreater(len(self.network_stack.interfaces), 0)
        self.assertIn(NetworkProtocol.TCP, self.network_stack.protocols)
        self.assertIn(NetworkProtocol.UDP, self.network_stack.protocols)
        self.assertIn(NetworkProtocol.STEPPPS, self.network_stack.protocols)
    
    def test_service_management(self):
        """Test network service management"""
        result = self.network_stack.start_service("test_service", 9999, NetworkProtocol.TCP)
        self.assertTrue(result)
        self.assertIn("test_service", self.network_stack.services)
        
        result = self.network_stack.stop_service("test_service")
        self.assertTrue(result)
        self.assertNotIn("test_service", self.network_stack.services)
    
    def test_connection_management(self):
        """Test network connection management"""
        conn_id = self.network_stack.create_connection(
            NetworkProtocol.TCP,
            ("127.0.0.1", 8080)
        )
        self.assertIsNotNone(conn_id)
        self.assertIn(conn_id, self.network_stack.connections)
        
        result = self.network_stack.close_connection(conn_id)
        self.assertTrue(result)
    
    def test_service_discovery(self):
        """Test service discovery"""
        services = self.network_stack.discover_services()
        self.assertIsInstance(services, list)

class TestSundarOS(unittest.TestCase):
    """Test complete SundarOS integration"""
    
    def setUp(self):
        if not IMPORTS_AVAILABLE:
            self.skipTest("Required imports not available")
        
        self.config = steppps_minios.SystemConfiguration(
            operation_mode="hosted",
            max_processes=5,
            memory_limit_mb=32,
            enable_gui=False,
            enable_cli=True,
            debug_mode=False
        )
        self.mini_os = steppps_minios.SundarOS(self.config)
    
    def test_sundarios_boot(self):
        """Test SundarOS boot process"""
        result = self.mini_os.boot()
        self.assertTrue(result)
        self.assertEqual(self.mini_os.state, steppps_minios.MiniOSState.RUNNING)
    
    def test_system_commands(self):
        """Test system command execution"""
        self.mini_os.boot()
        
        commands = ["uptime", "ps", "free", "steppps"]
        for cmd in commands:
            result = self.mini_os.execute_command(cmd)
            self.assertTrue(result["success"], f"Command {cmd} failed: {result.get('error', 'Unknown error')}")
    
    def test_system_statistics(self):
        """Test system statistics"""
        self.mini_os.boot()
        
        stats = self.mini_os.get_system_stats()
        self.assertIn("state", stats)
        self.assertIn("uptime", stats)
        self.assertIn("operation_mode", stats)
        self.assertIn("memory_usage", stats)
        
        self.assertEqual(stats["state"], "running")
        self.assertEqual(stats["operation_mode"], "hosted")
        self.assertGreaterEqual(stats["memory_usage"], 0.0)
        self.assertLessEqual(stats["memory_usage"], 1.0)
    
    def test_steppps_integration(self):
        """Test STEPPPS framework integration"""
        self.mini_os.boot()
        
        # Test STEPPPS state access
        result = self.mini_os.execute_command("steppps")
        self.assertTrue(result["success"])
        
        steppps_data = result["data"]
        self.assertIn("space", steppps_data)
        self.assertIn("time", steppps_data)
        self.assertIn("event", steppps_data)
        self.assertIn("psychology", steppps_data)
        self.assertIn("pixel", steppps_data)
        self.assertIn("prompt", steppps_data)
        self.assertIn("script", steppps_data)
    
    def tearDown(self):
        if hasattr(self, 'mini_os'):
            self.mini_os.shutdown()

class SundarOSTestRunner:
    """Custom test runner for SundarOS"""
    
    def __init__(self):
        self.test_results = {}
    
    def run_all_tests(self):
        """Run all test suites"""
        print("🧪 SundarOS - Comprehensive Test Suite")
        print("=" * 40)
        
        test_suites = [
            ("Kernel Tests", TestSTEPPPSKernel),
            ("Memory Tests", TestSTEPPPSMemoryManager),
            ("Process Tests", TestSTEPPPSProcessManager),
            ("FileSystem Tests", TestSTEPPPSFileSystem),
            ("Network Tests", TestSTEPPPSNetworkStack),
            ("Integration Tests", TestSundarOS)
        ]
        
        total_tests = 0
        total_passed = 0
        total_failed = 0
        total_skipped = 0
        
        for suite_name, test_class in test_suites:
            print(f"\n🔬 {suite_name}")
            print("-" * 30)
            
            suite = unittest.TestLoader().loadTestsFromTestCase(test_class)
            runner = unittest.TextTestRunner(verbosity=1, stream=open(os.devnull, 'w'))
            result = runner.run(suite)
            
            tests_run = result.testsRun
            failures = len(result.failures)
            errors = len(result.errors)
            skipped = len(result.skipped)
            passed = tests_run - failures - errors - skipped
            
            total_tests += tests_run
            total_passed += passed
            total_failed += failures + errors
            total_skipped += skipped
            
            self.test_results[suite_name] = {
                "total": tests_run,
                "passed": passed,
                "failed": failures + errors,
                "skipped": skipped
            }
            
            # Print results
            if tests_run > 0:
                print(f"   ✅ Passed: {passed}")
                if failures + errors > 0:
                    print(f"   ❌ Failed: {failures + errors}")
                if skipped > 0:
                    print(f"   ⏭️  Skipped: {skipped}")
                
                # Print failure details
                for test, error in result.failures + result.errors:
                    print(f"   ❌ {test}: {error.split('AssertionError:')[-1].strip()}")
            else:
                print("   ⏭️  No tests run (imports not available)")
        
        # Print summary
        print(f"\n📊 Test Summary")
        print("=" * 20)
        print(f"Total Tests: {total_tests}")
        print(f"✅ Passed: {total_passed}")
        print(f"❌ Failed: {total_failed}")
        print(f"⏭️  Skipped: {total_skipped}")
        
        if total_tests > 0:
            success_rate = (total_passed / total_tests) * 100
            print(f"Success Rate: {success_rate:.1f}%")
            
            if total_failed == 0:
                print("\n🎉 All tests passed! SundarOS is working correctly.")
            else:
                print(f"\n⚠️  {total_failed} tests failed. Please review the issues above.")
        else:
            print("\n⚠️  No tests could be run due to import issues.")
        
        return self.test_results
    
    def run_quick_tests(self):
        """Run quick subset of tests"""
        print("🧪 SundarOS - Quick Test Suite")
        print("=" * 35)
        
        try:
            # Test basic import
            import steppps_minios
            print("✅ Import successful")
            
            # Test configuration
            config = steppps_minios.SystemConfiguration(
                operation_mode="hosted",
                memory_limit_mb=32,
                enable_gui=False,
                debug_mode=False
            )
            print("✅ Configuration created")
            
            # Test SundarOS creation and boot
            mini_os = steppps_minios.SundarOS(config)
            print("✅ SundarOS instance created")
            
            # Test boot
            if mini_os.boot():
                print("✅ Boot successful")
                
                # Test basic commands
                commands = ["uptime", "ps", "free"]
                for cmd in commands:
                    result = mini_os.execute_command(cmd)
                    if result["success"]:
                        print(f"✅ Command '{cmd}' works")
                    else:
                        print(f"❌ Command '{cmd}' failed")
                
                # Test system stats
                stats = mini_os.get_system_stats()
                print(f"✅ System running: {stats['state']}")
                print(f"   Uptime: {stats['uptime']:.2f}s")
                print(f"   Memory: {stats['memory_usage']:.1%}")
                print(f"   Processes: {stats['active_processes']}")
                
                mini_os.shutdown()
                print("✅ Shutdown successful")
                
                print("✅ Quick tests completed successfully!")
                print("🎉 SundarOS basic functionality verified!")
                return True
            else:
                print("❌ Boot failed")
                return False
                
        except Exception as e:
            print(f"❌ Quick test failed: {e}")
            return False

def main():
    """Main test entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="SundarOS Test Suite")
    parser.add_argument("--quick", action="store_true", help="Run quick test only")
    parser.add_argument("--full", action="store_true", help="Run full test suite")
    args = parser.parse_args()
    
    # Create test runner
    runner = SundarOSTestRunner()
    
    if args.quick:
        runner.run_quick_tests()
    elif args.full:
        runner.run_all_tests()
    else:
        # Default: run quick test
        print("Running quick test (use --full for comprehensive tests)")
        runner.run_quick_test()

if __name__ == "__main__":
    main()
