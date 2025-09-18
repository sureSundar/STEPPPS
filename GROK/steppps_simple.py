#!/usr/bin/env python3
"""
STEPPPS Simple Terminal Platform
A working self-bootstrapping platform built using STEPPPS objects
Fixed version without JSON parsing issues
"""

import json
import http.server
import socketserver
import threading
import time
import os
from abc import ABC, abstractmethod

# Simple colors without ANSI codes for JSON compatibility
COLORS = {
    "red": "🔴",
    "green": "🟢",
    "blue": "🔵",
    "yellow": "🟡",
    "purple": "🟣",
    "cyan": "🔵",
    "white": "⚪"
}

class STEPPPS(ABC):
    """STEPPPS base class for platform objects"""

    def __init__(self, id, object_type="generic", **dimensions):
        self.id = id
        self.object_type = object_type
        self.data = {
            "space": dimensions.get("space", {"x": 0, "y": 0}),
            "temporal": dimensions.get("temporal", {"timestamp": time.time()}),
            "event": dimensions.get("event", {"lastEvent": None}),
            "psychology": dimensions.get("psychology", {"emotion": "neutral"}),
            "pixel": dimensions.get("pixel", {"color": "white"}),
            "prompt": dimensions.get("prompt", {"text": ""}),
            "script": dimensions.get("script", {"callback": None})
        }
        self.children = []
        self.parent = None
        self.platform_ref = None

    @abstractmethod
    def render(self):
        pass

    def bootstrap(self, source="storage", **kwargs):
        """Bootstrap this STEPPPS object"""
        try:
            if source == "genesis":
                self.data.update({
                    "psychology": {"mood": "creative", "energy": "high"},
                    "pixel": {"color": "cyan"},
                    "prompt": {"text": "Bootstrap platform genesis"}
                })
            elif source == "llm":
                # Simple LLM simulation without ANSI codes
                color_map = {
                    "menu": "cyan",
                    "button": "green",
                    "pixel": "purple",
                    "canvas": "blue"
                }
                self.data["pixel"]["color"] = color_map.get(self.object_type, "white")
                self.data["psychology"]["state"] = "active"

            self.data["event"]["lastEvent"] = f"bootstrapped_from_{source}"
            self.orchestrate_children()
            return True
        except Exception as e:
            print(f"Bootstrap failed for {self.id}: {e}")
            return False

    def orchestrate_children(self):
        """Bootstrap children"""
        for child in self.children:
            child.parent = self
            child.platform_ref = self.platform_ref
            child.bootstrap(source="parent")

    def add_child(self, child):
        child.parent = self
        child.platform_ref = self.platform_ref
        self.children.append(child)
        return child

    def to_json(self):
        return json.dumps({
            "id": self.id,
            "type": self.object_type,
            "data": self.data,
            "children": len(self.children)
        }, indent=2)

    def update_state(self, **updates):
        """Update STEPPPS state"""
        self.data.update(updates)
        self.data["temporal"]["last_modified"] = time.time()
        print(f"  📝 {self.id} updated: {updates}")

class SimplePlatform(STEPPPS):
    """Main platform object"""

    def __init__(self, id="simple_platform"):
        super().__init__(id, "platform")
        self.platform_ref = self
        self.running = True

    def bootstrap_platform(self):
        """Bootstrap the entire platform"""
        print("🚀 Bootstrapping Simple STEPPPS Platform...")

        self.bootstrap(source="genesis")

        # Create components
        header = SimpleHeader("header")
        self.add_child(header)

        canvas = SimpleCanvas("canvas")
        self.add_child(canvas)
        canvas.create_pixels()

        status = SimpleStatus("status")
        self.add_child(status)

        print("✅ Platform bootstrapped successfully!\n")
        return True

    def render(self):
        """Render the platform"""
        os.system('clear' if os.name == 'posix' else 'cls')

        for child in self.children:
            child.render()

    def run(self):
        """Run interactive platform"""
        while self.running:
            self.render()
            self.handle_input()

    def handle_input(self):
        """Handle user commands"""
        print("\n🎯 Commands: help | status | objects | click | json | quit")
        try:
            cmd = input("STEPPPS> ").strip().lower()

            if cmd in ['quit', 'exit', 'q']:
                self.running = False
                print("👋 STEPPPS platform stopped")
            elif cmd == 'help':
                self.show_help()
            elif cmd == 'status':
                self.show_status()
            elif cmd == 'objects':
                self.list_objects()
            elif cmd == 'click':
                self.click_pixel()
            elif cmd == 'json':
                print("\n📄 Platform JSON:")
                print(self.to_json())
            else:
                print(f"❌ Unknown command: {cmd}")

        except (KeyboardInterrupt, EOFError):
            self.running = False

    def show_help(self):
        print("""
🎯 STEPPPS Platform Commands:
  help    - Show this help
  status  - Platform status
  objects - List all STEPPPS objects
  click   - Simulate pixel click
  json    - Show platform JSON
  quit    - Exit platform
""")

    def show_status(self):
        print(f"\n📊 Platform Status:")
        print(f"  ID: {self.id}")
        print(f"  Objects: {len(self.children)}")
        print(f"  Psychology: {self.data['psychology']}")

    def list_objects(self):
        print(f"\n📦 STEPPPS Objects:")
        self._list_recursive(self, 0)

    def _list_recursive(self, obj, level):
        indent = "  " * level
        color_emoji = COLORS.get(obj.data["pixel"]["color"], "⚪")
        print(f"{indent}{color_emoji} {obj.id} ({obj.object_type})")
        for child in obj.children:
            self._list_recursive(child, level + 1)

    def click_pixel(self):
        """Simulate clicking first pixel"""
        canvas = None
        for child in self.children:
            if child.object_type == "canvas":
                canvas = child
                break

        if canvas and canvas.children:
            pixel = canvas.children[0]
            pixel.on_click()
            print(f"✅ Clicked pixel {pixel.id}")

class SimpleHeader(STEPPPS):
    def __init__(self, id):
        super().__init__(id, "header")

    def render(self):
        print("╔══════════════════════════════════════════════════════════════╗")
        print("║              STEPPPS Self-Bootstrapping Platform            ║")
        print("║               Every Element IS a STEPPPS Object             ║")
        print("╚══════════════════════════════════════════════════════════════╝")

class SimpleCanvas(STEPPPS):
    def __init__(self, id):
        super().__init__(id, "canvas")
        self.bootstrap(source="llm")

    def render(self):
        print(f"\n🎨 STEPPPS Canvas:")
        print("┌─────────────────────────────────────────┐")

        # Render pixels in 3x8 grid
        for row in range(3):
            line = "│ "
            for col in range(8):
                idx = row * 8 + col
                if idx < len(self.children):
                    pixel = self.children[idx]
                    color_emoji = COLORS.get(pixel.data["pixel"]["color"], "⚪")
                    line += f"{color_emoji} "
                else:
                    line += "  "
            line += " │"
            print(line)
        print("└─────────────────────────────────────────┘")

    def create_pixels(self):
        """Create STEPPPS pixel objects"""
        colors = ["red", "green", "blue", "yellow", "purple", "cyan", "white"]

        for i in range(24):  # 3x8 grid
            row, col = i // 8, i % 8
            pixel = SimplePixel(f"pixel_{row}_{col}", row, col)
            pixel.data["pixel"]["color"] = colors[i % len(colors)]
            self.add_child(pixel)

class SimplePixel(STEPPPS):
    def __init__(self, id, row, col):
        super().__init__(id, "pixel")
        self.data["space"]["row"] = row
        self.data["space"]["col"] = col
        self.bootstrap(source="llm")

    def render(self):
        pass  # Rendered by canvas

    def on_click(self):
        """Handle pixel click - cycle colors"""
        colors = ["red", "green", "blue", "yellow", "purple", "cyan", "white"]
        current = self.data["pixel"]["color"]
        try:
            idx = colors.index(current)
            new_color = colors[(idx + 1) % len(colors)]
        except ValueError:
            new_color = "red"

        # Update STEPPPS state
        self.update_state(**{
            "pixel": {"color": new_color},
            "event": {"lastEvent": "clicked", "click_time": time.time()}
        })

class SimpleStatus(STEPPPS):
    def __init__(self, id):
        super().__init__(id, "status")

    def render(self):
        objects = len(self.platform_ref.children) if self.platform_ref else 0
        print(f"\n📊 Status: Active | Objects: {objects} | Server: http://localhost:9001")

# HTTP Server for API
class SimpleHandler(http.server.BaseHTTPRequestHandler):
    platform_instance = None

    def do_GET(self):
        if self.path == "/status":
            data = {
                "status": "active",
                "platform": "STEPPPS Simple",
                "objects": len(self.platform_instance.children) if self.platform_instance else 0
            }
            self.send_json(data)
        elif self.path == "/objects":
            objects = []
            if self.platform_instance:
                self._collect_objects(self.platform_instance, objects)
            self.send_json(objects)
        else:
            self.send_response(404)
            self.end_headers()

    def _collect_objects(self, obj, objects):
        objects.append({
            "id": obj.id,
            "type": obj.object_type,
            "color": obj.data["pixel"]["color"]
        })
        for child in obj.children:
            self._collect_objects(child, objects)

    def send_json(self, data):
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode())

    def log_message(self, format, *args):
        pass  # Suppress logs

def run_server(port=9001):
    httpd = socketserver.TCPServer(("", port), SimpleHandler)
    print(f"🌐 Server running on http://localhost:{port}")
    threading.Thread(target=httpd.serve_forever, daemon=True).start()
    return httpd

def main():
    print("🚀 Starting Simple STEPPPS Platform...")

    # Start server
    server = run_server()

    # Create platform
    platform = SimplePlatform()
    SimpleHandler.platform_instance = platform

    # Bootstrap and run
    platform.bootstrap_platform()

    try:
        platform.run()
    except KeyboardInterrupt:
        pass
    finally:
        print("👋 Goodbye!")

if __name__ == "__main__":
    main()