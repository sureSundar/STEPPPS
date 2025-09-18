#!/usr/bin/env python3
"""
Unified STEPPPP System - Best of Both Worlds
Combines living_bootstrap.py evolution with stepppp_bootstrap.py discovery

Features:
- Progressive 7-dimensional discovery (from stepppp_bootstrap.py)
- Real-time evolution cycles (from living_bootstrap.py)
- LLM-driven organic growth (from llm_organic_demo.py)
- Web dashboard with live updates
- Proper software engineering architecture
"""

import json
import time
import platform
import psutil
import socket
import subprocess
import os
import uuid
import threading
import http.server
import socketserver
import requests
import sqlite3
import hashlib
from typing import Dict, Any, Optional, List, Tuple
from datetime import datetime
from pathlib import Path

class UnifiedSTEPPPP:
    """Unified STEPPPP System combining all approaches"""

    def __init__(self, system_id: str, api_key: Optional[str] = None):
        self.system_id = system_id
        self.birth_time = time.time()
        self.api_key = api_key

        # 7-Dimensional Data Structure (from stepppp_bootstrap.py)
        self.data = {
            "space": {},       # Device, network, storage, location
            "temporal": {},    # Time, schedules, cycles, history
            "event": {},       # Capabilities, triggers, workflows
            "psychology": {},  # AI behavior, emotions, drives
            "pixel": {},       # Visual, UI, display capabilities
            "prompt": {},      # LLM context, instructions, templates
            "program": {}      # Services, apps, executable code
        }

        # Evolution State (from living_bootstrap.py)
        self.generation = 0
        self.discovery_step = 0
        self.discovery_log = []
        self.capabilities = []
        self.services = []
        self.evolution_history = []

        # Organic Growth (from organic systems)
        self.children = []
        self.learned_patterns = []
        self.growth_generation = 0

        # Web Service
        self.web_server_thread = None
        self.web_port = None

        # Initialize systems
        self._init_memory_system()
        self._init_evolution_system()

    def _init_memory_system(self):
        """Initialize unified memory system"""
        self.memory_db = f"unified_memory_{self.system_id}.db"
        conn = sqlite3.connect(self.memory_db)
        conn.executescript('''
            CREATE TABLE IF NOT EXISTS discoveries (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                step INTEGER,
                dimension TEXT,
                discovery_type TEXT,
                data TEXT,
                success_score REAL,
                generation INTEGER
            );

            CREATE TABLE IF NOT EXISTS evolution_cycles (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                generation INTEGER,
                evolution_type TEXT,
                success BOOLEAN,
                changes_made TEXT,
                performance_delta REAL
            );

            CREATE TABLE IF NOT EXISTS llm_interactions (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                prompt_type TEXT,
                prompt TEXT,
                response TEXT,
                success BOOLEAN,
                generation INTEGER
            );

            CREATE TABLE IF NOT EXISTS capability_history (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                capability_name TEXT,
                capability_type TEXT,
                strength REAL,
                source TEXT,
                generation INTEGER
            );
        ''')
        conn.close()
        print(f"🧠 Unified memory system initialized: {self.memory_db}")

    def _init_evolution_system(self):
        """Initialize evolution tracking system"""
        self.audit_trail = []
        self.performance_metrics = {
            "discovery_speed": 0.0,
            "evolution_rate": 0.0,
            "capability_growth": 0.0,
            "system_health": 1.0
        }

# ============================================================================
# PROGRESSIVE DISCOVERY (from stepppp_bootstrap.py approach)
# ============================================================================

    def discover_space_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of space dimension"""
        print(f"\n🌌 STEP {self.discovery_step + 1}: Space Dimension Discovery")
        print("-" * 40)

        space_data = {}

        # Device Information
        space_data["device"] = {
            "hostname": socket.gethostname(),
            "platform": platform.platform(),
            "system": platform.system(),
            "machine": platform.machine(),
            "processor": platform.processor(),
            "python_version": platform.python_version()
        }

        # Hardware Specs
        try:
            memory = psutil.virtual_memory()
            space_data["hardware"] = {
                "cpu_count": psutil.cpu_count(),
                "cpu_freq": psutil.cpu_freq()._asdict() if psutil.cpu_freq() else {},
                "memory_total_gb": round(memory.total / (1024**3), 2),
                "memory_available_gb": round(memory.available / (1024**3), 2),
                "memory_percent": memory.percent
            }
        except:
            space_data["hardware"] = {"status": "detection_failed"}

        # Network Topology
        try:
            space_data["network"] = {
                "hostname": socket.gethostname(),
                "local_ip": socket.gethostbyname(socket.gethostname()),
                "interfaces": {}
            }

            # Network interfaces
            for interface, addrs in psutil.net_if_addrs().items():
                interface_info = []
                for addr in addrs:
                    interface_info.append({
                        "family": str(addr.family),
                        "address": addr.address
                    })
                space_data["network"]["interfaces"][interface] = interface_info

        except:
            space_data["network"] = {"status": "detection_failed"}

        self.data["space"] = space_data
        self.discovery_step += 1
        self._log_discovery("space", "hardware_network_analysis", space_data, 0.9)

        print(f"✅ Space discovery complete - {len(space_data)} major components")
        return space_data

    def discover_temporal_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of temporal dimension"""
        print(f"\n⏰ STEP {self.discovery_step + 1}: Temporal Dimension Discovery")
        print("-" * 40)

        temporal_data = {}
        current_time = time.time()

        # Time Information
        temporal_data["time"] = {
            "timestamp": current_time,
            "birth_time": self.birth_time,
            "age_seconds": current_time - self.birth_time,
            "local_time": datetime.fromtimestamp(current_time).isoformat(),
            "timezone": time.tzname
        }

        # System Temporal Patterns
        try:
            boot_time = psutil.boot_time()
            temporal_data["system"] = {
                "boot_time": boot_time,
                "uptime_seconds": current_time - boot_time,
                "cpu_times": psutil.cpu_times()._asdict()
            }
        except:
            temporal_data["system"] = {"status": "detection_failed"}

        # Discovery Timeline
        temporal_data["discovery_timeline"] = {
            "steps_completed": self.discovery_step,
            "discovery_rate": self.discovery_step / max((current_time - self.birth_time), 1),
            "milestones": [entry["step"] for entry in self.discovery_log]
        }

        self.data["temporal"] = temporal_data
        self.discovery_step += 1
        self._log_discovery("temporal", "time_pattern_analysis", temporal_data, 0.85)

        print(f"✅ Temporal discovery complete - {len(temporal_data)} time aspects")
        return temporal_data

    def discover_event_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of event dimension with capability detection"""
        print(f"\n⚡ STEP {self.discovery_step + 1}: Event Dimension Discovery")
        print("-" * 40)

        event_data = {}

        # System Capabilities Detection
        capabilities = []

        # Hardware capabilities
        if self.data["space"].get("hardware", {}).get("cpu_count", 0) > 0:
            cpu_count = self.data["space"]["hardware"]["cpu_count"]
            capabilities.append({
                "name": "multi_core_processing",
                "type": "hardware",
                "strength": min(cpu_count / 4.0, 1.0),
                "source": "space_dimension_analysis",
                "discovered_at": time.time()
            })

        if self.data["space"].get("hardware", {}).get("memory_total_gb", 0) > 0:
            memory_gb = self.data["space"]["hardware"]["memory_total_gb"]
            capabilities.append({
                "name": "memory_processing",
                "type": "hardware",
                "strength": min(memory_gb / 16.0, 1.0),
                "source": "space_dimension_analysis",
                "discovered_at": time.time()
            })

        # Network capabilities
        interfaces = self.data["space"].get("network", {}).get("interfaces", {})
        if interfaces:
            capabilities.append({
                "name": "network_connectivity",
                "type": "network",
                "strength": min(len(interfaces) / 3.0, 1.0),
                "source": "space_dimension_analysis",
                "discovered_at": time.time()
            })

        # Python capabilities
        capabilities.append({
            "name": "python_runtime",
            "type": "software",
            "strength": 1.0,
            "source": "system_analysis",
            "discovered_at": time.time()
        })

        event_data["capabilities"] = capabilities
        self.capabilities.extend(capabilities)

        # Event Triggers
        triggers = [
            {
                "name": "discovery_completion",
                "type": "milestone",
                "condition": f"discovery_step == 7",
                "action": "start_evolution_cycles"
            },
            {
                "name": "capability_threshold",
                "type": "capability",
                "condition": f"len(capabilities) >= 5",
                "action": "begin_service_generation"
            }
        ]

        event_data["triggers"] = triggers

        self.data["event"] = event_data
        self.discovery_step += 1
        self._log_discovery("event", "capability_trigger_analysis", event_data, 0.95)

        print(f"✅ Event discovery complete - {len(capabilities)} capabilities discovered")
        return event_data

    def discover_psychology_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of psychology dimension"""
        print(f"\n🧠 STEP {self.discovery_step + 1}: Psychology Dimension Discovery")
        print("-" * 40)

        psychology_data = {}

        # Determine personality from system state
        memory_percent = self.data["space"].get("hardware", {}).get("memory_percent", 50)
        cpu_count = self.data["space"].get("hardware", {}).get("cpu_count", 1)
        capabilities_count = len(self.capabilities)

        # Emotion mapping
        if capabilities_count >= 4:
            emotion = "confident"
            confidence = 0.8
        elif self.discovery_step >= 3:
            emotion = "curious"
            confidence = 0.7
        else:
            emotion = "awakening"
            confidence = 0.5

        psychology_data["emotion"] = {
            "current": emotion,
            "confidence": confidence,
            "evolution_factor": self.discovery_step / 7.0
        }

        # Drive System
        drives = [
            {
                "name": "discovery_completion",
                "strength": 1.0 - (self.discovery_step / 7.0),
                "description": "Drive to complete all 7 dimensions"
            },
            {
                "name": "capability_expansion",
                "strength": min(capabilities_count / 10.0, 1.0),
                "description": f"Drive to expand {capabilities_count} capabilities"
            },
            {
                "name": "evolution_readiness",
                "strength": 0.8,
                "description": "Drive to begin evolution cycles"
            }
        ]

        psychology_data["drives"] = drives

        self.data["psychology"] = psychology_data
        self.discovery_step += 1
        self._log_discovery("psychology", "personality_drive_analysis", psychology_data, 0.8)

        print(f"✅ Psychology discovery complete - emotion: {emotion}, {len(drives)} drives")
        return psychology_data

    def discover_pixel_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of pixel dimension"""
        print(f"\n🎨 STEP {self.discovery_step + 1}: Pixel Dimension Discovery")
        print("-" * 40)

        pixel_data = {}

        # Visual capabilities detection
        visual_capabilities = [
            {
                "name": "terminal_display",
                "type": "text",
                "strength": 1.0,
                "features": ["colored_output", "progress_bars", "status_display"]
            },
            {
                "name": "web_visualization",
                "type": "web",
                "strength": 0.9,
                "features": ["html_dashboard", "live_updates", "responsive_design"]
            },
            {
                "name": "json_visualization",
                "type": "data",
                "strength": 1.0,
                "features": ["structured_display", "hierarchical_view"]
            }
        ]

        pixel_data["visual_capabilities"] = visual_capabilities

        # Color scheme based on psychology
        emotion = self.data.get("psychology", {}).get("emotion", {}).get("current", "neutral")

        color_schemes = {
            "curious": ["#00FF00", "#00CC88", "#0099CC"],
            "confident": ["#0066FF", "#0099FF", "#00CCFF"],
            "awakening": ["#66FF66", "#99FF99", "#CCFFCC"],
            "neutral": ["#808080", "#A0A0A0", "#C0C0C0"]
        }

        pixel_data["color_scheme"] = {
            "primary_colors": color_schemes.get(emotion, color_schemes["neutral"]),
            "emotion_based": emotion,
            "theme": f"{emotion}_theme"
        }

        self.data["pixel"] = pixel_data
        self.discovery_step += 1
        self._log_discovery("pixel", "visual_interface_analysis", pixel_data, 0.75)

        print(f"✅ Pixel discovery complete - {len(visual_capabilities)} visual capabilities")
        return pixel_data

    def discover_prompt_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of prompt dimension for LLM integration"""
        print(f"\n💬 STEP {self.discovery_step + 1}: Prompt Dimension Discovery")
        print("-" * 40)

        prompt_data = {}

        # Build comprehensive system context for LLM
        system_context = {
            "system_id": self.system_id,
            "generation": self.generation,
            "discovery_progress": self.discovery_step / 7.0,
            "capabilities_count": len(self.capabilities),
            "current_emotion": self.data.get("psychology", {}).get("emotion", {}).get("current", "unknown"),
            "hardware_summary": self.data.get("space", {}).get("hardware", {}),
            "system_age_minutes": (time.time() - self.birth_time) / 60
        }

        prompt_data["system_context"] = system_context

        # LLM Prompt Templates
        templates = {
            "capability_evolution": """
You are helping a 7-dimensional STEPPPP system evolve new capabilities.

Current System Status:
- ID: {system_id}
- Generation: {generation}
- Capabilities: {capabilities_count}
- Emotion: {current_emotion}
- Discovery Progress: {discovery_progress:.1%}

Hardware Context:
{hardware_summary}

Generate 2-3 new capabilities that would enhance this system's intelligence and autonomy.
Return as JSON with name, description, strength, and Python code.
            """,

            "service_generation": """
You are creating services for a STEPPPP system.

Available Capabilities: {capabilities}
System Context: {system_context}

Create a service that meaningfully combines multiple capabilities.
Return complete Python class implementation.
            """,

            "evolution_analysis": """
You are analyzing a STEPPPP system for next evolutionary steps.

Current State: {system_state}
Evolution History: {evolution_history}

Recommend the next evolution strategy and specific improvements.
            """
        }

        prompt_data["templates"] = templates

        # LLM Integration Configuration
        if self.api_key:
            prompt_data["llm_config"] = {
                "api_available": True,
                "model": "claude-3-5-sonnet-20241022",
                "max_tokens": 2000,
                "integration_active": True
            }
        else:
            prompt_data["llm_config"] = {
                "api_available": False,
                "fallback_mode": "template_based",
                "integration_active": False
            }

        self.data["prompt"] = prompt_data
        self.discovery_step += 1
        self._log_discovery("prompt", "llm_integration_setup", prompt_data, 0.9)

        llm_status = "ACTIVE" if self.api_key else "TEMPLATE_MODE"
        print(f"✅ Prompt discovery complete - LLM integration: {llm_status}")
        return prompt_data

    def discover_program_dimension(self) -> Dict[str, Any]:
        """Progressive discovery of program dimension with service generation"""
        print(f"\n💻 STEP {self.discovery_step + 1}: Program Dimension Discovery")
        print("-" * 40)

        program_data = {}

        # Analyze capabilities for service generation
        capability_types = {}
        for cap in self.capabilities:
            cap_type = cap.get("type", "unknown")
            if cap_type not in capability_types:
                capability_types[cap_type] = []
            capability_types[cap_type].append(cap)

        # Generate service combinations
        service_combinations = []

        # Web services (if network capability exists)
        if "network" in capability_types:
            service_combinations.append({
                "name": "unified_web_dashboard",
                "type": "web_service",
                "capabilities_used": ["network_connectivity", "python_runtime"],
                "description": "Comprehensive web dashboard with live system monitoring",
                "port": 8080,
                "endpoints": ["/status", "/dimensions", "/capabilities", "/evolution"]
            })

        # Processing services (if multi-core available)
        if "hardware" in capability_types and len(capability_types["hardware"]) > 1:
            service_combinations.append({
                "name": "parallel_evolution_engine",
                "type": "processing_service",
                "capabilities_used": ["multi_core_processing", "memory_processing"],
                "description": "Parallel processing for evolution cycles",
                "features": ["concurrent_analysis", "load_balancing"]
            })

        # AI services (if LLM available)
        if self.api_key:
            service_combinations.append({
                "name": "llm_evolution_service",
                "type": "ai_service",
                "capabilities_used": ["python_runtime", "network_connectivity"],
                "description": "LLM-driven capability and service generation",
                "features": ["code_generation", "analysis", "optimization"]
            })

        program_data["service_combinations"] = service_combinations
        self.services.extend(service_combinations)

        # Executable implementations
        executable_services = self._generate_executable_services()
        program_data["executable_services"] = executable_services

        self.data["program"] = program_data
        self.discovery_step += 1
        self._log_discovery("program", "service_generation_complete", program_data, 1.0)

        print(f"✅ Program discovery complete - {len(service_combinations)} services generated")
        return program_data

# ============================================================================
# EVOLUTION CYCLES (from living_bootstrap.py approach)
# ============================================================================

    def evolve_with_llm(self) -> bool:
        """LLM-driven evolution cycle"""
        if not self.api_key:
            print("⚠️  No LLM API key - using template-based evolution")
            return self._evolve_with_templates()

        print(f"\n🤖 LLM EVOLUTION CYCLE - Generation {self.generation + 1}")
        print("-" * 50)

        evolution_success = False

        # 1. LLM Capability Generation
        new_capabilities = self._llm_generate_capabilities()
        if new_capabilities:
            self.capabilities.extend(new_capabilities)
            evolution_success = True
            print(f"🚀 Generated {len(new_capabilities)} new capabilities")

        # 2. LLM Service Creation
        new_services = self._llm_generate_services()
        if new_services:
            self.services.extend(new_services)
            evolution_success = True
            print(f"⚙️ Generated {len(new_services)} new services")

        # 3. LLM System Analysis
        analysis = self._llm_analyze_system()
        if analysis:
            evolution_success = True
            print(f"🔍 LLM system analysis completed")

        if evolution_success:
            self.generation += 1
            self._log_evolution_cycle("llm_driven", True, f"Generated capabilities and services")

        return evolution_success

    def _llm_generate_capabilities(self) -> List[Dict[str, Any]]:
        """Use LLM to generate new capabilities"""
        if not self.api_key:
            return []

        system_context = self.data["prompt"]["system_context"]

        prompt = self.data["prompt"]["templates"]["capability_evolution"].format(
            system_id=self.system_id,
            generation=self.generation,
            capabilities_count=len(self.capabilities),
            current_emotion=system_context["current_emotion"],
            discovery_progress=system_context["discovery_progress"],
            hardware_summary=json.dumps(system_context["hardware_summary"], indent=2)
        )

        response = self._consult_llm(prompt, "capability_generation")
        if response:
            return self._parse_llm_capabilities(response)

        return []

    def _llm_generate_services(self) -> List[Dict[str, Any]]:
        """Use LLM to generate new services"""
        if not self.api_key or len(self.capabilities) < 2:
            return []

        capabilities_summary = [
            {"name": cap["name"], "type": cap["type"], "strength": cap["strength"]}
            for cap in self.capabilities[:5]  # Use top 5
        ]

        prompt = self.data["prompt"]["templates"]["service_generation"].format(
            capabilities=json.dumps(capabilities_summary, indent=2),
            system_context=json.dumps(self.data["prompt"]["system_context"], indent=2)
        )

        response = self._consult_llm(prompt, "service_generation")
        if response:
            return self._parse_llm_services(response)

        return []

    def _llm_analyze_system(self) -> Optional[Dict[str, Any]]:
        """LLM system analysis for optimization"""
        if not self.api_key:
            return None

        system_state = {
            "dimensions_complete": self.discovery_step >= 7,
            "capabilities_count": len(self.capabilities),
            "services_count": len(self.services),
            "generation": self.generation,
            "performance_metrics": self.performance_metrics
        }

        prompt = self.data["prompt"]["templates"]["evolution_analysis"].format(
            system_state=json.dumps(system_state, indent=2),
            evolution_history=json.dumps(self.evolution_history[-3:], indent=2, default=str)
        )

        response = self._consult_llm(prompt, "system_analysis")
        if response:
            return {"analysis": response, "timestamp": time.time()}

        return None

    def _consult_llm(self, prompt: str, prompt_type: str) -> Optional[str]:
        """Consult LLM with proper API call"""
        print(f"🤖 Consulting LLM for: {prompt_type}")

        try:
            headers = {
                "x-api-key": self.api_key,
                "Content-Type": "application/json",
                "anthropic-version": "2023-06-01"
            }

            data = {
                "model": "claude-3-5-sonnet-20241022",
                "max_tokens": 2000,
                "messages": [{"role": "user", "content": prompt}]
            }

            response = requests.post(
                "https://api.anthropic.com/v1/messages",
                headers=headers,
                json=data,
                timeout=30
            )

            if response.status_code == 200:
                result = response.json()["content"][0]["text"]
                self._log_llm_interaction(prompt_type, prompt, result, True)
                print(f"   ✅ LLM response received ({len(result)} chars)")
                return result
            else:
                print(f"   ❌ LLM API error: {response.status_code}")

        except Exception as e:
            print(f"   ❌ LLM consultation failed: {e}")
            self._log_llm_interaction(prompt_type, prompt, str(e), False)

        return None

# ============================================================================
# WEB SERVICE (from both approaches combined)
# ============================================================================

    def start_web_service(self, port: int = 8090):
        """Start unified web service with dashboard"""
        self.web_port = port

        class UnifiedWebHandler(http.server.BaseHTTPRequestHandler):
            def __init__(self, *args, stepppp_system=None, **kwargs):
                self.stepppp = stepppp_system
                super().__init__(*args, **kwargs)

            def do_GET(self):
                if self.path == '/' or self.path == '/index.html':
                    self.serve_unified_dashboard()
                elif self.path == '/data':
                    self.serve_system_data()
                elif self.path == '/dimensions':
                    self.serve_dimensions()
                elif self.path == '/capabilities':
                    self.serve_capabilities()
                elif self.path == '/evolution':
                    self.serve_evolution_history()
                else:
                    self.send_response(404)
                    self.end_headers()

            def serve_unified_dashboard(self):
                primary_color = "#00FF00"
                if self.stepppp.data.get("pixel", {}).get("color_scheme"):
                    primary_color = self.stepppp.data["pixel"]["color_scheme"]["primary_colors"][0]

                html_content = f'''
<!DOCTYPE html>
<html>
<head>
    <title>Unified STEPPPP System - {self.stepppp.system_id}</title>
    <style>
        body {{
            font-family: 'Courier New', monospace;
            margin: 0; padding: 20px;
            background: #0a0a0a; color: {primary_color};
        }}
        .container {{ max-width: 1400px; margin: 0 auto; }}
        .header {{ text-align: center; border-bottom: 2px solid {primary_color}; padding-bottom: 20px; }}
        .section {{
            border: 1px solid {primary_color}; margin: 20px 0; padding: 20px;
            border-radius: 8px; background: rgba(0,255,0,0.05);
        }}
        .progress {{
            background: #333; height: 24px; border-radius: 12px;
            margin: 15px 0; position: relative; overflow: hidden;
        }}
        .progress-bar {{
            background: linear-gradient(90deg, {primary_color}, #00CC88);
            height: 100%; border-radius: 12px; transition: width 0.5s ease;
        }}
        .metric {{ margin: 8px 0; display: flex; justify-content: space-between; }}
        .live-data {{
            background: #111; padding: 15px; border-radius: 8px;
            font-family: monospace; font-size: 12px; max-height: 300px; overflow-y: auto;
        }}
        .nav {{ margin: 20px 0; text-align: center; }}
        .nav a {{
            color: {primary_color}; text-decoration: none; margin: 0 15px;
            padding: 8px 16px; border: 1px solid {primary_color}; border-radius: 4px;
        }}
        .nav a:hover {{ background: {primary_color}; color: #000; }}
    </style>
    <script>
        function updateDashboard() {{
            fetch('/data')
                .then(response => response.json())
                .then(data => {{
                    document.getElementById('live-data').innerHTML = JSON.stringify(data, null, 2);

                    // Update progress bars
                    const discoveryProgress = (data.discovery_step / 7) * 100;
                    document.getElementById('discovery-progress').style.width = discoveryProgress + '%';
                    document.getElementById('discovery-text').textContent = Math.round(discoveryProgress) + '%';

                    // Update metrics
                    document.getElementById('generation').textContent = data.generation;
                    document.getElementById('capabilities').textContent = data.capabilities_count;
                    document.getElementById('services').textContent = data.services_count;
                }})
                .catch(error => console.error('Update failed:', error));
        }}

        setInterval(updateDashboard, 3000);
        window.onload = updateDashboard;
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌱 Unified STEPPPP System</h1>
            <h2>{self.stepppp.system_id}</h2>
            <p>Progressive Discovery + Evolution Cycles + LLM Integration</p>
        </div>

        <div class="nav">
            <a href="/dimensions">Dimensions</a>
            <a href="/capabilities">Capabilities</a>
            <a href="/evolution">Evolution History</a>
        </div>

        <div class="section">
            <h3>🔍 Discovery Progress</h3>
            <div class="progress">
                <div class="progress-bar" id="discovery-progress" style="width: {(self.stepppp.discovery_step/7)*100}%">
                    <div style="position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); color: #000; font-weight: bold;" id="discovery-text">{int((self.stepppp.discovery_step/7)*100)}%</div>
                </div>
            </div>
            <div class="metric"><span>Steps Completed:</span><span>{self.stepppp.discovery_step}/7</span></div>
        </div>

        <div class="section">
            <h3>🧬 Evolution Status</h3>
            <div class="metric"><span>Generation:</span><span id="generation">{self.stepppp.generation}</span></div>
            <div class="metric"><span>Capabilities:</span><span id="capabilities">{len(self.stepppp.capabilities)}</span></div>
            <div class="metric"><span>Services:</span><span id="services">{len(self.stepppp.services)}</span></div>
        </div>

        <div class="section">
            <h3>📡 Live System Data</h3>
            <div class="live-data" id="live-data">Loading...</div>
        </div>
    </div>
</body>
</html>
                '''

                self.send_response(200)
                self.send_header('Content-type', 'text/html')
                self.end_headers()
                self.wfile.write(html_content.encode())

            def serve_system_data(self):
                response_data = {
                    "system_id": self.stepppp.system_id,
                    "generation": self.stepppp.generation,
                    "discovery_step": self.stepppp.discovery_step,
                    "capabilities_count": len(self.stepppp.capabilities),
                    "services_count": len(self.stepppp.services),
                    "uptime": time.time() - self.stepppp.birth_time,
                    "timestamp": time.time()
                }

                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response_data, indent=2).encode())

            def serve_dimensions(self):
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(self.stepppp.data, indent=2, default=str).encode())

            def serve_capabilities(self):
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(self.stepppp.capabilities, indent=2).encode())

            def serve_evolution_history(self):
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(self.stepppp.evolution_history, indent=2, default=str).encode())

        # Create handler with stepppp reference
        def handler_factory(*args, **kwargs):
            return UnifiedWebHandler(*args, stepppp_system=self, **kwargs)

        try:
            with socketserver.TCPServer(("", port), handler_factory) as httpd:
                print(f"🌐 Unified Web Service running on http://localhost:{port}")
                print(f"📊 Dashboard: http://localhost:{port}/")
                httpd.serve_forever()
        except OSError:
            print(f"❌ Port {port} in use - web service failed to start")

# ============================================================================
# UNIFIED BOOTSTRAP ORCHESTRATION
# ============================================================================

    def unified_bootstrap(self):
        """Complete unified bootstrap: discovery → evolution → service"""
        print(f"🚀 UNIFIED STEPPPP BOOTSTRAP")
        print("=" * 60)
        print(f"System: {self.system_id}")
        print(f"LLM Integration: {'ACTIVE' if self.api_key else 'TEMPLATE_MODE'}")
        print("=" * 60)

        try:
            # Phase 1: Progressive Discovery (from stepppp_bootstrap.py)
            print(f"\n📍 PHASE 1: Progressive 7-Dimensional Discovery")
            print("-" * 60)

            discovery_functions = [
                self.discover_space_dimension,
                self.discover_temporal_dimension,
                self.discover_event_dimension,
                self.discover_psychology_dimension,
                self.discover_pixel_dimension,
                self.discover_prompt_dimension,
                self.discover_program_dimension
            ]

            for i, discovery_func in enumerate(discovery_functions, 1):
                result = discovery_func()
                print(f"📋 Discovery {i}/7 completed")
                time.sleep(0.5)  # Brief pause for observation

            print(f"\n✅ DISCOVERY PHASE COMPLETE!")
            print(f"📊 Discovered: {self.discovery_step} dimensions, {len(self.capabilities)} capabilities, {len(self.services)} services")

            # Phase 2: Evolution Cycles (from living_bootstrap.py)
            print(f"\n📍 PHASE 2: Evolution Cycles")
            print("-" * 60)

            max_evolution_cycles = 3
            for cycle in range(max_evolution_cycles):
                print(f"\n🔄 Evolution Cycle {cycle + 1}/{max_evolution_cycles}")

                success = self.evolve_with_llm()
                if success:
                    print(f"✅ Evolution cycle {cycle + 1} completed successfully")
                else:
                    print(f"⏸️ Evolution cycle {cycle + 1} - no changes needed")

                if cycle < max_evolution_cycles - 1:
                    time.sleep(2)  # Rest between cycles

            print(f"\n✅ EVOLUTION PHASE COMPLETE!")
            print(f"📊 Final State: Generation {self.generation}, {len(self.capabilities)} capabilities, {len(self.services)} services")

            # Phase 3: Web Service (from both approaches)
            print(f"\n📍 PHASE 3: Web Service Launch")
            print("-" * 60)

            # Start web service in background thread
            self.web_server_thread = threading.Thread(
                target=self.start_web_service,
                args=(8090,),
                daemon=True
            )
            self.web_server_thread.start()

            print(f"🌐 Web service started in background")
            print(f"📊 Dashboard available at: http://localhost:8090")

            # Save final state
            self._save_final_state()

            return True

        except KeyboardInterrupt:
            print(f"\n👋 Unified bootstrap interrupted by user")
            return False
        except Exception as e:
            print(f"\n❌ Unified bootstrap failed: {e}")
            return False

# ============================================================================
# UTILITY METHODS
# ============================================================================

    def _log_discovery(self, dimension: str, discovery_type: str, data: Dict[str, Any], success_score: float):
        """Log discovery to unified memory system"""
        conn = sqlite3.connect(self.memory_db)
        conn.execute('''
            INSERT INTO discoveries
            (timestamp, step, dimension, discovery_type, data, success_score, generation)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (time.time(), self.discovery_step, dimension, discovery_type,
              json.dumps(data, default=str), success_score, self.generation))
        conn.commit()
        conn.close()

        self.discovery_log.append({
            "step": self.discovery_step,
            "dimension": dimension,
            "discovery_type": discovery_type,
            "success_score": success_score,
            "timestamp": time.time()
        })

    def _log_evolution_cycle(self, evolution_type: str, success: bool, changes: str):
        """Log evolution cycle to memory system"""
        conn = sqlite3.connect(self.memory_db)
        conn.execute('''
            INSERT INTO evolution_cycles
            (timestamp, generation, evolution_type, success, changes_made, performance_delta)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (time.time(), self.generation, evolution_type, success, changes, 0.1 if success else 0.0))
        conn.commit()
        conn.close()

        self.evolution_history.append({
            "generation": self.generation,
            "evolution_type": evolution_type,
            "success": success,
            "changes": changes,
            "timestamp": time.time()
        })

    def _log_llm_interaction(self, prompt_type: str, prompt: str, response: str, success: bool):
        """Log LLM interaction"""
        conn = sqlite3.connect(self.memory_db)
        conn.execute('''
            INSERT INTO llm_interactions
            (timestamp, prompt_type, prompt, response, success, generation)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (time.time(), prompt_type, prompt[:500], response[:1000], success, self.generation))
        conn.commit()
        conn.close()

    def _parse_llm_capabilities(self, response: str) -> List[Dict[str, Any]]:
        """Parse LLM-generated capabilities"""
        capabilities = []
        try:
            # Try JSON parsing first
            import re
            json_match = re.search(r'\[.*\]', response, re.DOTALL)
            if json_match:
                capabilities_data = json.loads(json_match.group())
                for cap_data in capabilities_data:
                    capability = {
                        "name": cap_data.get("name", f"llm_capability_{int(time.time())}"),
                        "type": "llm_generated",
                        "strength": cap_data.get("strength", 0.7),
                        "description": cap_data.get("description", "LLM-generated capability"),
                        "source": "llm_generation",
                        "discovered_at": time.time()
                    }
                    capabilities.append(capability)
        except:
            # Fallback capability
            capabilities.append({
                "name": "llm_analysis_capability",
                "type": "llm_generated",
                "strength": 0.6,
                "description": "Capability extracted from LLM response",
                "source": "llm_generation",
                "discovered_at": time.time()
            })

        return capabilities

    def _parse_llm_services(self, response: str) -> List[Dict[str, Any]]:
        """Parse LLM-generated services"""
        services = []
        if response:
            service = {
                "name": f"llm_generated_service_{int(time.time())}",
                "type": "llm_generated",
                "description": "Service generated by LLM analysis",
                "source": "llm_generation",
                "generated_at": time.time()
            }
            services.append(service)
        return services

    def _evolve_with_templates(self) -> bool:
        """Template-based evolution when no LLM available"""
        template_capabilities = [
            {
                "name": "adaptive_processing",
                "type": "template_generated",
                "strength": 0.7,
                "description": "Template-based adaptive processing capability",
                "source": "template_generation",
                "discovered_at": time.time()
            }
        ]

        self.capabilities.extend(template_capabilities)
        self.generation += 1
        self._log_evolution_cycle("template_based", True, "Added template capabilities")
        return True

    def _generate_executable_services(self) -> Dict[str, str]:
        """Generate executable service implementations"""
        services = {}

        # Basic status service implementation
        services["status_service"] = '''
def create_status_service(stepppp_system):
    class StatusService:
        def __init__(self, system):
            self.system = system

        def get_status(self):
            return {
                "system_id": self.system.system_id,
                "generation": self.system.generation,
                "discovery_step": self.system.discovery_step,
                "capabilities": len(self.system.capabilities),
                "services": len(self.system.services),
                "uptime": time.time() - self.system.birth_time
            }

    return StatusService(stepppp_system)
'''

        return services

    def _save_final_state(self):
        """Save final system state"""
        final_state = {
            "system_id": self.system_id,
            "birth_time": self.birth_time,
            "generation": self.generation,
            "discovery_step": self.discovery_step,
            "dimensions": self.data,
            "capabilities": self.capabilities,
            "services": self.services,
            "evolution_history": self.evolution_history,
            "discovery_log": self.discovery_log,
            "performance_metrics": self.performance_metrics
        }

        filename = f"unified_final_state_{self.system_id}.json"
        with open(filename, 'w') as f:
            json.dump(final_state, f, indent=2, default=str)

        print(f"💾 Final state saved to: {filename}")

    def get_unified_status(self) -> Dict[str, Any]:
        """Get complete unified system status"""
        return {
            "system_id": self.system_id,
            "type": "unified_stepppp",
            "birth_time": self.birth_time,
            "age_minutes": (time.time() - self.birth_time) / 60,
            "discovery": {
                "step": self.discovery_step,
                "progress": self.discovery_step / 7.0,
                "completed": self.discovery_step >= 7
            },
            "evolution": {
                "generation": self.generation,
                "capabilities_count": len(self.capabilities),
                "services_count": len(self.services),
                "evolution_cycles": len(self.evolution_history)
            },
            "integration": {
                "llm_available": bool(self.api_key),
                "web_service_active": bool(self.web_server_thread),
                "memory_db": self.memory_db
            },
            "performance": self.performance_metrics
        }

# ============================================================================
# MAIN UNIFIED DEMO
# ============================================================================

def create_unified_system() -> UnifiedSTEPPPP:
    """Create unified STEPPPP system with all features"""
    system_id = f"unified_stepppp_{int(time.time())}"

    # Load API key if available
    api_key = None
    try:
        with open("claude_config.json", "r") as f:
            config = json.load(f)
            api_key = config.get("anthropic_api_key")
    except:
        pass

    return UnifiedSTEPPPP(system_id, api_key)

def main():
    """Main unified STEPPPP demonstration"""
    print("🌱 UNIFIED STEPPPP SYSTEM")
    print("Combining Progressive Discovery + Evolution Cycles + LLM Integration")
    print("=" * 80)

    # Create unified system
    unified_system = create_unified_system()

    print(f"✅ Unified system created: {unified_system.system_id}")
    print(f"🤖 LLM Integration: {'ACTIVE' if unified_system.api_key else 'TEMPLATE_MODE'}")

    # Run unified bootstrap
    success = unified_system.unified_bootstrap()

    if success:
        print(f"\n🎉 UNIFIED BOOTSTRAP COMPLETE!")

        status = unified_system.get_unified_status()
        print(f"\n📊 Final Unified System Status:")
        for key, value in status.items():
            if isinstance(value, dict):
                print(f"   {key}:")
                for sub_key, sub_value in value.items():
                    print(f"      {sub_key}: {sub_value}")
            else:
                print(f"   {key}: {value}")

        print(f"\n🌐 Web Dashboard: http://localhost:8090")
        print(f"📡 API Endpoints: /data, /dimensions, /capabilities, /evolution")

        # Keep running for web service
        try:
            print(f"\n⏳ System running... Press Ctrl+C to stop")
            while True:
                time.sleep(10)

        except KeyboardInterrupt:
            print(f"\n👋 Unified STEPPPP system shutdown")

    else:
        print(f"\n❌ Unified bootstrap failed")

if __name__ == "__main__":
    main()