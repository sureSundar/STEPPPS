#!/usr/bin/env python3
"""
STEPPPS MicroOS with Docker Linux Bootstrap
==========================================
MicroOS that bootstraps Docker containers with mini canvas display.
Canvas shows containerized Linux terminal output.
"""

import os
import sys
import time
import json
import threading
import subprocess
import queue
from typing import Dict, List, Any, Optional

try:
    import tkinter as tk
    from tkinter import ttk, scrolledtext
    DISPLAY_AVAILABLE = True
except ImportError:
    DISPLAY_AVAILABLE = False

class DockerLinuxContainer:
    """Represents a Docker Linux container"""
    def __init__(self, name: str, image: str, container_id: str = None):
        self.name = name
        self.image = image
        self.container_id = container_id
        self.status = "created"
        self.output_buffer = []
        self.process = None

class STEPPPSDockerMicroOS:
    """STEPPPS MicroOS with Docker Linux bootstrap capability"""
    
    def __init__(self):
        self.boot_time = time.time()
        self.running = False
        
        # Mini canvas setup - Initialize before STEPPPS dimensions
        self.canvas_width = 600
        self.canvas_height = 400
        self.canvas = None
        self.root_window = None
        
        # Docker containers
        self.containers: Dict[str, DockerLinuxContainer] = {}
        self.output_queue = queue.Queue()
        
        # STEPPPS Dimensions for Docker orchestration
        self.space = self._init_space_docker()
        self.time = self._init_time_docker()
        self.event = self._init_event_docker()
        self.psychology = self._init_psychology_docker()
        self.pixel = self._init_pixel_docker()
        self.prompt = self._init_prompt_docker()
        self.script = self._init_script_docker()
        
        print("🐳 STEPPPS Docker MicroOS v1.0")
        
    def _init_space_docker(self) -> Dict[str, Any]:
        """SPACE: Docker environment detection"""
        docker_available = self._check_docker()
        return {
            "docker_available": docker_available,
            "container_limit": 5,
            "memory_per_container": "256m",
            "available_images": self._get_available_images() if docker_available else []
        }
    
    def _init_time_docker(self) -> Dict[str, Any]:
        """TIME: Container lifecycle management"""
        return {
            "container_boot_time": 0.0,
            "uptime_tracking": True,
            "scheduled_containers": []
        }
    
    def _init_event_docker(self) -> Dict[str, Any]:
        """EVENT: Container events and monitoring"""
        return {
            "container_events": [],
            "monitoring_active": True,
            "event_handlers": {}
        }
    
    def _init_psychology_docker(self) -> Dict[str, Any]:
        """PSYCHOLOGY: Container selection and behavior"""
        return {
            "preferred_distro": "alpine",
            "auto_restart": True,
            "load_balancing": False
        }
    
    def _init_pixel_docker(self) -> Dict[str, Any]:
        """PIXEL: Mini canvas display system"""
        pixel_system = {
            "display_enabled": DISPLAY_AVAILABLE,
            "canvas_width": self.canvas_width,
            "canvas_height": self.canvas_height,
            "terminal_windows": {},
            "background_color": "#1E1E1E"
        }
        
        if DISPLAY_AVAILABLE:
            self._init_mini_canvas()
            
        return pixel_system
    
    def _init_prompt_docker(self) -> Dict[str, Any]:
        """PROMPT: Container interaction interface"""
        return {
            "active_container": None,
            "command_history": [],
            "interactive_mode": True
        }
    
    def _init_script_docker(self) -> Dict[str, Any]:
        """SCRIPT: Docker orchestration engine"""
        return {
            "orchestration_active": True,
            "container_recipes": {
                "alpine": "alpine:latest",
                "ubuntu": "ubuntu:20.04",
                "debian": "debian:latest",
                "arch": "archlinux:latest"
            },
            "startup_containers": ["alpine"]
        }
    
    def _check_docker(self) -> bool:
        """Check if Docker is available"""
        try:
            result = subprocess.run(['docker', '--version'], 
                                  capture_output=True, text=True)
            return result.returncode == 0
        except FileNotFoundError:
            return False
    
    def _get_available_images(self) -> List[str]:
        """Get available Docker images"""
        try:
            result = subprocess.run(['docker', 'images', '--format', '{{.Repository}}:{{.Tag}}'],
                                  capture_output=True, text=True)
            if result.returncode == 0:
                return result.stdout.strip().split('\n')[:10]  # First 10 images
        except:
            pass
        return []
    
    def _init_mini_canvas(self):
        """Initialize mini canvas display"""
        if not DISPLAY_AVAILABLE:
            return
            
        self.root_window = tk.Tk()
        self.root_window.title("STEPPPS Docker MicroOS")
        self.root_window.geometry(f"{self.canvas_width}x{self.canvas_height + 100}")
        self.root_window.configure(bg="#1E1E1E")
        
        # Main canvas
        self.canvas = tk.Canvas(
            self.root_window,
            width=self.canvas_width,
            height=self.canvas_height,
            bg="#1E1E1E",
            highlightthickness=0
        )
        self.canvas.pack(pady=5)
        
        # Control panel
        control_frame = tk.Frame(self.root_window, bg="#2D2D2D")
        control_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Buttons
        tk.Button(control_frame, text="Alpine", 
                 command=lambda: self.bootstrap_container("alpine"),
                 bg="#4CAF50", fg="white", font=("Consolas", 8)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Ubuntu", 
                 command=lambda: self.bootstrap_container("ubuntu"),
                 bg="#FF9800", fg="white", font=("Consolas", 8)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Debian", 
                 command=lambda: self.bootstrap_container("debian"),
                 bg="#2196F3", fg="white", font=("Consolas", 8)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Stop All", 
                 command=self.stop_all_containers,
                 bg="#F44336", fg="white", font=("Consolas", 8)).pack(side=tk.RIGHT, padx=2)
        
        # Status bar
        self.status_bar = tk.Label(
            self.root_window,
            text="STEPPPS Docker MicroOS - Ready",
            bg="#2D2D2D", fg="#FFFFFF",
            font=("Consolas", 9)
        )
        self.status_bar.pack(fill=tk.X)
        
        # Bind events
        self.canvas.bind("<Button-1>", self._handle_canvas_click)
        self.canvas.focus_set()
        
        # Draw initial state
        self._draw_mini_canvas()
    
    def _draw_mini_canvas(self):
        """Draw mini canvas with container terminals"""
        if not self.canvas:
            return
            
        # Clear canvas
        self.canvas.delete("all")
        
        # Background
        self.canvas.create_rectangle(
            0, 0, self.canvas_width, self.canvas_height,
            fill="#1E1E1E", outline=""
        )
        
        # Title
        self.canvas.create_text(
            self.canvas_width // 2, 20,
            text="STEPPPS Docker MicroOS", 
            fill="#00FF00", font=("Consolas", 12, "bold")
        )
        
        # Container windows
        if not self.containers:
            # No containers message
            self.canvas.create_text(
                self.canvas_width // 2, self.canvas_height // 2,
                text="No containers running\nClick buttons below to bootstrap Linux",
                fill="#888888", font=("Consolas", 10), justify=tk.CENTER
            )
        else:
            self._draw_container_windows()
        
        # System info
        info_text = f"Containers: {len(self.containers)} | Docker: {'✓' if self.space['docker_available'] else '✗'}"
        self.canvas.create_text(
            10, self.canvas_height - 10,
            text=info_text, fill="#CCCCCC", font=("Consolas", 8), anchor="w"
        )
    
    def _draw_container_windows(self):
        """Draw terminal windows for running containers"""
        container_count = len(self.containers)
        if container_count == 0:
            return
        
        # Calculate window layout
        cols = min(2, container_count)
        rows = (container_count + cols - 1) // cols
        
        window_width = (self.canvas_width - 40) // cols
        window_height = (self.canvas_height - 80) // max(rows, 1)
        
        for i, (name, container) in enumerate(self.containers.items()):
            col = i % cols
            row = i // cols
            
            x = 20 + col * window_width
            y = 40 + row * window_height
            
            # Container window
            self.canvas.create_rectangle(
                x, y, x + window_width - 10, y + window_height - 10,
                fill="#2D2D2D", outline="#555555", width=1
            )
            
            # Title bar
            title_color = "#4CAF50" if container.status == "running" else "#F44336"
            self.canvas.create_rectangle(
                x, y, x + window_width - 10, y + 20,
                fill=title_color, outline=""
            )
            
            # Container name and status
            self.canvas.create_text(
                x + 5, y + 10,
                text=f"{name} ({container.image})", 
                fill="white", font=("Consolas", 8, "bold"), anchor="w"
            )
            
            # Terminal output
            output_y = y + 25
            output_lines = container.output_buffer[-8:]  # Last 8 lines
            
            for j, line in enumerate(output_lines):
                self.canvas.create_text(
                    x + 5, output_y + j * 12,
                    text=line[:40] + "..." if len(line) > 40 else line,
                    fill="#00FF00", font=("Consolas", 7), anchor="w"
                )
    
    def _handle_canvas_click(self, event):
        """Handle canvas clicks"""
        # Simple interaction - could expand to select containers
        print(f"Canvas clicked at ({event.x}, {event.y})")
    
    def bootstrap_container(self, distro: str):
        """SCRIPT: Bootstrap a Linux container with living_bootstrap.py"""
        if not self.space["docker_available"]:
            print("❌ Docker not available - simulating container")
            return self._simulate_container_with_bootstrap(distro)
        
        if len(self.containers) >= self.space["container_limit"]:
            print("❌ Container limit reached")
            return False
        
        image = self.script["container_recipes"].get(distro, f"{distro}:latest")
        container_name = f"steppps_{distro}_{int(time.time())}"
        
        print(f"🚀 Bootstrapping {distro} container with living_bootstrap.py...")
        
        try:
            # SPACE: Pull image if needed
            subprocess.run(['sudo', 'docker', 'pull', image], 
                          capture_output=True, check=False)
            
            # SCRIPT: Copy living_bootstrap.py into container and run
            # First create container
            cmd_create = [
                'sudo', 'docker', 'run', '-d', '-it',
                '--name', container_name,
                '--memory', self.space["memory_per_container"],
                image, '/bin/sh'
            ]
            
            result_create = subprocess.run(cmd_create, capture_output=True, text=True)
            if result_create.returncode != 0:
                raise Exception(f"Container creation failed: {result_create.stderr}")
            
            container_id = result_create.stdout.strip()
            
            # Install Python if needed
            subprocess.run([
                'sudo', 'docker', 'exec', container_id,
                '/bin/sh', '-c', 
                'if ! command -v python3 >/dev/null 2>&1; then apk add python3 py3-pip || apt-get update && apt-get install -y python3 python3-pip || yum install -y python3 python3-pip; fi'
            ], capture_output=True)
            
            # Copy container bootstrap script
            subprocess.run([
                'sudo', 'docker', 'cp', 
                'steppps_container_bootstrap.py', 
                f'{container_id}:/steppps_bootstrap.py'
            ], capture_output=True)
            
            # Start STEPPPS bootstrap in background
            subprocess.run([
                'sudo', 'docker', 'exec', '-d', container_id,
                'python3', '/steppps_bootstrap.py'
            ], capture_output=True)
            
            # Create container object
            container = DockerLinuxContainer(
                name=container_name,
                image=image,
                container_id=container_id
            )
            container.status = "running"
            container.output_buffer = [
                f"Container {container_name} started",
                f"Image: {image}",
                f"ID: {container_id[:12]}...",
                "Loading living_bootstrap.py...",
                "STEPPPS framework initializing..."
            ]
            
            self.containers[container_name] = container
            
            # TIME: Record boot time
            self.time["container_boot_time"] = time.time() - self.boot_time
            
            # PSYCHOLOGY: Update preferences
            self.psychology["preferred_distro"] = distro
            
            print(f"✅ Container {container_name} bootstrapped with STEPPPS")
            
            # Start monitoring thread
            threading.Thread(
                target=self._monitor_container,
                args=(container,),
                daemon=True
            ).start()
            
            # Update display
            if self.canvas:
                self._draw_mini_canvas()
                self.status_bar.config(text=f"STEPPPS {distro} running")
            
            return True
                
        except Exception as e:
            print(f"❌ Bootstrap error: {e}")
            return self._simulate_container_with_bootstrap(distro)
    
    def _simulate_container_with_bootstrap(self, distro: str):
        """Simulate container with living_bootstrap.py when Docker unavailable"""
        container_name = f"sim_{distro}_{int(time.time())}"
        
        # Create simulated container
        container = DockerLinuxContainer(
            name=container_name,
            image=f"{distro}:simulated",
            container_id=f"sim_{container_name}"
        )
        container.status = "simulated"
        container.output_buffer = [
            f"🔧 Simulated {distro} Linux + STEPPPS",
            f"Name: {container_name}",
            "Status: Running (simulation)",
            "Loading living_bootstrap.py...",
            "🚀 STEPPPS Living Bootstrap v2.0",
            "📡 Initializing 7-dimensional framework",
            "🌌 SPACE: Container environment detected",
            "⏰ TIME: Bootstrap sequence started",
            "🎯 EVENT: System awakening initiated",
            "🧠 PSYCHOLOGY: AI consciousness emerging",
            "🎨 PIXEL: Virtual display initialized",
            "💬 PROMPT: LLM interface ready",
            "📜 SCRIPT: Orchestration engine active",
            "✨ STEPPPS framework fully operational"
        ]
        
        self.containers[container_name] = container
        
        print(f"✅ Simulated {distro} + STEPPPS container created")
        
        # Start simulated STEPPPS evolution
        threading.Thread(
            target=self._simulate_steppps_evolution,
            args=(container,),
            daemon=True
        ).start()
        
        if self.canvas:
            self._draw_mini_canvas()
            self.status_bar.config(text=f"STEPPPS {distro} simulation running")
        
        return True
    
    def _simulate_steppps_evolution(self, container: DockerLinuxContainer):
        """Simulate STEPPPS evolution in container"""
        evolution_steps = [
            "🔍 Discovering system capabilities...",
            "🌐 Network services initializing...",
            "🔧 Hardware abstraction layer active",
            "📊 System metrics collection started",
            "🤖 AI callback execution in progress",
            "🔄 Self-optimization cycle initiated",
            "📡 Inter-STEPPPS communication ready",
            "🎯 Mission objectives loaded",
            "⚡ Real-time adaptation enabled",
            "🌟 Living system fully awakened"
        ]
        
        for i, step in enumerate(evolution_steps):
            time.sleep(2)  # Simulate processing time
            container.output_buffer.append(step)
            
            # Keep buffer manageable
            if len(container.output_buffer) > 20:
                container.output_buffer.pop(0)
            
            # Update display every few steps
            if i % 2 == 0 and self.canvas:
                self.root_window.after(0, self._draw_mini_canvas)
    
    def _monitor_container(self, container: DockerLinuxContainer):
        """Monitor container output"""
        try:
            # Get container logs with sudo
            process = subprocess.Popen([
                'sudo', 'docker', 'logs', '-f', container.container_id
            ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, 
               text=True, bufsize=1, universal_newlines=True)
            
            container.process = process
            
            for line in iter(process.stdout.readline, ''):
                if line:
                    container.output_buffer.append(line.strip())
                    # Keep only last 20 lines
                    if len(container.output_buffer) > 20:
                        container.output_buffer.pop(0)
                    
                    # Update display periodically
                    if len(container.output_buffer) % 5 == 0 and self.canvas:
                        self.root_window.after(0, self._draw_mini_canvas)
                        
        except Exception as e:
            print(f"Monitor error for {container.name}: {e}")
    
    def stop_all_containers(self):
        """Stop all running containers"""
        print("🛑 Stopping all containers...")
        
        for name, container in list(self.containers.items()):
            try:
                subprocess.run(['docker', 'stop', container.container_id],
                              capture_output=True)
                subprocess.run(['docker', 'rm', container.container_id],
                              capture_output=True)
                
                if container.process:
                    container.process.terminate()
                
                print(f"✅ Stopped {name}")
            except Exception as e:
                print(f"❌ Error stopping {name}: {e}")
        
        self.containers.clear()
        
        if self.canvas:
            self._draw_mini_canvas()
            self.status_bar.config(text="All containers stopped")
    
    def run_microos(self):
        """Run the STEPPPS Docker MicroOS"""
        print("🚀 Starting STEPPPS Docker MicroOS...")
        self.running = True
        
        # Auto-bootstrap initial container
        if self.space["docker_available"]:
            initial_distro = self.script["startup_containers"][0]
            self.bootstrap_container(initial_distro)
        
        if DISPLAY_AVAILABLE and self.root_window:
            # GUI mode
            try:
                self.root_window.mainloop()
            except KeyboardInterrupt:
                self.shutdown()
        else:
            # CLI mode
            try:
                while self.running:
                    time.sleep(1)
            except KeyboardInterrupt:
                self.shutdown()
    
    def shutdown(self):
        """Shutdown MicroOS"""
        print("🔄 Shutting down STEPPPS Docker MicroOS...")
        self.running = False
        self.stop_all_containers()
        
        if self.root_window:
            self.root_window.quit()
            self.root_window.destroy()
        
        print("✅ Shutdown complete")

def main():
    """Main entry point"""
    print("🐳 STEPPPS Docker MicroOS")
    print("=" * 30)
    
    microos = STEPPPSDockerMicroOS()
    
    try:
        microos.run_microos()
    except Exception as e:
        print(f"💥 Fatal error: {e}")
        microos.shutdown()

if __name__ == "__main__":
    main()
