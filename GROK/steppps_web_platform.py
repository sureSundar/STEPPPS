#!/usr/bin/env python3
"""
STEPPPS Web Platform
A self-bootstrapping platform using HTML5 Canvas as the primary display
Every canvas element (pixel, shape, UI component) IS a STEPPPS object
"""

import json
import http.server
import socketserver
import threading
import time
import os
import webbrowser
from abc import ABC, abstractmethod

class STEPPPS(ABC):
    """Enhanced STEPPPS base class for web canvas objects"""

    def __init__(self, id, object_type="generic", **dimensions):
        self.id = id
        self.object_type = object_type
        self.data = {
            "space": dimensions.get("space", {"x": 0, "y": 0, "width": 10, "height": 10}),
            "temporal": dimensions.get("temporal", {"timestamp": time.time(), "frame": 0}),
            "event": dimensions.get("event", {"lastEvent": None, "interactive": True}),
            "psychology": dimensions.get("psychology", {"emotion": "neutral", "energy": 0.5}),
            "pixel": dimensions.get("pixel", {"color": "#3498db", "alpha": 1.0}),
            "prompt": dimensions.get("prompt", {"text": "", "llm_temperature": 0.7}),
            "script": dimensions.get("script", {"animation": None, "behavior": None})
        }
        self.children = []
        self.parent = None
        self.platform_ref = None
        self.visible = True
        self.dirty = True  # Needs redraw

    def bootstrap(self, source="storage", **kwargs):
        """Bootstrap this STEPPPS object for web canvas"""
        try:
            if source == "genesis":
                self.data.update({
                    "psychology": {"emotion": "creative", "energy": 1.0},
                    "pixel": {"color": "#2c3e50", "alpha": 1.0},
                    "prompt": {"text": "Bootstrap web platform genesis"}
                })
            elif source == "llm":
                # Enhanced LLM simulation for web objects
                self.simulate_web_llm(kwargs.get("prompt", f"Generate {self.object_type} properties"))
            elif source == "user_input":
                # Bootstrap from user interaction
                self.data["event"]["lastEvent"] = "user_interaction"
                self.data["psychology"]["energy"] = min(1.0, self.data["psychology"]["energy"] + 0.2)

            self.data["event"]["lastEvent"] = f"bootstrapped_from_{source}"
            self.data["temporal"]["timestamp"] = time.time()
            self.orchestrate_children()
            self.dirty = True
            return True
        except Exception as e:
            print(f"Bootstrap failed for {self.id}: {e}")
            return False

    def simulate_web_llm(self, prompt):
        """Enhanced LLM simulation for web canvas objects"""
        responses = {
            "platform": {
                "pixel": {"color": "#2c3e50", "alpha": 1.0},
                "psychology": {"emotion": "stable", "energy": 0.8}
            },
            "canvas": {
                "pixel": {"color": "#ecf0f1", "alpha": 1.0},
                "psychology": {"emotion": "receptive", "energy": 0.6}
            },
            "pixel": {
                "pixel": {"color": "#e74c3c", "alpha": 0.8},
                "psychology": {"emotion": "vibrant", "energy": 0.9}
            },
            "button": {
                "pixel": {"color": "#3498db", "alpha": 1.0},
                "psychology": {"emotion": "ready", "energy": 0.7}
            },
            "shape": {
                "pixel": {"color": "#9b59b6", "alpha": 0.9},
                "psychology": {"emotion": "fluid", "energy": 0.8}
            }
        }

        config = responses.get(self.object_type, {"pixel": {"color": "#95a5a6", "alpha": 0.5}})
        self.data.update(config)

    def orchestrate_children(self):
        """Enhanced orchestration for web objects"""
        for child in self.children:
            child.parent = self
            child.platform_ref = self.platform_ref
            child.bootstrap(source="parent")

    def add_child(self, child):
        """Add child and maintain canvas hierarchy"""
        child.parent = self
        child.platform_ref = self.platform_ref
        self.children.append(child)
        self.dirty = True
        return child

    def to_canvas_json(self):
        """Convert to JSON for canvas rendering"""
        return {
            "id": self.id,
            "type": self.object_type,
            "space": self.data["space"],
            "pixel": self.data["pixel"],
            "visible": self.visible,
            "children": [child.to_canvas_json() for child in self.children]
        }

    def update_state(self, **updates):
        """Update state and mark for redraw"""
        for key, value in updates.items():
            if "." in key:
                parts = key.split(".")
                target = self.data
                for part in parts[:-1]:
                    target = target[part]
                target[parts[-1]] = value
            else:
                self.data[key] = value

        self.data["temporal"]["timestamp"] = time.time()
        self.dirty = True
        if self.platform_ref:
            self.platform_ref.dirty = True

    @abstractmethod
    def render_to_canvas(self):
        """Return canvas drawing commands"""
        pass

class WebPlatform(STEPPPS):
    """Root web platform object"""

    def __init__(self, id="web_platform_genesis"):
        super().__init__(id, "platform")
        self.platform_ref = self
        self.canvas_width = 1200
        self.canvas_height = 800
        self.frame_count = 0

    def bootstrap_platform(self):
        """Bootstrap the entire web platform"""
        print("🚀 Bootstrapping STEPPPS Web Platform...")

        self.bootstrap(source="genesis")

        # Create main canvas
        main_canvas = WebCanvas("main_canvas",
                               width=self.canvas_width,
                               height=self.canvas_height)
        self.add_child(main_canvas)

        # Create toolbar
        toolbar = WebToolbar("toolbar")
        self.add_child(toolbar)

        # Create demo objects
        main_canvas.create_demo_objects()

        print("✅ Web Platform bootstrapped successfully!")
        return True

    def render_to_canvas(self):
        """Platform renders its children"""
        commands = []
        for child in self.children:
            if child.visible:
                commands.extend(child.render_to_canvas())
        return commands

    def get_platform_state(self):
        """Get full platform state for web client"""
        return {
            "platform": {
                "id": self.id,
                "frame": self.frame_count,
                "timestamp": time.time(),
                "canvas_size": {"width": self.canvas_width, "height": self.canvas_height}
            },
            "objects": self.to_canvas_json(),
            "render_commands": self.render_to_canvas() if self.dirty else []
        }

    def handle_canvas_event(self, event_data):
        """Handle events from canvas (clicks, etc.)"""
        event_type = event_data.get("type")
        x, y = event_data.get("x", 0), event_data.get("y", 0)

        if event_type == "click":
            # Find clicked object
            clicked_obj = self.find_object_at_position(x, y)
            if clicked_obj:
                clicked_obj.handle_click(x, y)
                self.dirty = True
                return {"status": "handled", "object": clicked_obj.id}

        return {"status": "unhandled"}

    def find_object_at_position(self, x, y):
        """Find STEPPPS object at canvas coordinates"""
        return self._find_recursive(self, x, y)

    def _find_recursive(self, obj, x, y):
        """Recursively find object at position"""
        # Check children first (top layer)
        for child in reversed(obj.children):  # Reverse for top-to-bottom hit testing
            found = self._find_recursive(child, x, y)
            if found:
                return found

        # Check if this object contains the point
        if hasattr(obj, 'contains_point') and obj.contains_point(x, y):
            return obj

        return None

class WebCanvas(STEPPPS):
    """Main canvas object for rendering"""

    def __init__(self, id, width=800, height=600):
        super().__init__(id, "canvas")
        self.data["space"]["width"] = width
        self.data["space"]["height"] = height
        self.bootstrap(source="llm")

    def render_to_canvas(self):
        """Canvas renders background and children"""
        commands = [
            {
                "type": "clear",
                "color": self.data["pixel"]["color"]
            }
        ]

        # Render all children
        for child in self.children:
            if child.visible:
                commands.extend(child.render_to_canvas())

        return commands

    def create_demo_objects(self):
        """Create demo STEPPPS objects on canvas"""
        # Create pixel grid
        for row in range(5):
            for col in range(8):
                x = 50 + col * 60
                y = 50 + row * 60
                pixel = WebPixel(f"pixel_{row}_{col}", x, y, 40, 40)
                self.add_child(pixel)

        # Create interactive shapes
        button = WebButton("demo_button", 500, 100, 120, 40, "Bootstrap!")
        self.add_child(button)

        shape = WebShape("demo_shape", 650, 200, 100, 80, "circle")
        self.add_child(shape)

        # Create text display
        text = WebText("demo_text", 50, 400, "STEPPPS Web Platform")
        self.add_child(text)

class WebPixel(STEPPPS):
    """Individual pixel as STEPPPS object"""

    def __init__(self, id, x, y, width=20, height=20):
        super().__init__(id, "pixel")
        self.data["space"].update({"x": x, "y": y, "width": width, "height": height})
        self.bootstrap(source="llm")

    def render_to_canvas(self):
        """Render pixel to canvas"""
        space = self.data["space"]
        pixel = self.data["pixel"]

        return [{
            "type": "rect",
            "x": space["x"],
            "y": space["y"],
            "width": space["width"],
            "height": space["height"],
            "fill": pixel["color"],
            "alpha": pixel["alpha"],
            "stroke": "#2c3e50",
            "strokeWidth": 1
        }]

    def contains_point(self, x, y):
        """Check if point is inside this pixel"""
        space = self.data["space"]
        return (space["x"] <= x <= space["x"] + space["width"] and
                space["y"] <= y <= space["y"] + space["height"])

    def handle_click(self, x, y):
        """Handle pixel click - cycle colors"""
        colors = ["#e74c3c", "#2ecc71", "#3498db", "#f39c12", "#9b59b6", "#1abc9c"]
        current_color = self.data["pixel"]["color"]

        try:
            idx = colors.index(current_color)
            new_color = colors[(idx + 1) % len(colors)]
        except ValueError:
            new_color = colors[0]

        self.update_state(**{
            "pixel.color": new_color,
            "event.lastEvent": "clicked",
            "psychology.energy": min(1.0, self.data["psychology"]["energy"] + 0.1)
        })

        print(f"🎯 Pixel {self.id} clicked - new color: {new_color}")

class WebButton(STEPPPS):
    """Button as STEPPPS object"""

    def __init__(self, id, x, y, width, height, text):
        super().__init__(id, "button")
        self.data["space"].update({"x": x, "y": y, "width": width, "height": height})
        self.text = text
        self.bootstrap(source="llm")

    def render_to_canvas(self):
        """Render button to canvas"""
        space = self.data["space"]
        pixel = self.data["pixel"]

        return [{
            "type": "rect",
            "x": space["x"],
            "y": space["y"],
            "width": space["width"],
            "height": space["height"],
            "fill": pixel["color"],
            "alpha": pixel["alpha"],
            "stroke": "#2c3e50",
            "strokeWidth": 2,
            "borderRadius": 5
        }, {
            "type": "text",
            "x": space["x"] + space["width"] / 2,
            "y": space["y"] + space["height"] / 2,
            "text": self.text,
            "fill": "#ffffff",
            "font": "16px Arial",
            "textAlign": "center",
            "textBaseline": "middle"
        }]

    def contains_point(self, x, y):
        space = self.data["space"]
        return (space["x"] <= x <= space["x"] + space["width"] and
                space["y"] <= y <= space["y"] + space["height"])

    def handle_click(self, x, y):
        """Handle button click - trigger platform bootstrap"""
        self.update_state(**{
            "event.lastEvent": "clicked",
            "psychology.energy": 1.0
        })

        # Trigger platform re-bootstrap
        if self.platform_ref:
            self.platform_ref.bootstrap_platform()

        print(f"🔄 Button {self.id} clicked - triggering platform bootstrap")

class WebShape(STEPPPS):
    """Generic shape as STEPPPS object"""

    def __init__(self, id, x, y, width, height, shape_type="rect"):
        super().__init__(id, "shape")
        self.data["space"].update({"x": x, "y": y, "width": width, "height": height})
        self.shape_type = shape_type
        self.bootstrap(source="llm")

    def render_to_canvas(self):
        """Render shape to canvas"""
        space = self.data["space"]
        pixel = self.data["pixel"]

        if self.shape_type == "circle":
            return [{
                "type": "circle",
                "x": space["x"] + space["width"] / 2,
                "y": space["y"] + space["height"] / 2,
                "radius": min(space["width"], space["height"]) / 2,
                "fill": pixel["color"],
                "alpha": pixel["alpha"],
                "stroke": "#2c3e50",
                "strokeWidth": 2
            }]
        else:
            return [{
                "type": "rect",
                "x": space["x"],
                "y": space["y"],
                "width": space["width"],
                "height": space["height"],
                "fill": pixel["color"],
                "alpha": pixel["alpha"],
                "stroke": "#2c3e50",
                "strokeWidth": 2
            }]

    def contains_point(self, x, y):
        space = self.data["space"]
        if self.shape_type == "circle":
            cx = space["x"] + space["width"] / 2
            cy = space["y"] + space["height"] / 2
            radius = min(space["width"], space["height"]) / 2
            dx, dy = x - cx, y - cy
            return dx * dx + dy * dy <= radius * radius
        else:
            return (space["x"] <= x <= space["x"] + space["width"] and
                    space["y"] <= y <= space["y"] + space["height"])

    def handle_click(self, x, y):
        """Handle shape click - morph shape"""
        shapes = ["rect", "circle"]
        current_idx = shapes.index(self.shape_type)
        self.shape_type = shapes[(current_idx + 1) % len(shapes)]

        self.update_state(**{
            "event.lastEvent": "morphed",
            "psychology.emotion": "fluid"
        })

        print(f"🔄 Shape {self.id} morphed to: {self.shape_type}")

class WebText(STEPPPS):
    """Text display as STEPPPS object"""

    def __init__(self, id, x, y, text):
        super().__init__(id, "text")
        self.data["space"].update({"x": x, "y": y})
        self.text = text
        self.bootstrap(source="llm")

    def render_to_canvas(self):
        """Render text to canvas"""
        space = self.data["space"]
        pixel = self.data["pixel"]

        return [{
            "type": "text",
            "x": space["x"],
            "y": space["y"],
            "text": self.text,
            "fill": pixel["color"],
            "font": "24px Arial",
            "textAlign": "left",
            "textBaseline": "top"
        }]

class WebToolbar(STEPPPS):
    """Toolbar as STEPPPS object"""

    def __init__(self, id):
        super().__init__(id, "toolbar")
        self.data["space"].update({"x": 0, "y": 0, "width": 1200, "height": 40})

    def render_to_canvas(self):
        """Render toolbar"""
        space = self.data["space"]

        return [{
            "type": "rect",
            "x": space["x"],
            "y": space["y"],
            "width": space["width"],
            "height": space["height"],
            "fill": "#34495e",
            "alpha": 1.0
        }, {
            "type": "text",
            "x": 10,
            "y": 25,
            "text": "STEPPPS Web Platform - Click objects to interact",
            "fill": "#ecf0f1",
            "font": "14px Arial",
            "textAlign": "left",
            "textBaseline": "middle"
        }]

# HTTP Server for Web Platform
class WebPlatformHandler(http.server.BaseHTTPRequestHandler):
    platform_instance = None

    def do_GET(self):
        if self.path == "/" or self.path == "/index.html":
            self.serve_html_file()
        elif self.path == "/platform.js":
            self.serve_js_file()
        elif self.path == "/api/platform/state":
            self.send_json(self.platform_instance.get_platform_state())
        elif self.path == "/api/platform/bootstrap":
            self.platform_instance.bootstrap_platform()
            self.send_json({"status": "bootstrapped"})
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        if self.path == "/api/canvas/event":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            event_data = json.loads(post_data.decode('utf-8'))

            result = self.platform_instance.handle_canvas_event(event_data)
            self.send_json(result)
        else:
            self.send_response(404)
            self.end_headers()

    def serve_html_file(self):
        html_content = self.generate_html()
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(html_content.encode())

    def serve_js_file(self):
        js_content = self.generate_js()
        self.send_response(200)
        self.send_header("Content-type", "application/javascript")
        self.end_headers()
        self.wfile.write(js_content.encode())

    def generate_html(self):
        return '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>STEPPPS Web Platform</title>
    <style>
        body {
            margin: 0;
            padding: 20px;
            background: #2c3e50;
            color: #ecf0f1;
            font-family: Arial, sans-serif;
        }

        #platform-container {
            max-width: 1200px;
            margin: 0 auto;
        }

        h1 {
            text-align: center;
            color: #3498db;
            margin-bottom: 20px;
        }

        #steppps-canvas {
            border: 2px solid #3498db;
            background: #ecf0f1;
            display: block;
            margin: 0 auto;
            cursor: pointer;
        }

        #controls {
            margin-top: 20px;
            text-align: center;
        }

        button {
            background: #3498db;
            color: white;
            border: none;
            padding: 10px 20px;
            margin: 5px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
        }

        button:hover {
            background: #2980b9;
        }

        #info {
            margin-top: 20px;
            padding: 15px;
            background: #34495e;
            border-radius: 5px;
        }

        .status {
            color: #2ecc71;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div id="platform-container">
        <h1>🚀 STEPPPS Self-Bootstrapping Web Platform</h1>
        <p style="text-align: center;">Every canvas element IS a STEPPPS object. Click to interact!</p>

        <canvas id="steppps-canvas" width="1200" height="800"></canvas>

        <div id="controls">
            <button onclick="bootstrapPlatform()">🔄 Bootstrap Platform</button>
            <button onclick="refreshCanvas()">🎨 Refresh Canvas</button>
            <button onclick="showPlatformJSON()">📄 Show JSON</button>
            <button onclick="toggleAnimation()">▶️ Toggle Animation</button>
        </div>

        <div id="info">
            <div>Status: <span id="status" class="status">Loading...</span></div>
            <div>Frame: <span id="frame">0</span></div>
            <div>Objects: <span id="object-count">0</span></div>
            <div>Last Event: <span id="last-event">None</span></div>
        </div>
    </div>

    <script src="/platform.js"></script>
</body>
</html>'''

    def generate_js(self):
        return '''// STEPPPS Web Platform JavaScript
class STEPPPSWebPlatform {
    constructor() {
        this.canvas = document.getElementById('steppps-canvas');
        this.ctx = this.canvas.getContext('2d');
        this.animating = false;
        this.frameCount = 0;

        this.canvas.addEventListener('click', (e) => this.handleCanvasClick(e));
        this.startRenderLoop();
        this.updateStatus();
    }

    async handleCanvasClick(event) {
        const rect = this.canvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;

        try {
            const response = await fetch('/api/canvas/event', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({type: 'click', x: x, y: y})
            });

            const result = await response.json();
            document.getElementById('last-event').textContent =
                result.status === 'handled' ? `Clicked ${result.object}` : 'Click not handled';

            this.refreshCanvas();
        } catch (error) {
            console.error('Error handling click:', error);
        }
    }

    async fetchPlatformState() {
        try {
            const response = await fetch('/api/platform/state');
            return await response.json();
        } catch (error) {
            console.error('Error fetching platform state:', error);
            return null;
        }
    }

    async renderCanvas() {
        const state = await this.fetchPlatformState();
        if (!state) return;

        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        // Execute render commands
        const commands = state.render_commands || [];
        commands.forEach(cmd => this.executeRenderCommand(cmd));

        // Update UI
        document.getElementById('frame').textContent = state.platform.frame;
        this.frameCount = state.platform.frame;

        this.updateObjectCount(state.objects);
    }

    executeRenderCommand(cmd) {
        this.ctx.save();

        if (cmd.alpha !== undefined) {
            this.ctx.globalAlpha = cmd.alpha;
        }

        switch (cmd.type) {
            case 'clear':
                this.ctx.fillStyle = cmd.color || '#ecf0f1';
                this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
                break;

            case 'rect':
                this.ctx.fillStyle = cmd.fill || '#000';
                this.ctx.fillRect(cmd.x, cmd.y, cmd.width, cmd.height);

                if (cmd.stroke) {
                    this.ctx.strokeStyle = cmd.stroke;
                    this.ctx.lineWidth = cmd.strokeWidth || 1;
                    this.ctx.strokeRect(cmd.x, cmd.y, cmd.width, cmd.height);
                }
                break;

            case 'circle':
                this.ctx.beginPath();
                this.ctx.arc(cmd.x, cmd.y, cmd.radius, 0, 2 * Math.PI);
                this.ctx.fillStyle = cmd.fill || '#000';
                this.ctx.fill();

                if (cmd.stroke) {
                    this.ctx.strokeStyle = cmd.stroke;
                    this.ctx.lineWidth = cmd.strokeWidth || 1;
                    this.ctx.stroke();
                }
                break;

            case 'text':
                this.ctx.fillStyle = cmd.fill || '#000';
                this.ctx.font = cmd.font || '16px Arial';
                this.ctx.textAlign = cmd.textAlign || 'left';
                this.ctx.textBaseline = cmd.textBaseline || 'top';
                this.ctx.fillText(cmd.text, cmd.x, cmd.y);
                break;
        }

        this.ctx.restore();
    }

    updateObjectCount(objects) {
        let count = 0;
        const countRecursive = (obj) => {
            count++;
            if (obj.children) {
                obj.children.forEach(countRecursive);
            }
        };
        countRecursive(objects);
        document.getElementById('object-count').textContent = count;
    }

    startRenderLoop() {
        const loop = () => {
            if (this.animating || this.frameCount === 0) {
                this.renderCanvas();
            }
            requestAnimationFrame(loop);
        };
        loop();
    }

    async updateStatus() {
        try {
            const state = await this.fetchPlatformState();
            document.getElementById('status').textContent = state ? 'Active' : 'Error';
        } catch (error) {
            document.getElementById('status').textContent = 'Error';
        }

        setTimeout(() => this.updateStatus(), 1000);
    }
}

// Global functions for buttons
async function bootstrapPlatform() {
    try {
        await fetch('/api/platform/bootstrap');
        document.getElementById('last-event').textContent = 'Platform bootstrapped';
        platform.refreshCanvas();
    } catch (error) {
        console.error('Bootstrap error:', error);
    }
}

function refreshCanvas() {
    platform.renderCanvas();
}

async function showPlatformJSON() {
    const state = await platform.fetchPlatformState();
    console.log('Platform State:', state);
    alert('Platform JSON logged to console (F12)');
}

function toggleAnimation() {
    platform.animating = !platform.animating;
    const btn = event.target;
    btn.textContent = platform.animating ? '⏸️ Stop Animation' : '▶️ Start Animation';
}

// Initialize platform when page loads
let platform;
window.addEventListener('load', () => {
    platform = new STEPPPSWebPlatform();
});'''

    def send_json(self, data):
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode())

    def log_message(self, format, *args):
        pass  # Suppress HTTP server logs

def run_web_server(port=8888):
    """Run web server for STEPPPS platform"""
    httpd = socketserver.TCPServer(("", port), WebPlatformHandler)
    print(f"🌐 STEPPPS Web Platform running on http://localhost:{port}")
    threading.Thread(target=httpd.serve_forever, daemon=True).start()
    return httpd

def main():
    """Main function to start the web platform"""
    print("🚀 Starting STEPPPS Web Platform...")

    # Create and bootstrap platform
    platform = WebPlatform()
    WebPlatformHandler.platform_instance = platform

    platform.bootstrap_platform()

    # Start web server
    server = run_web_server()

    # Open browser
    webbrowser.open("http://localhost:8888")

    print("✅ Platform running! Check your browser.")
    print("🎯 Click canvas objects to interact with STEPPPS!")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        print("👋 STEPPPS Web Platform stopped")

if __name__ == "__main__":
    main()