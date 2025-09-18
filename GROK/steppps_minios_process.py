#!/usr/bin/env python3
"""
STEPPPS Mini OS - Process Management System
==========================================
STEPPPS-native process management with autonomous scheduling and AI-driven optimization.
Each process contains full STEPPPS dimensions for autonomous behavior.

Architecture:
- SPACE: Process memory layout and resource allocation
- TIME: Process scheduling and temporal coordination
- EVENT: Inter-process communication and signals
- PSYCHOLOGY: Process behavior and adaptation
- PIXEL: Process display and visualization
- PROMPT: Process AI interface and automation
- SCRIPT: Process execution and orchestration
"""

import os
import sys
import time
import json
import threading
import queue
import signal
import subprocess
from typing import Dict, List, Any, Optional, Callable, Union
from dataclasses import dataclass, field
from enum import Enum
from abc import ABC, abstractmethod

# ============================================================================
# PROCESS ENUMS AND DATA STRUCTURES
# ============================================================================

class ProcessState(Enum):
    """Process execution states"""
    NEW = "new"
    READY = "ready"
    RUNNING = "running"
    BLOCKED = "blocked"
    SUSPENDED = "suspended"
    TERMINATED = "terminated"
    ZOMBIE = "zombie"

class ProcessPriority(Enum):
    """Process priority levels"""
    CRITICAL = 0    # System critical processes
    HIGH = 1        # High priority user processes
    NORMAL = 5      # Normal user processes
    LOW = 10        # Background processes
    IDLE = 15       # Idle time processes

class ProcessType(Enum):
    """Process types"""
    SYSTEM = "system"       # System processes
    USER = "user"           # User processes
    DAEMON = "daemon"       # Background daemons
    STEPPPS = "steppps"     # STEPPPS autonomous processes

class IPCType(Enum):
    """Inter-process communication types"""
    PIPE = "pipe"
    QUEUE = "queue"
    SHARED_MEMORY = "shared_memory"
    SIGNAL = "signal"
    SOCKET = "socket"

@dataclass
class ProcessMemory:
    """Process memory layout"""
    virtual_base: int = 0x10000000
    heap_size: int = 1024 * 1024  # 1MB
    stack_size: int = 64 * 1024   # 64KB
    code_size: int = 0
    data_size: int = 0
    allocated_pages: List[int] = field(default_factory=list)
    memory_usage: int = 0

@dataclass
class ProcessSTEPPPS:
    """STEPPPS dimensions for each process"""
    space: Dict[str, Any] = field(default_factory=dict)
    time: Dict[str, Any] = field(default_factory=dict)
    event: Dict[str, Any] = field(default_factory=dict)
    psychology: Dict[str, Any] = field(default_factory=dict)
    pixel: Dict[str, Any] = field(default_factory=dict)
    prompt: Dict[str, Any] = field(default_factory=dict)
    script: Dict[str, Any] = field(default_factory=dict)

@dataclass
class STEPPPSProcess:
    """STEPPPS-enhanced process control block"""
    pid: int
    ppid: int
    name: str
    process_type: ProcessType
    state: ProcessState = ProcessState.NEW
    priority: ProcessPriority = ProcessPriority.NORMAL
    
    # Process execution context
    entry_point: Optional[Callable] = None
    arguments: List[str] = field(default_factory=list)
    environment: Dict[str, str] = field(default_factory=dict)
    
    # Process resources
    memory: ProcessMemory = field(default_factory=ProcessMemory)
    cpu_time: float = 0.0
    io_time: float = 0.0
    
    # Process timing
    created_time: float = field(default_factory=time.time)
    start_time: Optional[float] = None
    end_time: Optional[float] = None
    
    # STEPPPS dimensions
    steppps: ProcessSTEPPPS = field(default_factory=ProcessSTEPPPS)
    
    # Process communication
    ipc_channels: Dict[str, Any] = field(default_factory=dict)
    signals: queue.Queue = field(default_factory=queue.Queue)
    
    # Process thread
    thread: Optional[threading.Thread] = None
    
    def __post_init__(self):
        """Initialize STEPPPS dimensions after creation"""
        self._init_steppps_dimensions()
    
    def _init_steppps_dimensions(self):
        """Initialize STEPPPS dimensions for the process"""
        self.steppps.space = {
            "memory_layout": {
                "virtual_base": self.memory.virtual_base,
                "heap_size": self.memory.heap_size,
                "stack_size": self.memory.stack_size
            },
            "resource_allocation": {},
            "working_directory": os.getcwd(),
            "file_descriptors": {}
        }
        
        self.steppps.time = {
            "created": self.created_time,
            "quantum": 100,  # milliseconds
            "deadline": None,
            "periodic": False,
            "scheduling_policy": "round_robin"
        }
        
        self.steppps.event = {
            "signal_handlers": {},
            "event_subscriptions": [],
            "ipc_endpoints": {},
            "notification_queue": queue.Queue()
        }
        
        self.steppps.psychology = {
            "behavior_mode": "autonomous" if self.process_type == ProcessType.STEPPPS else "standard",
            "adaptation_enabled": True,
            "learning_rate": 0.1,
            "performance_metrics": {},
            "optimization_goals": ["efficiency", "responsiveness"]
        }
        
        self.steppps.pixel = {
            "display_region": None,
            "visualization_enabled": False,
            "ui_components": {},
            "rendering_context": None
        }
        
        self.steppps.prompt = {
            "ai_enabled": self.process_type == ProcessType.STEPPPS,
            "automation_level": "full" if self.process_type == ProcessType.STEPPPS else "none",
            "command_interface": None,
            "response_handlers": {}
        }
        
        self.steppps.script = {
            "execution_context": {},
            "service_dependencies": [],
            "orchestration_rules": {},
            "callback_handlers": {}
        }

# ============================================================================
# PROCESS SCHEDULER INTERFACES
# ============================================================================

class IProcessScheduler(ABC):
    """Interface for process schedulers"""
    
    @abstractmethod
    def schedule_process(self, process: STEPPPSProcess) -> bool:
        """Schedule a process for execution"""
        pass
    
    @abstractmethod
    def get_next_process(self) -> Optional[STEPPPSProcess]:
        """Get next process to run"""
        pass
    
    @abstractmethod
    def update_priority(self, pid: int, priority: ProcessPriority) -> bool:
        """Update process priority"""
        pass

class IProcessCommunication(ABC):
    """Interface for inter-process communication"""
    
    @abstractmethod
    def create_channel(self, pid1: int, pid2: int, channel_type: IPCType) -> str:
        """Create IPC channel between processes"""
        pass
    
    @abstractmethod
    def send_message(self, channel_id: str, message: Any) -> bool:
        """Send message through IPC channel"""
        pass
    
    @abstractmethod
    def receive_message(self, channel_id: str, timeout: float = None) -> Any:
        """Receive message from IPC channel"""
        pass

# ============================================================================
# STEPPPS PROCESS SCHEDULER
# ============================================================================

class STEPPPSProcessScheduler(IProcessScheduler):
    """STEPPPS-enhanced process scheduler with AI optimization"""
    
    def __init__(self, scheduling_algorithm: str = "adaptive_round_robin"):
        self.algorithm = scheduling_algorithm
        self.ready_queue = queue.PriorityQueue()
        self.running_process: Optional[STEPPPSProcess] = None
        self.blocked_processes: Dict[int, STEPPPSProcess] = {}
        
        # Scheduling statistics
        self.schedule_count = 0
        self.context_switches = 0
        self.total_wait_time = 0.0
        
        # AI-driven optimization
        self.performance_history = []
        self.optimization_enabled = True
        
    def schedule_process(self, process: STEPPPSProcess) -> bool:
        """Schedule process with STEPPPS-aware priority calculation"""
        try:
            # Calculate dynamic priority based on STEPPPS dimensions
            dynamic_priority = self._calculate_dynamic_priority(process)
            
            # Add to ready queue with priority
            self.ready_queue.put((dynamic_priority, process.pid, process))
            process.state = ProcessState.READY
            
            # Update STEPPPS time dimension
            process.steppps.time["scheduled_at"] = time.time()
            
            self.schedule_count += 1
            return True
            
        except Exception as e:
            print(f"❌ Failed to schedule process {process.pid}: {e}")
            return False
    
    def get_next_process(self) -> Optional[STEPPPSProcess]:
        """Get next process using STEPPPS-enhanced scheduling"""
        try:
            if self.ready_queue.empty():
                return None
            
            # Get highest priority process
            priority, pid, process = self.ready_queue.get()
            
            # Update scheduling statistics
            if self.running_process:
                self.context_switches += 1
            
            # Calculate wait time
            wait_time = time.time() - process.steppps.time.get("scheduled_at", time.time())
            self.total_wait_time += wait_time
            
            # Update process state
            process.state = ProcessState.RUNNING
            process.start_time = time.time()
            process.steppps.time["last_run"] = time.time()
            
            self.running_process = process
            
            # AI optimization
            if self.optimization_enabled:
                self._record_scheduling_decision(process, wait_time)
            
            return process
            
        except queue.Empty:
            return None
        except Exception as e:
            print(f"❌ Scheduler error: {e}")
            return None
    
    def update_priority(self, pid: int, priority: ProcessPriority) -> bool:
        """Update process priority"""
        # Implementation would update priority in ready queue
        return True
    
    def _calculate_dynamic_priority(self, process: STEPPPSProcess) -> int:
        """Calculate dynamic priority based on STEPPPS dimensions"""
        base_priority = process.priority.value
        
        # PSYCHOLOGY dimension adjustments
        psychology = process.steppps.psychology
        if psychology.get("behavior_mode") == "autonomous":
            base_priority -= 1  # Higher priority for autonomous processes
        
        # TIME dimension adjustments
        time_info = process.steppps.time
        if time_info.get("deadline"):
            deadline = time_info["deadline"]
            if deadline - time.time() < 1.0:  # Urgent deadline
                base_priority -= 2
        
        # SPACE dimension adjustments
        space_info = process.steppps.space
        memory_usage = space_info.get("memory_usage", 0)
        if memory_usage > 50:  # High memory usage
            base_priority += 1  # Lower priority
        
        return max(0, base_priority)
    
    def _record_scheduling_decision(self, process: STEPPPSProcess, wait_time: float):
        """Record scheduling decision for AI optimization"""
        decision_record = {
            "timestamp": time.time(),
            "pid": process.pid,
            "priority": process.priority.value,
            "wait_time": wait_time,
            "process_type": process.process_type.value,
            "steppps_behavior": process.steppps.psychology.get("behavior_mode", "standard")
        }
        
        self.performance_history.append(decision_record)
        
        # Keep only recent history
        if len(self.performance_history) > 1000:
            self.performance_history = self.performance_history[-500:]
    
    def get_scheduler_stats(self) -> Dict[str, Any]:
        """Get scheduler performance statistics"""
        avg_wait_time = self.total_wait_time / max(1, self.schedule_count)
        
        return {
            "algorithm": self.algorithm,
            "processes_scheduled": self.schedule_count,
            "context_switches": self.context_switches,
            "average_wait_time": avg_wait_time,
            "ready_queue_size": self.ready_queue.qsize(),
            "blocked_processes": len(self.blocked_processes),
            "optimization_enabled": self.optimization_enabled,
            "performance_samples": len(self.performance_history)
        }

# ============================================================================
# STEPPPS PROCESS COMMUNICATION
# ============================================================================

class STEPPPSProcessCommunication(IProcessCommunication):
    """STEPPPS-enhanced inter-process communication"""
    
    def __init__(self):
        self.channels: Dict[str, Dict[str, Any]] = {}
        self.message_queues: Dict[str, queue.Queue] = {}
        self.shared_memory: Dict[str, Dict[str, Any]] = {}
        self.signal_handlers: Dict[int, Dict[int, Callable]] = {}
        
    def create_channel(self, pid1: int, pid2: int, channel_type: IPCType) -> str:
        """Create IPC channel with STEPPPS integration"""
        channel_id = f"{channel_type.value}_{pid1}_{pid2}_{int(time.time())}"
        
        channel_info = {
            "id": channel_id,
            "type": channel_type,
            "participants": [pid1, pid2],
            "created_at": time.time(),
            "message_count": 0,
            "steppps_enabled": True
        }
        
        if channel_type == IPCType.QUEUE:
            self.message_queues[channel_id] = queue.Queue()
        elif channel_type == IPCType.SHARED_MEMORY:
            self.shared_memory[channel_id] = {}
        
        self.channels[channel_id] = channel_info
        return channel_id
    
    def send_message(self, channel_id: str, message: Any) -> bool:
        """Send message with STEPPPS metadata"""
        try:
            if channel_id not in self.channels:
                return False
            
            channel = self.channels[channel_id]
            
            # Wrap message with STEPPPS metadata
            steppps_message = {
                "content": message,
                "timestamp": time.time(),
                "channel_id": channel_id,
                "steppps_metadata": {
                    "sender_psychology": {"intent": "communication"},
                    "message_priority": "normal",
                    "delivery_mode": "async"
                }
            }
            
            if channel["type"] == IPCType.QUEUE:
                self.message_queues[channel_id].put(steppps_message)
            elif channel["type"] == IPCType.SHARED_MEMORY:
                self.shared_memory[channel_id]["latest_message"] = steppps_message
            
            channel["message_count"] += 1
            return True
            
        except Exception as e:
            print(f"❌ Failed to send message: {e}")
            return False
    
    def receive_message(self, channel_id: str, timeout: float = None) -> Any:
        """Receive message with STEPPPS processing"""
        try:
            if channel_id not in self.channels:
                return None
            
            channel = self.channels[channel_id]
            
            if channel["type"] == IPCType.QUEUE:
                try:
                    message = self.message_queues[channel_id].get(timeout=timeout)
                    return message["content"]  # Return unwrapped content
                except queue.Empty:
                    return None
            elif channel["type"] == IPCType.SHARED_MEMORY:
                return self.shared_memory[channel_id].get("latest_message", {}).get("content")
            
            return None
            
        except Exception as e:
            print(f"❌ Failed to receive message: {e}")
            return None

# ============================================================================
# STEPPPS PROCESS MANAGER
# ============================================================================

class STEPPPSProcessManager:
    """Main process management system with STEPPPS integration"""
    
    def __init__(self, max_processes: int = 256):
        self.max_processes = max_processes
        self.processes: Dict[int, STEPPPSProcess] = {}
        self.next_pid = 1
        
        # Core components
        self.scheduler = STEPPPSProcessScheduler()
        self.ipc = STEPPPSProcessCommunication()
        
        # Process management state
        self.running = False
        self.scheduler_thread: Optional[threading.Thread] = None
        
        # Statistics
        self.total_processes_created = 0
        self.total_processes_terminated = 0
        
        print("🔄 STEPPPS Process Manager initialized")
    
    def create_process(self, name: str, entry_point: Callable, 
                      process_type: ProcessType = ProcessType.USER,
                      priority: ProcessPriority = ProcessPriority.NORMAL,
                      arguments: List[str] = None) -> Optional[int]:
        """Create new STEPPPS process"""
        try:
            if len(self.processes) >= self.max_processes:
                print(f"❌ Maximum processes ({self.max_processes}) reached")
                return None
            
            # Create process
            pid = self.next_pid
            self.next_pid += 1
            
            process = STEPPPSProcess(
                pid=pid,
                ppid=os.getpid(),  # Parent is the OS
                name=name,
                process_type=process_type,
                priority=priority,
                entry_point=entry_point,
                arguments=arguments or []
            )
            
            # Initialize process environment
            process.environment = dict(os.environ)
            process.environment["STEPPPS_PID"] = str(pid)
            process.environment["STEPPPS_TYPE"] = process_type.value
            
            # Add to process table
            self.processes[pid] = process
            self.total_processes_created += 1
            
            print(f"📦 Process created: {name} (PID: {pid}, Type: {process_type.value})")
            return pid
            
        except Exception as e:
            print(f"❌ Failed to create process: {e}")
            return None
    
    def start_process(self, pid: int) -> bool:
        """Start process execution"""
        try:
            if pid not in self.processes:
                return False
            
            process = self.processes[pid]
            
            if process.state != ProcessState.NEW:
                return False
            
            # Create and start thread
            process.thread = threading.Thread(
                target=self._process_wrapper,
                args=(process,),
                name=f"STEPPPS-{process.name}-{pid}"
            )
            
            process.thread.start()
            
            # Schedule process
            return self.scheduler.schedule_process(process)
            
        except Exception as e:
            print(f"❌ Failed to start process {pid}: {e}")
            return False
    
    def terminate_process(self, pid: int, force: bool = False) -> bool:
        """Terminate process"""
        try:
            if pid not in self.processes:
                return False
            
            process = self.processes[pid]
            
            # Send termination signal
            if not force and process.thread and process.thread.is_alive():
                # Graceful termination
                process.signals.put({"type": "SIGTERM", "timestamp": time.time()})
                process.thread.join(timeout=5.0)
            
            # Force termination if needed
            if force or (process.thread and process.thread.is_alive()):
                process.state = ProcessState.TERMINATED
            
            # Cleanup
            process.end_time = time.time()
            self.total_processes_terminated += 1
            
            # Remove from active processes after delay (zombie state)
            threading.Timer(1.0, lambda: self.processes.pop(pid, None)).start()
            
            print(f"🗑️  Process terminated: {process.name} (PID: {pid})")
            return True
            
        except Exception as e:
            print(f"❌ Failed to terminate process {pid}: {e}")
            return False
    
    def _process_wrapper(self, process: STEPPPSProcess):
        """Wrapper for process execution with STEPPPS integration"""
        try:
            # Update process state
            process.state = ProcessState.RUNNING
            process.start_time = time.time()
            
            # STEPPPS initialization for the process
            self._init_process_steppps(process)
            
            # Execute process entry point
            if process.entry_point:
                result = process.entry_point(*process.arguments)
                process.steppps.script["exit_code"] = result if isinstance(result, int) else 0
            
            # Process completed normally
            process.state = ProcessState.TERMINATED
            process.end_time = time.time()
            
        except Exception as e:
            print(f"❌ Process {process.pid} crashed: {e}")
            process.state = ProcessState.TERMINATED
            process.steppps.script["exit_code"] = -1
            process.steppps.script["error"] = str(e)
    
    def _init_process_steppps(self, process: STEPPPSProcess):
        """Initialize STEPPPS dimensions for running process"""
        
        # SPACE: Setup process memory and resources
        process.steppps.space["memory_usage"] = process.memory.memory_usage
        process.steppps.space["resource_limits"] = {
            "max_memory": process.memory.heap_size,
            "max_files": 256,
            "max_threads": 10
        }
        
        # TIME: Setup timing and scheduling
        process.steppps.time["start_time"] = process.start_time
        process.steppps.time["cpu_time"] = 0.0
        
        # EVENT: Setup event handling
        process.steppps.event["signal_handlers"]["SIGTERM"] = self._handle_sigterm
        
        # PSYCHOLOGY: Initialize behavior
        if process.process_type == ProcessType.STEPPPS:
            process.steppps.psychology["autonomous_mode"] = True
            process.steppps.psychology["adaptation_enabled"] = True
        
        # PROMPT: Setup AI interface if enabled
        if process.steppps.prompt["ai_enabled"]:
            process.steppps.prompt["ai_context"] = {
                "process_name": process.name,
                "process_type": process.process_type.value,
                "capabilities": ["autonomous_execution", "self_optimization"]
            }
    
    def _handle_sigterm(self, process: STEPPPSProcess):
        """Handle SIGTERM signal for graceful shutdown"""
        process.steppps.psychology["shutdown_requested"] = True
        process.state = ProcessState.TERMINATED
    
    def start_scheduler(self):
        """Start the process scheduler"""
        if self.running:
            return
        
        self.running = True
        self.scheduler_thread = threading.Thread(
            target=self._scheduler_loop,
            name="STEPPPS-Scheduler"
        )
        self.scheduler_thread.start()
        print("⚡ STEPPPS Process Scheduler started")
    
    def stop_scheduler(self):
        """Stop the process scheduler"""
        self.running = False
        if self.scheduler_thread:
            self.scheduler_thread.join()
        print("🛑 STEPPPS Process Scheduler stopped")
    
    def _scheduler_loop(self):
        """Main scheduler loop"""
        while self.running:
            try:
                # Get next process to run
                next_process = self.scheduler.get_next_process()
                
                if next_process:
                    # Process gets CPU time slice
                    time.sleep(0.01)  # 10ms time slice simulation
                    
                    # Update CPU time
                    next_process.cpu_time += 0.01
                    next_process.steppps.time["cpu_time"] = next_process.cpu_time
                    
                    # Reschedule if still running
                    if next_process.state == ProcessState.RUNNING:
                        next_process.state = ProcessState.READY
                        self.scheduler.schedule_process(next_process)
                
                else:
                    # No processes to run, idle
                    time.sleep(0.1)
                    
            except Exception as e:
                print(f"❌ Scheduler error: {e}")
                time.sleep(0.1)
    
    def get_process_list(self) -> List[Dict[str, Any]]:
        """Get list of all processes"""
        process_list = []
        
        for pid, process in self.processes.items():
            process_info = {
                "pid": pid,
                "ppid": process.ppid,
                "name": process.name,
                "type": process.process_type.value,
                "state": process.state.value,
                "priority": process.priority.value,
                "cpu_time": process.cpu_time,
                "memory_usage": process.memory.memory_usage,
                "created_time": process.created_time,
                "steppps_enabled": True,
                "autonomous": process.steppps.psychology.get("behavior_mode") == "autonomous"
            }
            process_list.append(process_info)
        
        return process_list
    
    def get_system_stats(self) -> Dict[str, Any]:
        """Get process management statistics"""
        return {
            "total_processes": len(self.processes),
            "max_processes": self.max_processes,
            "processes_created": self.total_processes_created,
            "processes_terminated": self.total_processes_terminated,
            "scheduler_running": self.running,
            "scheduler_stats": self.scheduler.get_scheduler_stats(),
            "process_states": {
                state.value: len([p for p in self.processes.values() if p.state == state])
                for state in ProcessState
            }
        }

# ============================================================================
# DEMO PROCESSES
# ============================================================================

def demo_user_process(message: str = "Hello from user process"):
    """Demo user process"""
    print(f"👤 User Process: {message}")
    time.sleep(2)
    return 0

def demo_steppps_process():
    """Demo STEPPPS autonomous process"""
    print("🤖 STEPPPS Autonomous Process starting...")
    
    # Simulate autonomous behavior
    for i in range(5):
        print(f"🧠 STEPPPS Process: Autonomous iteration {i+1}")
        time.sleep(1)
    
    print("🤖 STEPPPS Process completed autonomously")
    return 0

def demo_system_process():
    """Demo system process"""
    print("⚙️  System Process: Monitoring system health...")
    
    for i in range(10):
        print(f"📊 System Monitor: Check {i+1}/10")
        time.sleep(0.5)
    
    return 0

def main():
    """Main entry point for process management demo"""
    print("🔄 STEPPPS Process Management System")
    print("=" * 40)
    
    # Create process manager
    pm = STEPPPSProcessManager(max_processes=10)
    
    try:
        # Start scheduler
        pm.start_scheduler()
        
        # Create demo processes
        user_pid = pm.create_process("UserDemo", demo_user_process, ProcessType.USER)
        steppps_pid = pm.create_process("STEPPPSDemo", demo_steppps_process, ProcessType.STEPPPS)
        system_pid = pm.create_process("SystemMonitor", demo_system_process, ProcessType.SYSTEM, ProcessPriority.HIGH)
        
        # Start processes
        if user_pid:
            pm.start_process(user_pid)
        if steppps_pid:
            pm.start_process(steppps_pid)
        if system_pid:
            pm.start_process(system_pid)
        
        # Monitor for a while
        for i in range(15):
            time.sleep(1)
            stats = pm.get_system_stats()
            print(f"\n📊 System Stats (t={i+1}s):")
            print(f"   Active Processes: {stats['total_processes']}")
            print(f"   Scheduler: {'Running' if stats['scheduler_running'] else 'Stopped'}")
            
            # Show process list
            processes = pm.get_process_list()
            for proc in processes:
                print(f"   PID {proc['pid']}: {proc['name']} ({proc['state']}) - CPU: {proc['cpu_time']:.2f}s")
        
        print("\n🏁 Demo completed")
        
    except KeyboardInterrupt:
        print("\n🛑 Interrupted")
    finally:
        # Cleanup
        pm.stop_scheduler()
        
        # Terminate all processes
        for pid in list(pm.processes.keys()):
            pm.terminate_process(pid)

if __name__ == "__main__":
    main()
