#!/usr/bin/env python3
"""
STEPPPS Swarm Orchestrator - Advanced Container Management
=========================================================
Multi-container STEPPPS orchestration with inter-container communication
and real-time state visualization.
"""

import os
import sys
import time
import json
import threading
import subprocess
import queue
import socket
import http.server
import socketserver
from typing import Dict, List, Any, Optional
from dataclasses import dataclass, asdict

try:
    import tkinter as tk
    from tkinter import ttk, scrolledtext
    DISPLAY_AVAILABLE = True
except ImportError:
    DISPLAY_AVAILABLE = False

@dataclass
class STEPPPSNode:
    """Represents a STEPPPS node in the swarm"""
    container_id: str
    name: str
    image: str
    status: str
    steppps_state: Dict[str, Any]
    last_heartbeat: float
    communication_port: int
    output_buffer: List[str]

class STEPPPSSwarmOrchestrator:
    """Advanced STEPPPS container swarm orchestrator"""
    
    def __init__(self):
        self.boot_time = time.time()
        self.running = False
        
        # Swarm configuration
        self.max_nodes = 10
        self.base_port = 8000
        self.heartbeat_interval = 5.0
        
        # STEPPPS nodes in swarm
        self.nodes: Dict[str, STEPPPSNode] = {}
        self.communication_queue = queue.Queue()
        
        # Canvas setup for advanced visualization
        self.canvas_width = 1000
        self.canvas_height = 700
        self.canvas = None
        self.root_window = None
        
        # Swarm orchestration state
        self.orchestration_state = {
            "space": self._init_swarm_space(),
            "time": self._init_swarm_time(),
            "event": self._init_swarm_event(),
            "psychology": self._init_swarm_psychology(),
            "pixel": self._init_swarm_pixel(),
            "prompt": self._init_swarm_prompt(),
            "script": self._init_swarm_script()
        }
        
        print("🌌 STEPPPS Swarm Orchestrator v1.0")
        print("🚀 Advanced multi-container STEPPPS management")
        
    def _init_swarm_space(self) -> Dict[str, Any]:
        """SPACE: Swarm topology and resources"""
        return {
            "docker_available": self._check_docker(),
            "max_nodes": self.max_nodes,
            "active_nodes": 0,
            "resource_pool": {
                "cpu_cores": os.cpu_count() or 4,
                "memory_gb": 8,  # Estimated
                "storage_gb": 100
            },
            "network_topology": "mesh"
        }
    
    def _init_swarm_time(self) -> Dict[str, Any]:
        """TIME: Swarm temporal coordination"""
        return {
            "swarm_boot_time": self.boot_time,
            "global_clock": time.time(),
            "sync_interval": 1.0,
            "orchestration_cycles": 0
        }
    
    def _init_swarm_event(self) -> Dict[str, Any]:
        """EVENT: Swarm-wide event coordination"""
        return {
            "global_events": [],
            "node_events": {},
            "event_propagation": True,
            "consensus_required": False
        }
    
    def _init_swarm_psychology(self) -> Dict[str, Any]:
        """PSYCHOLOGY: Collective swarm intelligence"""
        return {
            "swarm_mood": "initializing",
            "collective_confidence": 0.5,
            "learning_mode": "distributed",
            "consensus_threshold": 0.7
        }
    
    def _init_swarm_pixel(self) -> Dict[str, Any]:
        """PIXEL: Advanced swarm visualization"""
        pixel_system = {
            "display_enabled": DISPLAY_AVAILABLE,
            "canvas_width": self.canvas_width,
            "canvas_height": self.canvas_height,
            "visualization_mode": "network_graph",
            "node_colors": {},
            "connection_lines": []
        }
        
        if DISPLAY_AVAILABLE:
            self._init_swarm_canvas()
            
        return pixel_system
    
    def _init_swarm_prompt(self) -> Dict[str, Any]:
        """PROMPT: Swarm-wide AI coordination"""
        return {
            "distributed_prompting": True,
            "consensus_prompts": [],
            "node_responses": {},
            "collective_reasoning": True
        }
    
    def _init_swarm_script(self) -> Dict[str, Any]:
        """SCRIPT: Advanced orchestration engine"""
        return {
            "orchestration_mode": "autonomous",
            "deployment_strategies": [
                "round_robin",
                "load_balanced",
                "capability_aware",
                "geographic_distributed"
            ],
            "active_strategy": "capability_aware",
            "auto_scaling": True,
            "fault_tolerance": True
        }
    
    def _check_docker(self) -> bool:
        """Check Docker availability"""
        try:
            result = subprocess.run(['docker', '--version'], 
                                  capture_output=True, text=True)
            return result.returncode == 0
        except FileNotFoundError:
            return False
    
    def _init_swarm_canvas(self):
        """Initialize advanced swarm visualization canvas"""
        if not DISPLAY_AVAILABLE:
            return
            
        self.root_window = tk.Tk()
        self.root_window.title("STEPPPS Swarm Orchestrator")
        self.root_window.geometry(f"{self.canvas_width}x{self.canvas_height + 150}")
        self.root_window.configure(bg="#0A0A0A")
        
        # Main canvas for network visualization
        self.canvas = tk.Canvas(
            self.root_window,
            width=self.canvas_width,
            height=self.canvas_height,
            bg="#0A0A0A",
            highlightthickness=0
        )
        self.canvas.pack(pady=5)
        
        # Control panel
        control_frame = tk.Frame(self.root_window, bg="#1A1A1A")
        control_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Node deployment buttons
        tk.Button(control_frame, text="Deploy Alpine Swarm", 
                 command=lambda: self.deploy_swarm_nodes("alpine", 3),
                 bg="#4CAF50", fg="white", font=("Consolas", 9)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Deploy Ubuntu Swarm", 
                 command=lambda: self.deploy_swarm_nodes("ubuntu", 2),
                 bg="#FF9800", fg="white", font=("Consolas", 9)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Mixed Deployment", 
                 command=self.deploy_mixed_swarm,
                 bg="#9C27B0", fg="white", font=("Consolas", 9)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Scale Up", 
                 command=self.scale_up_swarm,
                 bg="#2196F3", fg="white", font=("Consolas", 9)).pack(side=tk.LEFT, padx=2)
        
        tk.Button(control_frame, text="Terminate Swarm", 
                 command=self.terminate_swarm,
                 bg="#F44336", fg="white", font=("Consolas", 9)).pack(side=tk.RIGHT, padx=2)
        
        # Status panel
        status_frame = tk.Frame(self.root_window, bg="#1A1A1A")
        status_frame.pack(fill=tk.X, padx=5, pady=2)
        
        self.status_label = tk.Label(
            status_frame,
            text="STEPPPS Swarm Orchestrator - Ready",
            bg="#1A1A1A", fg="#00FF00",
            font=("Consolas", 10, "bold")
        )
        self.status_label.pack(side=tk.LEFT)
        
        self.node_count_label = tk.Label(
            status_frame,
            text="Nodes: 0",
            bg="#1A1A1A", fg="#FFFFFF",
            font=("Consolas", 10)
        )
        self.node_count_label.pack(side=tk.RIGHT)
        
        # Bind events
        self.canvas.bind("<Button-1>", self._handle_canvas_click)
        self.canvas.focus_set()
        
        # Start visualization update loop
        self._update_swarm_visualization()
    
    def _update_swarm_visualization(self):
        """Update swarm network visualization"""
        if not self.canvas:
            return
            
        # Clear canvas
        self.canvas.delete("all")
        
        # Background grid
        self._draw_background_grid()
        
        # Title
        self.canvas.create_text(
            self.canvas_width // 2, 30,
            text="STEPPPS Swarm Network", 
            fill="#00FFFF", font=("Consolas", 16, "bold")
        )
        
        if not self.nodes:
            # No nodes message
            self.canvas.create_text(
                self.canvas_width // 2, self.canvas_height // 2,
                text="No STEPPPS nodes deployed\nUse buttons below to deploy swarm",
                fill="#888888", font=("Consolas", 12), justify=tk.CENTER
            )
        else:
            self._draw_swarm_network()
        
        # Update status
        self.node_count_label.config(text=f"Nodes: {len(self.nodes)}")
        
        # Schedule next update
        if self.running:
            self.root_window.after(1000, self._update_swarm_visualization)
    
    def _draw_background_grid(self):
        """Draw background grid for network visualization"""
        grid_size = 50
        
        # Vertical lines
        for x in range(0, self.canvas_width, grid_size):
            self.canvas.create_line(
                x, 0, x, self.canvas_height,
                fill="#111111", width=1
            )
        
        # Horizontal lines
        for y in range(0, self.canvas_height, grid_size):
            self.canvas.create_line(
                0, y, self.canvas_width, y,
                fill="#111111", width=1
            )
    
    def _draw_swarm_network(self):
        """Draw STEPPPS swarm network topology"""
        node_positions = self._calculate_node_positions()
        
        # Draw connections between nodes
        self._draw_node_connections(node_positions)
        
        # Draw individual nodes
        for node_name, (x, y) in node_positions.items():
            node = self.nodes[node_name]
            self._draw_steppps_node(node, x, y)
    
    def _calculate_node_positions(self) -> Dict[str, tuple]:
        """Calculate optimal positions for nodes in network visualization"""
        positions = {}
        node_count = len(self.nodes)
        
        if node_count == 0:
            return positions
        
        # Circular layout for better visualization
        center_x = self.canvas_width // 2
        center_y = (self.canvas_height // 2) + 50
        radius = min(250, (self.canvas_width - 200) // 2)
        
        for i, node_name in enumerate(self.nodes.keys()):
            angle = 2 * 3.14159 * i / node_count
            x = center_x + radius * cos(angle)
            y = center_y + radius * sin(angle)
            positions[node_name] = (x, y)
        
        return positions
    
    def _draw_node_connections(self, positions: Dict[str, tuple]):
        """Draw connections between STEPPPS nodes"""
        node_names = list(positions.keys())
        
        for i, node1 in enumerate(node_names):
            for node2 in node_names[i+1:]:
                x1, y1 = positions[node1]
                x2, y2 = positions[node2]
                
                # Connection strength based on communication
                connection_color = "#333333"
                line_width = 1
                
                # Highlight active connections
                if self._nodes_communicating(node1, node2):
                    connection_color = "#00FF00"
                    line_width = 2
                
                self.canvas.create_line(
                    x1, y1, x2, y2,
                    fill=connection_color, width=line_width
                )
    
    def _draw_steppps_node(self, node: STEPPPSNode, x: float, y: float):
        """Draw individual STEPPPS node with state visualization"""
        # Node circle
        node_color = self._get_node_color(node)
        node_size = 30
        
        self.canvas.create_oval(
            x - node_size, y - node_size,
            x + node_size, y + node_size,
            fill=node_color, outline="#FFFFFF", width=2
        )
        
        # Node label
        self.canvas.create_text(
            x, y - node_size - 15,
            text=node.name.split('_')[1],  # Show distro name
            fill="#FFFFFF", font=("Consolas", 8, "bold")
        )
        
        # Status indicator
        status_color = "#00FF00" if node.status == "running" else "#FF0000"
        self.canvas.create_oval(
            x + node_size - 5, y - node_size + 5,
            x + node_size + 5, y - node_size + 15,
            fill=status_color, outline=""
        )
        
        # STEPPPS state mini-visualization
        self._draw_node_steppps_state(node, x, y + node_size + 10)
    
    def _draw_node_steppps_state(self, node: STEPPPSNode, x: float, y: float):
        """Draw mini STEPPPS state visualization for node"""
        dimensions = ["S", "T", "E", "P", "P", "P", "S"]
        colors = ["#FF0000", "#FF8000", "#FFFF00", "#80FF00", 
                 "#00FF80", "#0080FF", "#8000FF"]
        
        for i, (dim, color) in enumerate(zip(dimensions, colors)):
            dim_x = x - 30 + i * 10
            dim_y = y
            
            # Small rectangle for each dimension
            self.canvas.create_rectangle(
                dim_x - 3, dim_y - 3,
                dim_x + 3, dim_y + 3,
                fill=color, outline=""
            )
    
    def _get_node_color(self, node: STEPPPSNode) -> str:
        """Get color for node based on its state"""
        if "alpine" in node.image:
            return "#4CAF50"
        elif "ubuntu" in node.image:
            return "#FF9800"
        elif "debian" in node.image:
            return "#2196F3"
        else:
            return "#9C27B0"
    
    def _nodes_communicating(self, node1: str, node2: str) -> bool:
        """Check if two nodes are actively communicating"""
        # Simplified communication check
        return time.time() % 10 < 2  # Simulate periodic communication
    
    def _handle_canvas_click(self, event):
        """Handle canvas clicks for node interaction"""
        print(f"Swarm canvas clicked at ({event.x}, {event.y})")
        # Could implement node selection and detailed view
    
    def deploy_swarm_nodes(self, distro: str, count: int):
        """Deploy multiple STEPPPS nodes of same distro"""
        print(f"🚀 Deploying {count} {distro} STEPPPS nodes...")
        
        for i in range(count):
            node_name = f"steppps_{distro}_{int(time.time())}_{i}"
            self._deploy_single_node(distro, node_name)
            time.sleep(1)  # Stagger deployments
        
        self.status_label.config(text=f"Deployed {count} {distro} nodes")
    
    def deploy_mixed_swarm(self):
        """Deploy mixed distro swarm"""
        print("🌈 Deploying mixed STEPPPS swarm...")
        
        distros = ["alpine", "ubuntu", "debian"]
        for distro in distros:
            self._deploy_single_node(distro, f"steppps_{distro}_{int(time.time())}")
            time.sleep(1)
        
        self.status_label.config(text="Mixed swarm deployed")
    
    def _deploy_single_node(self, distro: str, node_name: str):
        """Deploy single STEPPPS node"""
        if not self.orchestration_state["space"]["docker_available"]:
            print(f"❌ Docker unavailable - simulating {node_name}")
            self._simulate_steppps_node(distro, node_name)
            return
        
        try:
            # Create container
            image = f"{distro}:latest"
            port = self.base_port + len(self.nodes)
            
            cmd = [
                'sudo', 'docker', 'run', '-d', '-it',
                '--name', node_name,
                '--memory', '256m',
                '-p', f'{port}:8000',
                image, '/bin/sh'
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                container_id = result.stdout.strip()
                
                # Install Python and copy STEPPPS bootstrap
                subprocess.run([
                    'sudo', 'docker', 'exec', container_id,
                    '/bin/sh', '-c', 
                    'if ! command -v python3 >/dev/null 2>&1; then apk add python3 py3-pip || apt-get update && apt-get install -y python3 python3-pip; fi'
                ], capture_output=True)
                
                subprocess.run([
                    'sudo', 'docker', 'cp', 
                    'steppps_container_bootstrap.py', 
                    f'{container_id}:/steppps_bootstrap.py'
                ], capture_output=True)
                
                # Start STEPPPS in background
                subprocess.run([
                    'sudo', 'docker', 'exec', '-d', container_id,
                    'python3', '/steppps_bootstrap.py'
                ], capture_output=True)
                
                # Create node object
                node = STEPPPSNode(
                    container_id=container_id,
                    name=node_name,
                    image=image,
                    status="running",
                    steppps_state={},
                    last_heartbeat=time.time(),
                    communication_port=port,
                    output_buffer=[]
                )
                
                self.nodes[node_name] = node
                self.orchestration_state["space"]["active_nodes"] = len(self.nodes)
                
                print(f"✅ Node {node_name} deployed")
                
        except Exception as e:
            print(f"❌ Node deployment failed: {e}")
            self._simulate_steppps_node(distro, node_name)
    
    def _simulate_steppps_node(self, distro: str, node_name: str):
        """Simulate STEPPPS node when Docker unavailable"""
        node = STEPPPSNode(
            container_id=f"sim_{node_name}",
            name=node_name,
            image=f"{distro}:simulated",
            status="simulated",
            steppps_state={
                "space": {"environment": "simulated"},
                "time": {"uptime": 0},
                "psychology": {"emotion": "simulated"}
            },
            last_heartbeat=time.time(),
            communication_port=self.base_port + len(self.nodes),
            output_buffer=[f"Simulated {distro} STEPPPS node"]
        )
        
        self.nodes[node_name] = node
        print(f"✅ Simulated node {node_name}")
    
    def scale_up_swarm(self):
        """Scale up the swarm with additional nodes"""
        if len(self.nodes) >= self.max_nodes:
            print("❌ Maximum node limit reached")
            return
        
        # Add one node of each type
        distros = ["alpine", "ubuntu"]
        for distro in distros:
            if len(self.nodes) < self.max_nodes:
                node_name = f"steppps_{distro}_scale_{int(time.time())}"
                self._deploy_single_node(distro, node_name)
        
        self.status_label.config(text="Swarm scaled up")
    
    def terminate_swarm(self):
        """Terminate all nodes in swarm"""
        print("🛑 Terminating STEPPPS swarm...")
        
        for node_name, node in list(self.nodes.items()):
            try:
                if not node.container_id.startswith("sim_"):
                    subprocess.run([
                        'sudo', 'docker', 'stop', node.container_id
                    ], capture_output=True)
                    subprocess.run([
                        'sudo', 'docker', 'rm', node.container_id
                    ], capture_output=True)
                
                print(f"✅ Terminated {node_name}")
            except Exception as e:
                print(f"❌ Error terminating {node_name}: {e}")
        
        self.nodes.clear()
        self.orchestration_state["space"]["active_nodes"] = 0
        self.status_label.config(text="Swarm terminated")
    
    def run_orchestrator(self):
        """Run the STEPPPS swarm orchestrator"""
        print("🌌 Starting STEPPPS Swarm Orchestrator...")
        self.running = True
        
        if DISPLAY_AVAILABLE and self.root_window:
            # Start heartbeat monitoring
            threading.Thread(target=self._heartbeat_monitor, daemon=True).start()
            
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
    
    def _heartbeat_monitor(self):
        """Monitor node heartbeats"""
        while self.running:
            current_time = time.time()
            
            for node_name, node in list(self.nodes.items()):
                # Update heartbeat (simplified)
                node.last_heartbeat = current_time
                
                # Check for dead nodes
                if current_time - node.last_heartbeat > 30:
                    print(f"💔 Node {node_name} heartbeat lost")
                    node.status = "disconnected"
            
            time.sleep(self.heartbeat_interval)
    
    def shutdown(self):
        """Shutdown orchestrator"""
        print("🔄 Shutting down STEPPPS Swarm Orchestrator...")
        self.running = False
        self.terminate_swarm()
        
        if self.root_window:
            self.root_window.quit()
            self.root_window.destroy()
        
        print("✅ Orchestrator shutdown complete")

# Helper functions for canvas math
def cos(angle):
    """Cosine function"""
    import math
    return math.cos(angle)

def sin(angle):
    """Sine function"""
    import math
    return math.sin(angle)

def main():
    """Main entry point"""
    print("🌌 STEPPPS Swarm Orchestrator")
    print("=" * 40)
    
    orchestrator = STEPPPSSwarmOrchestrator()
    
    try:
        orchestrator.run_orchestrator()
    except Exception as e:
        print(f"💥 Fatal error: {e}")
        orchestrator.shutdown()

if __name__ == "__main__":
    main()
