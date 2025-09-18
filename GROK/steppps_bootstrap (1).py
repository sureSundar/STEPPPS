from abc import ABC, abstractmethod
import json
import http.server
import socketserver
import threading
import requests
import time
import tkinter as tk
import webbrowser
import os

# Default JSON payload if steppps_config.json is missing
DEFAULT_CONFIG = {
    "space": {"x": 0, "y": 0},
    "temporal": {"timestamp": time.time()},
    "event": {"lastEvent": "init"},
    "psychology": {"emotion": "neutral", "confidence": 0.5},
    "pixel": {"color": "#FFFFFF"},
    "prompt": {"text": "Initialize a neutral pixel state"},
    "script": {"callback": None}
}

class STEPPPS(ABC):
    """Abstract base class for STEPPPS objects (SOLID: Single responsibility)."""
    def __init__(self, id, space=None, temporal=None, event=None, psychology=None, pixel=None, prompt=None, script=None):
        self.id = id
        self.data = {
            "space": space or {"x": 0, "y": 0},
            "temporal": temporal or {"timestamp": time.time()},
            "event": event or {"lastEvent": None},
            "psychology": psychology or {"emotion": "neutral", "confidence": 0.5},
            "pixel": pixel or {"color": "#FFFFFF"},
            "prompt": prompt or {"text": ""},
            "script": script or {"callback": None}
        }
        self.children = []

    @abstractmethod
    def render(self, canvas=None):
        """Concrete subclasses implement rendering (Open-Closed)."""
        pass

    def bootstrap(self, source="storage", payload_url=None, llm_prompt=None):
        """Bootstraps from source (DRY: Unified logic)."""
        try:
            if source == "storage":
                if os.path.exists("steppps_config.json"):
                    with open("steppps_config.json", "r") as f:
                        self.data.update(json.load(f))
                else:
                    print("Warning: steppps_config.json not found, using default")
                    self.data.update(DEFAULT_CONFIG)
            elif source == "network":
                response = requests.get(payload_url)
                response.raise_for_status()
                self.data.update(response.json())
            elif source == "llm":
                llm_response = self.simulate_llm(llm_prompt)
                # Replace with: requests.post("http://localhost:11434/api/generate", json={"model": "llama3.1", "prompt": llm_prompt})
                self.data.update(json.loads(llm_response))
            elif source == "parent":
                self.data["prompt"]["text"] = llm_prompt or "Inherited state"
                self.data["pixel"]["color"] = "#00FF00"  # Mock inherited state
            self.data["event"]["lastEvent"] = "bootstrapped"
            self.orchestrate_children()
        except Exception as e:
            print(f"Bootstrap failed: {e}")
            self.data.update(DEFAULT_CONFIG)

    def simulate_llm(self, prompt):
        """Mock LLM for PoC (replace with Ollama/xAI API)."""
        color = "#00FF00" if "sunset" in (prompt or "").lower() else "#FFFFFF"
        return json.dumps({"pixel": {"color": color}, "prompt": {"text": prompt or "Generated state"}})

    def orchestrate_children(self):
        """Propagate bootstrap (Scalable, SOLID: Dependency inversion)."""
        for child in self.children:
            child.bootstrap(source="parent", llm_prompt=self.data["prompt"]["text"])

    def to_json(self):
        """Communicable unit (KISS: Simple serialization)."""
        return json.dumps(self.data)

    def from_json(self, json_str):
        """Load from payload."""
        self.data.update(json.loads(json_str))

    def communicate(self, target_url, payload):
        """Global orchestration simulation (Responsible: Mock auth)."""
        print(f"Simulating POST to {target_url} with payload: {payload}")
        # In production: requests.post(target_url, json=payload, headers={"Authorization": "Bearer token"})
        return {"status": 200, "response": payload}  # Mock response

class Pixel(STEPPPS):
    def render(self, canvas=None):
        """Render pixel to console or Tkinter canvas."""
        print(f"Rendering pixel {self.id} at {self.data['space']} with color {self.data['pixel']['color']}")
        if canvas:
            canvas.create_rectangle(
                self.data["space"]["x"] * 10, self.data["space"]["y"] * 10,
                (self.data["space"]["x"] + 1) * 10, (self.data["space"]["y"] + 1) * 10,
                fill=self.data["pixel"]["color"]
            )

class IoTDevice(STEPPPS):
    def __init__(self, id, sensor_type, **kwargs):
        super().__init__(id, **kwargs)
        self.sensor_type = sensor_type

    def render(self, canvas=None):
        """Render IoT metric."""
        metric = self.get_metric()
        print(f"{self.sensor_type} metric: {metric}")
        if canvas:
            canvas.create_text(50, 50, text=f"{self.sensor_type}: {metric['value']}", fill="#000000")

    def get_metric(self):
        """Simulate sensor data."""
        metrics = {
            "camera": "frame_data_640x480",
            "wifi": "signal_80%",
            "speaker": "volume_50%",
            "mic": "recording_level_medium",
            "bluetooth": "paired_headphones"
        }
        return {"metric": self.sensor_type, "value": metrics.get(self.sensor_type, "unknown")}

class STEPPPSHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        """Handle IoT/display endpoints."""
        if self.path.startswith("/monitor/"):
            sensor = self.path.split("/")[-2]
            if sensor == "browser":
                webbrowser.open(f"http://localhost:9001{self.path}")
                payload = [{"space": {"x": 0, "y": 0}, "prompt": {"text": "Browser page"}, "pixel": {"color": "#0000FF"}}]
            else:
                device = IoTDevice("iot1", sensor)
                payload = [device.to_json()]
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps(payload).encode())
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        """Handle incoming JSON arrays."""
        content_length = int(self.headers["Content-Length"])
        payload = json.loads(self.rfile.read(content_length))
        root = Pixel("root", 0, 0)
        root.from_json(payload[0])
        self.send_response(200)
        self.end_headers()
        self.wfile.write(json.dumps([root.to_json()]).encode())

def run_server(port=9001):
    """Embedded HTTP server (HTTPS for production)."""
    httpd = socketserver.TCPServer(("", port), STEPPPSHandler)
    print(f"Server running on http://localhost:{port}")
    threading.Thread(target=httpd.serve_forever, daemon=True).start()

def create_ui():
    """Tkinter UI for pixel rendering and sensor testing."""
    root = tk.Tk()
    root.title("STEPPPS Display")
    canvas = tk.Canvas(root, width=200, height=200, bg="white")
    canvas.pack()
    tk.Label(root, text="Enter LLM Prompt (e.g., 'sunset')").pack()
    entry = tk.Entry(root, width=30)
    entry.pack()
    tk.Button(root, text="Bootstrap & Render", command=lambda: bootstrap_and_render(canvas, entry.get())).pack()
    tk.Button(root, text="Test Sensor", command=lambda: test_sensor(canvas)).pack()
    root.mainloop()

def bootstrap_and_render(canvas, llm_prompt):
    """Bootstrap root and children, render to canvas."""
    root = Pixel("root", 0, 0)
    root.bootstrap(source="llm", llm_prompt=llm_prompt)
    for x in range(5):
        for y in range(5):
            child = Pixel(f"child_{x}_{y}", x, y)
            root.children.append(child)
    root.orchestrate_children()
    canvas.delete("all")
    root.render(canvas)
    for child in root.children:
        child.render(canvas)
    root.communicate("https://us-device.example.com/monitor/camera/", [root.to_json()])

def test_sensor(canvas):
    """Simulate sensor endpoint."""
    sensor = IoTDevice("sensor1", "camera")
    sensor.render(canvas)
    webbrowser.open("http://localhost:9001/monitor/camera/")

# Mock Java/JS interop (print equivalent code/output)
def simulate_other_languages():
    print("\nSimulated Java STEPPPS class:")
    print("""
public class STEPPPS {
    private Map<String, Object> data = new HashMap<>();
    public STEPPPS(String id) { data.put("pixel", Map.of("color", "#FFFFFF")); }
    public void bootstrap(String source) {
        System.out.println("Bootstrapping " + id + " from " + source);
        data.put("pixel", Map.of("color", "#00FF00"));
    }
    public void render() { System.out.println("Rendering: Color " + ((Map) data.get("pixel")).get("color")); }
}
Output: Bootstrapping root_java from storage; Rendering: Color #00FF00
    """)
    print("\nSimulated JavaScript STEPPPS class:")
    print("""
class STEPPPS {
  constructor(id) { this.data = { pixel: { color: '#FFFFFF' } }; }
  bootstrap(source) { console.log(`Bootstrapping ${this.id} from ${source}`); this.data.pixel.color = '#00FF00'; }
  render() { console.log(`Rendering ${this.id}: Color ${this.data.pixel.color}`); }
}
Output: Bootstrapping root_js from storage; Rendering root_js: Color #00FF00
    """)

if __name__ == "__main__":
    run_server()
    simulate_other_languages()
    create_ui()
