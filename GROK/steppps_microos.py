#!/usr/bin/env python3
"""
STEPPPS MicroOS - Universal Operating System Framework
=====================================================
A micro operating system built on STEPPPS framework that runs on any digital device.
Canvas-based display with hardware abstraction through STEPPPS dimensions.

Architecture:
- SPACE: Hardware abstraction layer (CPU, Memory, Storage, Network)
- TIME: Process scheduling and temporal management
- EVENT: Interrupt handling and system events
- PSYCHOLOGY: Process management and system behavior
- PIXEL: Canvas-based display system and graphics
- PROMPT: User interface and AI interaction
- SCRIPT: System services and application orchestration
"""

import json
import time
import os
import sys
import threading
import queue
import subprocess
from typing import Dict, List, Any, Optional, Callable
from dataclasses import dataclass, field
from enum import Enum

try:
    import tkinter as tk
    from tkinter import ttk, messagebox, filedialog
    DISPLAY_AVAILABLE = True
except ImportError:
    DISPLAY_AVAILABLE = False

class DeviceCapability(Enum):
    MINIMAL = "minimal"      # Calculator, IoT devices
    BASIC = "basic"          # Raspberry Pi, embedded systems
    STANDARD = "standard"    # Laptops, desktops
    ADVANCED = "advanced"    # Servers, workstations

class ProcessState(Enum):
    READY = "ready"
    RUNNING = "running"
    BLOCKED = "blocked"
    TERMINATED = "terminated"

@dataclass
class STEPPPSProcess:
    """STEPPPS-based process representation"""
    pid: int
    name: str
    state: ProcessState = ProcessState.READY
    priority: int = 5
    memory_usage: int = 0
    cpu_time: float = 0.0
    created_time: float = field(default_factory=time.time)
    steppps_data: Dict[str, Any] = field(default_factory=dict)
    canvas_region: Optional[Dict[str, int]] = None

@dataclass
class HardwareResource:
    """Hardware resource abstraction"""
    name: str
    type: str
    available: bool = True
    capacity: int = 100
    usage: int = 0
    properties: Dict[str, Any] = field(default_factory=dict)

class STEPPPSMicroOS:
    """
    STEPPPS MicroOS - Universal Operating System
    Runs on any digital device with canvas-based display
    """
    
    def __init__(self, device_type: str = "auto"):
        self.device_capability = self._detect_device_capability(device_type)
        self.running = False
        self.boot_time = time.time()
        
        # Canvas Display System - Initialize before STEPPPS dimensions
        self.canvas_width = 800
        self.canvas_height = 600
        self.canvas = None
        self.root_window = None
        
        # OS Core Components - Initialize before STEPPPS dimensions
        self.processes: Dict[int, STEPPPSProcess] = {}
        self.next_pid = 1
        self.scheduler_queue = queue.PriorityQueue()
        self.event_queue = queue.Queue()
        self.hardware_resources: Dict[str, HardwareResource] = {}
        
        # STEPPPS Dimensions as OS Subsystems
        self.space = self._init_space_subsystem()      # Hardware abstraction
        self.time = self._init_time_subsystem()        # Process scheduling
        self.event = self._init_event_subsystem()      # Event handling
        self.psychology = self._init_psychology_subsystem()  # Process management
        self.pixel = self._init_pixel_subsystem()      # Display system
        self.prompt = self._init_prompt_subsystem()    # User interface
        self.script = self._init_script_subsystem()    # Service orchestration
        
        print(f"🖥️  STEPPPS MicroOS v1.0 - {self.device_capability.value.upper()} MODE")
        
    def _detect_device_capability(self, device_type: str) -> DeviceCapability:
        """Detect device capability based on system resources"""
        if device_type != "auto":
            return DeviceCapability(device_type)
            
        try:
            # Simple heuristics for device classification
            import psutil
            memory_gb = psutil.virtual_memory().total / (1024**3)
            cpu_count = psutil.cpu_count()
            
            if memory_gb < 1 or cpu_count < 2:
                return DeviceCapability.MINIMAL
            elif memory_gb < 4 or cpu_count < 4:
                return DeviceCapability.BASIC
            elif memory_gb < 16:
                return DeviceCapability.STANDARD
            else:
                return DeviceCapability.ADVANCED
        except ImportError:
            # Fallback without psutil
            return DeviceCapability.BASIC
    
    def _init_space_subsystem(self) -> Dict[str, Any]:
        """Initialize SPACE dimension - Hardware Abstraction Layer"""
        space = {
            "hardware_detected": False,
            "cpu_cores": 1,
            "memory_mb": 512,
            "storage_mb": 1024,
            "network_interfaces": [],
            "display_capability": DISPLAY_AVAILABLE,
            "device_type": self.device_capability.value
        }
        
        # Detect actual hardware if possible
        try:
            import psutil
            space["cpu_cores"] = psutil.cpu_count()
            space["memory_mb"] = int(psutil.virtual_memory().total / (1024*1024))
            space["storage_mb"] = int(psutil.disk_usage('/').total / (1024*1024))
            space["hardware_detected"] = True
            
            # Network interfaces
            for interface, addrs in psutil.net_if_addrs().items():
                space["network_interfaces"].append({
                    "name": interface,
                    "addresses": [addr.address for addr in addrs]
                })
        except ImportError:
            pass
            
        return space
    
    def _init_time_subsystem(self) -> Dict[str, Any]:
        """Initialize TIME dimension - Process Scheduling"""
        return {
            "scheduler_type": "round_robin",
            "time_slice_ms": 100,
            "system_uptime": 0.0,
            "process_count": 0,
            "scheduling_enabled": True
        }
    
    def _init_event_subsystem(self) -> Dict[str, Any]:
        """Initialize EVENT dimension - System Event Handling"""
        return {
            "interrupt_handlers": {},
            "event_queue_size": 0,
            "last_event": "system_boot",
            "event_processing": True
        }
    
    def _init_psychology_subsystem(self) -> Dict[str, Any]:
        """Initialize PSYCHOLOGY dimension - System Behavior & Process Management"""
        return {
            "system_mood": "booting",
            "load_average": 0.0,
            "memory_pressure": False,
            "adaptive_behavior": True,
            "process_management": "active"
        }
    
    def _init_pixel_subsystem(self) -> Dict[str, Any]:
        """Initialize PIXEL dimension - Canvas Display System"""
        pixel_system = {
            "display_enabled": DISPLAY_AVAILABLE,
            "canvas_width": self.canvas_width,
            "canvas_height": self.canvas_height,
            "color_depth": 24,
            "refresh_rate": 60,
            "windows": [],
            "desktop_color": "#2E3440"
        }
        
        if DISPLAY_AVAILABLE:
            self._init_canvas_display()
            
        return pixel_system
    
    def _init_prompt_subsystem(self) -> Dict[str, Any]:
        """Initialize PROMPT dimension - User Interface"""
        return {
            "shell_active": True,
            "gui_enabled": DISPLAY_AVAILABLE,
            "command_history": [],
            "user_session": "microos_user",
            "interface_mode": "graphical" if DISPLAY_AVAILABLE else "text"
        }
    
    def _init_script_subsystem(self) -> Dict[str, Any]:
        """Initialize SCRIPT dimension - Service Orchestration"""
        return {
            "services_running": [],
            "startup_scripts": ["init_display", "start_scheduler", "enable_shell"],
            "system_services": {
                "display_manager": "active" if DISPLAY_AVAILABLE else "disabled",
                "process_scheduler": "active",
                "event_handler": "active",
                "shell": "active"
            }
        }
    
    def _init_canvas_display(self):
        """Initialize canvas-based display system"""
        if not DISPLAY_AVAILABLE:
            return
            
        self.root_window = tk.Tk()
        self.root_window.title("STEPPPS MicroOS")
        self.root_window.geometry(f"{self.canvas_width}x{self.canvas_height + 50}")
        self.root_window.configure(bg="#2E3440")
        
        # Create main canvas
        self.canvas = tk.Canvas(
            self.root_window,
            width=self.canvas_width,
            height=self.canvas_height,
            bg="#2E3440",
            highlightthickness=0
        )
        self.canvas.pack(pady=10)
        
        # Status bar
        self.status_bar = tk.Label(
            self.root_window,
            text="STEPPPS MicroOS - Booting...",
            bg="#3B4252",
            fg="#ECEFF4",
            font=("Consolas", 10)
        )
        self.status_bar.pack(fill=tk.X)
        
        # Bind events
        self.canvas.bind("<Button-1>", self._handle_canvas_click)
        self.canvas.bind("<Key>", self._handle_canvas_key)
        self.canvas.focus_set()
        
        # Draw initial desktop
        self._draw_desktop()
    
    def _draw_desktop(self):
        """Draw the desktop environment on canvas"""
        if not self.canvas:
            return
            
        # Clear canvas
        self.canvas.delete("all")
        
        # Draw desktop background
        self.canvas.create_rectangle(
            0, 0, self.canvas_width, self.canvas_height,
            fill="#2E3440", outline=""
        )
        
        # Draw system info window
        info_x, info_y = 50, 50
        info_width, info_height = 300, 200
        
        self.canvas.create_rectangle(
            info_x, info_y, info_x + info_width, info_y + info_height,
            fill="#3B4252", outline="#5E81AC", width=2
        )
        
        # Title bar
        self.canvas.create_rectangle(
            info_x, info_y, info_x + info_width, info_y + 25,
            fill="#5E81AC", outline=""
        )
        self.canvas.create_text(
            info_x + info_width//2, info_y + 12,
            text="System Information", fill="white", font=("Consolas", 10, "bold")
        )
        
        # System info text
        info_text = [
            f"STEPPPS MicroOS v1.0",
            f"Device: {self.device_capability.value.upper()}",
            f"CPU Cores: {self.space['cpu_cores']}",
            f"Memory: {self.space['memory_mb']} MB",
            f"Uptime: {time.time() - self.boot_time:.1f}s",
            f"Processes: {len(self.processes)}"
        ]
        
        for i, line in enumerate(info_text):
            self.canvas.create_text(
                info_x + 10, info_y + 40 + i * 20,
                text=line, fill="#ECEFF4", font=("Consolas", 9), anchor="w"
            )
        
        # Draw process windows
        self._draw_process_windows()
        
        # Update status bar
        if hasattr(self, 'status_bar'):
            self.status_bar.config(
                text=f"STEPPPS MicroOS - {len(self.processes)} processes - {self.psychology['system_mood']}"
            )
    
    def _draw_process_windows(self):
        """Draw windows for running processes"""
        window_x, window_y = 400, 50
        window_width, window_height = 200, 150
        
        for i, (pid, process) in enumerate(self.processes.items()):
            if process.canvas_region:
                # Use custom region if specified
                x = process.canvas_region['x']
                y = process.canvas_region['y']
                w = process.canvas_region['width']
                h = process.canvas_region['height']
            else:
                # Default positioning
                x = window_x + (i % 2) * 210
                y = window_y + (i // 2) * 160
                w, h = window_width, window_height
            
            # Draw window
            self.canvas.create_rectangle(
                x, y, x + w, y + h,
                fill="#434C5E", outline="#88C0D0", width=1
            )
            
            # Title bar
            self.canvas.create_rectangle(
                x, y, x + w, y + 20,
                fill="#88C0D0", outline=""
            )
            self.canvas.create_text(
                x + w//2, y + 10,
                text=f"{process.name} (PID: {pid})", 
                fill="black", font=("Consolas", 8, "bold")
            )
            
            # Process info
            info_lines = [
                f"State: {process.state.value}",
                f"Priority: {process.priority}",
                f"Memory: {process.memory_usage} KB",
                f"CPU Time: {process.cpu_time:.2f}s"
            ]
            
            for j, line in enumerate(info_lines):
                self.canvas.create_text(
                    x + 5, y + 30 + j * 15,
                    text=line, fill="#ECEFF4", font=("Consolas", 8), anchor="w"
                )
    
    def _handle_canvas_click(self, event):
        """Handle mouse clicks on canvas"""
        x, y = event.x, event.y
        print(f"Canvas click at ({x}, {y})")
        
        # Add click event to event queue
        self.event_queue.put({
            "type": "mouse_click",
            "x": x, "y": y,
            "timestamp": time.time()
        })
    
    def _handle_canvas_key(self, event):
        """Handle keyboard input"""
        key = event.keysym
        print(f"Key pressed: {key}")
        
        # Add key event to event queue
        self.event_queue.put({
            "type": "key_press",
            "key": key,
            "timestamp": time.time()
        })
    
    def create_process(self, name: str, priority: int = 5, 
                      canvas_region: Optional[Dict[str, int]] = None) -> int:
        """Create a new STEPPPS process"""
        pid = self.next_pid
        self.next_pid += 1
        
        process = STEPPPSProcess(
            pid=pid,
            name=name,
            priority=priority,
            canvas_region=canvas_region,
            steppps_data={
                "space": {"allocated_memory": 1024},
                "time": {"created": time.time()},
                "event": {"event_count": 0},
                "psychology": {"mood": "active"},
                "pixel": {"canvas_region": canvas_region},
                "prompt": {"interactive": True},
                "script": {"main_function": "run"}
            }
        )
        
        self.processes[pid] = process
        self.time["process_count"] = len(self.processes)
        
        print(f"📦 Process created: {name} (PID: {pid})")
        return pid
    
    def terminate_process(self, pid: int) -> bool:
        """Terminate a process"""
        if pid in self.processes:
            process = self.processes[pid]
            process.state = ProcessState.TERMINATED
            del self.processes[pid]
            self.time["process_count"] = len(self.processes)
            print(f"🗑️  Process terminated: {process.name} (PID: {pid})")
            return True
        return False
    
    def schedule_processes(self):
        """Simple round-robin process scheduler"""
        if not self.time["scheduling_enabled"]:
            return
            
        for process in self.processes.values():
            if process.state == ProcessState.READY:
                process.state = ProcessState.RUNNING
                process.cpu_time += self.time["time_slice_ms"] / 1000.0
                
                # Simulate process work
                time.sleep(0.01)  # 10ms simulation
                
                process.state = ProcessState.READY
    
    def process_events(self):
        """Process system events from event queue"""
        while not self.event_queue.empty():
            try:
                event = self.event_queue.get_nowait()
                self.event["last_event"] = event["type"]
                self.event["event_queue_size"] = self.event_queue.qsize()
                
                # Handle different event types
                if event["type"] == "mouse_click":
                    self._handle_mouse_event(event)
                elif event["type"] == "key_press":
                    self._handle_key_event(event)
                    
            except queue.Empty:
                break
    
    def _handle_mouse_event(self, event):
        """Handle mouse click events"""
        x, y = event["x"], event["y"]
        
        # Check if click is on a process window
        for pid, process in self.processes.items():
            if process.canvas_region:
                region = process.canvas_region
                if (region["x"] <= x <= region["x"] + region["width"] and
                    region["y"] <= y <= region["y"] + region["height"]):
                    print(f"🖱️  Clicked on process: {process.name}")
                    break
    
    def _handle_key_event(self, event):
        """Handle keyboard events"""
        key = event["key"]
        
        # Simple command handling
        if key == "Return":
            self.create_process(f"Task_{len(self.processes)+1}")
        elif key == "Delete":
            if self.processes:
                pid = max(self.processes.keys())
                self.terminate_process(pid)
    
    def update_display(self):
        """Update the canvas display"""
        if self.canvas:
            self._draw_desktop()
            self.root_window.update()
    
    def run_system_loop(self):
        """Main system loop"""
        print("🚀 Starting STEPPPS MicroOS...")
        self.running = True
        self.psychology["system_mood"] = "running"
        
        # Create initial system processes
        self.create_process("System Monitor", priority=1)
        self.create_process("Shell", priority=2)
        
        loop_count = 0
        while self.running:
            try:
                # Update system uptime
                self.time["system_uptime"] = time.time() - self.boot_time
                
                # Process scheduling
                self.schedule_processes()
                
                # Event processing
                self.process_events()
                
                # Update display every 10 loops (~1 second)
                if loop_count % 10 == 0:
                    self.update_display()
                
                # System health monitoring
                self.psychology["load_average"] = len(self.processes) / self.space["cpu_cores"]
                
                time.sleep(0.1)  # 100ms system tick
                loop_count += 1
                
            except KeyboardInterrupt:
                print("\n🛑 Shutdown signal received...")
                self.shutdown()
                break
            except Exception as e:
                print(f"❌ System error: {e}")
                continue
    
    def shutdown(self):
        """Graceful system shutdown"""
        print("🔄 Shutting down STEPPPS MicroOS...")
        self.running = False
        self.psychology["system_mood"] = "shutting_down"
        
        # Terminate all processes
        for pid in list(self.processes.keys()):
            self.terminate_process(pid)
        
        # Close display
        if self.root_window:
            self.root_window.quit()
            self.root_window.destroy()
        
        print("✅ STEPPPS MicroOS shutdown complete")
    
    def get_system_info(self) -> Dict[str, Any]:
        """Get comprehensive system information"""
        return {
            "steppps_microos_version": "1.0",
            "device_capability": self.device_capability.value,
            "uptime": time.time() - self.boot_time,
            "space": self.space,
            "time": self.time,
            "event": self.event,
            "psychology": self.psychology,
            "pixel": self.pixel,
            "prompt": self.prompt,
            "script": self.script,
            "processes": {pid: {
                "name": p.name,
                "state": p.state.value,
                "priority": p.priority,
                "memory_usage": p.memory_usage,
                "cpu_time": p.cpu_time
            } for pid, p in self.processes.items()}
        }

def main():
    """Main entry point for STEPPPS MicroOS"""
    print("🌟 STEPPPS MicroOS - Universal Operating System")
    print("=" * 50)
    
    try:
        # Create and start the micro OS
        micro_os = STEPPPSMicroOS()
        micro_os.run_system_loop()
    except Exception as e:
        print(f"💥 Fatal system error: {e}")
        import traceback
        traceback.print_exc()
        if 'micro_os' in locals():
            micro_os.shutdown()

if __name__ == "__main__":
    main()
