#!/usr/bin/env python3
"""
STEPPPS Terminal Platform
A self-bootstrapping platform built entirely using STEPPPS objects
Runs in terminal to avoid X11/GUI issues while demonstrating the paradigm
Every 'UI' element (menu, input, display) IS a STEPPPS object
"""

import json
import http.server
import socketserver
import threading
import requests
import time
import os
import sys
from abc import ABC, abstractmethod

# ANSI color codes for terminal rendering
class Colors:
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BG_BLACK = '\033[40m'
    BG_BLUE = '\033[44m'

# Platform genesis configuration
PLATFORM_GENESIS_CONFIG = {
    "space": {"x": 0, "y": 0, "width": 80, "height": 24},
    "temporal": {"timestamp": time.time(), "created": time.time()},
    "event": {"lastEvent": "platform_genesis"},
    "psychology": {"mood": "creative", "energy": "high", "focus": "terminal"},
    "pixel": {"color": Colors.CYAN, "background": Colors.BG_BLACK},
    "prompt": {"text": "Bootstrap a terminal-based STEPPPS development platform"},
    "script": {"platform_type": "terminal_environment", "interactive": True}
}

class STEPPPS(ABC):
    """Enhanced STEPPPS base class for terminal platform objects"""

    def __init__(self, id, object_type="generic", **dimensions):
        self.id = id
        self.object_type = object_type
        self.data = {
            "space": dimensions.get("space", {"x": 0, "y": 0}),
            "temporal": dimensions.get("temporal", {"timestamp": time.time()}),
            "event": dimensions.get("event", {"lastEvent": None}),
            "psychology": dimensions.get("psychology", {"emotion": "neutral"}),
            "pixel": dimensions.get("pixel", {"color": Colors.WHITE}),
            "prompt": dimensions.get("prompt", {"text": ""}),
            "script": dimensions.get("script", {"callback": None})
        }
        self.children = []
        self.parent = None
        self.platform_ref = None
        self.visible = True

    @abstractmethod
    def render(self):
        """Terminal objects must implement their own rendering"""
        pass

    def bootstrap(self, source="storage", **kwargs):
        """Enhanced bootstrap for terminal objects"""
        try:
            if source == "genesis":
                self.data.update(PLATFORM_GENESIS_CONFIG)
            elif source == "platform_config":
                config_file = f"{self.object_type}_config.json"
                if os.path.exists(config_file):
                    with open(config_file, "r") as f:
                        config_data = json.load(f)
                        # Adapt colors for terminal
                        if "pixel" in config_data and "color" in config_data["pixel"]:
                            color_map = {
                                "#3498db": Colors.BLUE,
                                "#2ecc71": Colors.GREEN,
                                "#e74c3c": Colors.RED,
                                "#f39c12": Colors.YELLOW,
                                "#9b59b6": Colors.MAGENTA,
                                "#1abc9c": Colors.CYAN
                            }
                            hex_color = config_data["pixel"]["color"]
                            config_data["pixel"]["color"] = color_map.get(hex_color, Colors.WHITE)
                        self.data.update(config_data)
            elif source == "parent":
                if self.parent:
                    self.inherit_from_parent()
            elif source == "llm":
                prompt = kwargs.get("prompt", f"Generate terminal configuration for {self.object_type}")
                llm_response = self.simulate_llm(prompt)
                self.data.update(json.loads(llm_response))

            self.data["event"]["lastEvent"] = f"bootstrapped_from_{source}"
            self.orchestrate_children()
            return True
        except Exception as e:
            print(f"Bootstrap failed for {self.id}: {e}")
            return False

    def simulate_llm(self, prompt):
        """Terminal-adapted LLM simulation"""
        responses = {
            "menu": f'{{"pixel": {{"color": "{Colors.CYAN}"}}, "psychology": {{"state": "active"}}}}',
            "button": f'{{"pixel": {{"color": "{Colors.GREEN}"}}, "psychology": {{"state": "enabled"}}}}',
            "input": f'{{"pixel": {{"color": "{Colors.YELLOW}"}}, "psychology": {{"validation": "active"}}}}',
            "display": f'{{"pixel": {{"color": "{Colors.BLUE}"}}, "psychology": {{"render_mode": "live"}}}}',
            "pixel": f'{{"pixel": {{"color": "{Colors.MAGENTA}"}}}}'
        }
        return responses.get(self.object_type, f'{{"pixel": {{"color": "{Colors.WHITE}"}}}}')

    def inherit_from_parent(self):
        """Inherit properties from parent"""
        if self.parent:
            self.data["pixel"] = self.parent.data["pixel"].copy()
            self.data["psychology"] = self.parent.data["psychology"].copy()

    def orchestrate_children(self):
        """Enhanced orchestration for terminal objects"""
        for child in self.children:
            child.parent = self
            child.platform_ref = self.platform_ref
            child.bootstrap(source="parent")

    def add_child(self, child):
        """Add a child STEPPPS object"""
        child.parent = self
        child.platform_ref = self.platform_ref
        self.children.append(child)
        return child

    def to_json(self):
        return json.dumps({
            "id": self.id,
            "object_type": self.object_type,
            "data": self.data,
            "children_count": len(self.children)
        }, indent=2)

    def update_state(self, **updates):
        """Update STEPPPS state and log changes"""
        for key, value in updates.items():
            if "." in key:  # Handle nested updates like "pixel.color"
                parts = key.split(".")
                target = self.data
                for part in parts[:-1]:
                    target = target[part]
                target[parts[-1]] = value
            else:
                self.data[key] = value

        self.data["temporal"]["last_modified"] = time.time()
        print(f"  📝 {self.id} state updated: {updates}")

class TerminalPlatform(STEPPPS):
    """Root terminal platform - genesis object"""

    def __init__(self, id="terminal_platform_genesis"):
        super().__init__(id, "platform")
        self.platform_ref = self
        self.running = True
        self.components = {}
        self.current_menu = None

    def bootstrap_platform(self):
        """Genesis method - bootstraps the entire terminal platform"""
        print(f"{Colors.CYAN}{Colors.BOLD}🚀 Bootstrapping STEPPPS Terminal Platform...{Colors.RESET}")

        # Bootstrap self
        self.bootstrap(source="genesis")

        # Create core platform components
        self.bootstrap_header()
        self.bootstrap_main_menu()
        self.bootstrap_canvas()
        self.bootstrap_status()

        print(f"{Colors.GREEN}✅ Terminal Platform bootstrapped successfully!{Colors.RESET}\n")
        return True

    def bootstrap_header(self):
        """Bootstrap platform header"""
        header = TerminalHeader("platform_header")
        self.add_child(header)
        self.components["header"] = header

    def bootstrap_main_menu(self):
        """Bootstrap main menu system"""
        menu = TerminalMenu("main_menu")
        self.add_child(menu)
        self.components["menu"] = menu
        self.current_menu = menu

    def bootstrap_canvas(self):
        """Bootstrap canvas for STEPPPS objects"""
        canvas = TerminalCanvas("main_canvas")
        self.add_child(canvas)
        self.components["canvas"] = canvas

        # Add some demo pixels
        canvas.create_demo_pixels()

    def bootstrap_status(self):
        """Bootstrap status bar"""
        status = TerminalStatus("status_bar")
        self.add_child(status)
        self.components["status"] = status

    def render(self):
        """Render the entire platform"""
        # Clear screen
        os.system('clear' if os.name == 'posix' else 'cls')

        # Render all components
        for component in self.components.values():
            if component.visible:
                component.render()

    def run(self):
        """Main platform loop"""
        while self.running:
            self.render()
            self.handle_input()
            time.sleep(0.1)

    def handle_input(self):
        """Handle user input"""
        try:
            print(f"\n{Colors.YELLOW}STEPPPS> {Colors.RESET}", end="")
            user_input = input().strip().lower()

            if user_input in ['quit', 'exit', 'q']:
                self.running = False
                print(f"{Colors.RED}Shutting down STEPPPS platform...{Colors.RESET}")
            elif user_input == 'help':
                self.show_help()
            elif user_input == 'status':
                self.show_platform_status()
            elif user_input == 'objects':
                self.list_objects()
            elif user_input == 'click':
                self.simulate_pixel_click()
            elif user_input == 'bootstrap':
                self.re_bootstrap()
            elif user_input == 'json':
                self.show_platform_json()
            else:
                print(f"{Colors.RED}Unknown command: {user_input}. Type 'help' for commands.{Colors.RESET}")

        except KeyboardInterrupt:
            self.running = False
        except EOFError:
            self.running = False

    def show_help(self):
        """Show available commands"""
        help_text = f"""
{Colors.CYAN}{Colors.BOLD}STEPPPS Terminal Platform Commands:{Colors.RESET}
  {Colors.GREEN}help{Colors.RESET}      - Show this help
  {Colors.GREEN}status{Colors.RESET}    - Show platform status
  {Colors.GREEN}objects{Colors.RESET}   - List all STEPPPS objects
  {Colors.GREEN}click{Colors.RESET}     - Simulate pixel click
  {Colors.GREEN}bootstrap{Colors.RESET} - Re-bootstrap platform
  {Colors.GREEN}json{Colors.RESET}      - Show platform JSON
  {Colors.GREEN}quit{Colors.RESET}      - Exit platform
"""
        print(help_text)

    def show_platform_status(self):
        """Show platform status"""
        total_objects = len(self.children)
        print(f"\n{Colors.CYAN}📊 Platform Status:{Colors.RESET}")
        print(f"  Total STEPPPS Objects: {total_objects}")
        print(f"  Platform ID: {self.id}")
        print(f"  Bootstrap Source: {self.data['event']['lastEvent']}")
        print(f"  Psychology: {self.data['psychology']['mood']}")

    def list_objects(self):
        """List all STEPPPS objects"""
        print(f"\n{Colors.CYAN}📦 STEPPPS Objects:{Colors.RESET}")
        self._list_objects_recursive(self, 0)

    def _list_objects_recursive(self, obj, level):
        """Recursively list objects"""
        indent = "  " * level
        color = obj.data["pixel"]["color"]
        print(f"{indent}{color}├─ {obj.id} ({obj.object_type}){Colors.RESET}")
        for child in obj.children:
            self._list_objects_recursive(child, level + 1)

    def simulate_pixel_click(self):
        """Simulate clicking a pixel"""
        canvas = self.components.get("canvas")
        if canvas and canvas.children:
            pixel = canvas.children[0]  # Click first pixel
            pixel.on_click()
            print(f"{Colors.GREEN}✅ Simulated click on pixel {pixel.id}{Colors.RESET}")

    def re_bootstrap(self):
        """Re-bootstrap the platform"""
        print(f"{Colors.YELLOW}🔄 Re-bootstrapping platform...{Colors.RESET}")
        self.bootstrap_platform()

    def show_platform_json(self):
        """Show platform as JSON"""
        print(f"\n{Colors.CYAN}📄 Platform JSON:{Colors.RESET}")
        print(self.to_json())

class TerminalHeader(STEPPPS):
    """Platform header as STEPPPS object"""

    def __init__(self, id):
        super().__init__(id, "header")
        self.bootstrap(source="platform_config")

    def render(self):
        color = self.data["pixel"]["color"]
        print(f"{color}{Colors.BOLD}╔══════════════════════════════════════════════════════════════════════════════╗{Colors.RESET}")
        print(f"{color}{Colors.BOLD}║                    STEPPPS Self-Bootstrapping Platform                      ║{Colors.RESET}")
        print(f"{color}{Colors.BOLD}║                     Every Element IS a STEPPPS Object                       ║{Colors.RESET}")
        print(f"{color}{Colors.BOLD}╚══════════════════════════════════════════════════════════════════════════════╝{Colors.RESET}")

class TerminalMenu(STEPPPS):
    """Main menu as STEPPPS object"""

    def __init__(self, id):
        super().__init__(id, "menu")
        self.bootstrap(source="platform_config")

    def render(self):
        color = self.data["pixel"]["color"]
        print(f"\n{color}🎯 Available Commands: help | status | objects | click | bootstrap | json | quit{Colors.RESET}")

class TerminalCanvas(STEPPPS):
    """Canvas for STEPPPS pixel objects"""

    def __init__(self, id):
        super().__init__(id, "canvas")
        self.bootstrap(source="platform_config")

    def render(self):
        color = self.data["pixel"]["color"]
        print(f"\n{color}🎨 STEPPPS Canvas:{Colors.RESET}")

        # Render pixels in a grid
        print("┌─────────────────────────────────────────┐")
        for row in range(3):
            line = "│ "
            for col in range(8):
                pixel_idx = row * 8 + col
                if pixel_idx < len(self.children):
                    pixel = self.children[pixel_idx]
                    pixel_color = pixel.data["pixel"]["color"]
                    line += f"{pixel_color}●{Colors.RESET} "
                else:
                    line += "  "
            line += " │"
            print(line)
        print("└─────────────────────────────────────────┘")

    def create_demo_pixels(self):
        """Create demo STEPPPS pixel objects"""
        colors = [Colors.RED, Colors.GREEN, Colors.BLUE, Colors.YELLOW,
                 Colors.MAGENTA, Colors.CYAN, Colors.WHITE]

        for i in range(24):  # 3x8 grid
            row, col = i // 8, i % 8
            pixel = TerminalPixel(f"pixel_{row}_{col}", row=row, col=col)
            pixel.data["pixel"]["color"] = colors[i % len(colors)]
            self.add_child(pixel)

class TerminalPixel(STEPPPS):
    """Individual pixel as STEPPPS object"""

    def __init__(self, id, row=0, col=0):
        super().__init__(id, "pixel")
        self.data["space"]["row"] = row
        self.data["space"]["col"] = col
        self.bootstrap(source="llm", prompt="Generate pixel properties")

    def render(self):
        """Pixels are rendered by the canvas"""
        pass

    def on_click(self):
        """Handle pixel click"""
        colors = [Colors.RED, Colors.GREEN, Colors.BLUE, Colors.YELLOW,
                 Colors.MAGENTA, Colors.CYAN, Colors.WHITE]

        # Cycle through colors
        current_color = self.data["pixel"]["color"]
        try:
            current_idx = colors.index(current_color)
            new_color = colors[(current_idx + 1) % len(colors)]
        except ValueError:
            new_color = Colors.RED

        # Update STEPPPS state
        self.update_state(**{
            "pixel.color": new_color,
            "event.lastEvent": "clicked",
            "temporal.last_click": time.time()
        })

class TerminalStatus(STEPPPS):
    """Status bar as STEPPPS object"""

    def __init__(self, id):
        super().__init__(id, "status")
        self.bootstrap(source="platform_config")

    def render(self):
        color = self.data["pixel"]["color"]
        object_count = len(self.platform_ref.children) if self.platform_ref else 0
        print(f"\n{color}📊 Status: Platform Active | Objects: {object_count} | Server: http://localhost:9001{Colors.RESET}")

class TerminalSTEPPPSHandler(http.server.BaseHTTPRequestHandler):
    """HTTP handler for terminal platform"""
    platform_instance = None

    def do_GET(self):
        if self.path == "/platform/status":
            status = {
                "status": "active",
                "platform_type": "terminal",
                "objects": len(self.platform_instance.children) if self.platform_instance else 0,
                "uptime": time.time() - self.platform_instance.data["temporal"]["created"] if self.platform_instance else 0
            }
            self.send_json_response(status)
        elif self.path == "/platform/export":
            if self.platform_instance:
                self.send_json_response(json.loads(self.platform_instance.to_json()))
            else:
                self.send_json_response({"error": "Platform not initialized"})
        elif self.path == "/objects":
            objects = []
            if self.platform_instance:
                self._collect_objects(self.platform_instance, objects)
            self.send_json_response(objects)
        else:
            self.send_response(404)
            self.end_headers()

    def _collect_objects(self, obj, objects):
        """Collect all objects recursively"""
        objects.append({
            "id": obj.id,
            "type": obj.object_type,
            "state": obj.data
        })
        for child in obj.children:
            self._collect_objects(child, objects)

    def send_json_response(self, data):
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode())

    def log_message(self, format, *args):
        """Suppress HTTP server logs"""
        pass

def run_terminal_server(port=9001):
    """Run embedded server for terminal platform"""
    httpd = socketserver.TCPServer(("", port), TerminalSTEPPPSHandler)
    print(f"{Colors.GREEN}🌐 STEPPPS Server running on http://localhost:{port}{Colors.RESET}")
    threading.Thread(target=httpd.serve_forever, daemon=True).start()
    return httpd

def main():
    """Main function to bootstrap the terminal STEPPPS platform"""
    print(f"{Colors.BOLD}{Colors.CYAN}🚀 Starting STEPPPS Terminal Platform...{Colors.RESET}")

    # Start embedded server
    server = run_terminal_server()

    # Create and bootstrap platform
    platform = TerminalPlatform()

    # Link platform to server
    TerminalSTEPPPSHandler.platform_instance = platform

    # Bootstrap the platform
    platform.bootstrap_platform()

    # Run the interactive platform
    try:
        platform.run()
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Platform shutdown requested{Colors.RESET}")
    finally:
        print(f"{Colors.GREEN}🎯 STEPPPS Terminal Platform stopped{Colors.RESET}")

if __name__ == "__main__":
    main()