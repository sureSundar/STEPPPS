from abc import ABC, abstractmethod
import json
import http.server
import socketserver
import threading
import requests
import time
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
    def render(self):
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
                    print("Warning: steppps_config.json not found, using default config")
                    self.data.update(DEFAULT_CONFIG)
            elif source == "network":
                response = requests.get(payload_url)
                response.raise_for_status()
                self.data.update(response.json())
            elif source == "llm":
                llm_response = self.simulate_llm(llm_prompt)  # Mock for simplicity
                # Use Ollama: requests.post("http://localhost:11434/api/generate", json={"model": "llama3.1", "prompt": llm_prompt})
                self.data.update(json.loads(llm_response))
            self.data["event"]["lastEvent"] = "bootstrapped"
            self.orchestrate_children()
        except Exception as e:
            print(f"Bootstrap failed: {e}")
            self.data.update(DEFAULT_CONFIG)  # Fallback

    def simulate_llm(self, prompt):
        """Mock LLM for PoC (replace with Ollama/xAI API)."""
        return json.dumps({"pixel": {"color": "#00FF00"}, "prompt": {"text": prompt or "Generated state"}})

    def orchestrate_children(self):
        """Propagate bootstrap to children (Scalable, SOLID: Dependency inversion)."""
        for child in self.children:
            child.bootstrap(source="parent", llm_prompt=self.data["prompt"]["text"])

    def to_json(self):
        """Communicable unit (KISS: Simple serialization)."""
        return json.dumps(self.data)

    def from_json(self, json_str):
        """Load from payload."""
        self.data.update(json.loads(json_str))

    def communicate(self, target_url, payload):
        """Global orchestration over HTTPS (Responsible: Auth header)."""
        try:
            headers = {"Authorization": "Bearer token"}  # Add auth for production
            response = requests.post(target_url, json=payload, headers=headers)
            if response.status_code == 200:
                self.from_json(response.text)
            else:
                print(f"Communication failed: {response.status_code}")
        except Exception as e:
            print(f"Communication error: {e}")

class Pixel(STEPPPS):
    def render(self):
        """Concrete pixel rendering (e.g., for canvas or display)."""
        print(f"Rendering pixel {self.id} at {self.data['space']} with color {self.data['pixel']['color']}")

class IoTDevice(STEPPPS):
    def __init__(self, id, sensor_type, **kwargs):
        super().__init__(id, **kwargs)
        self.sensor_type = sensor_type

    def render(self):
        """IoT metric output."""
        print(f"{self.sensor_type} metric: {self.data['event']['lastEvent']}")

    def get_metric(self):
        """Simulate sensor data."""
        return {"metric": self.sensor_type, "value": "sample_data"}

class STEPPPSHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        """Handle IoT endpoints (e.g., /monitor/camera/)."""
        if self.path.startswith("/monitor/"):
            sensor = self.path.split("/")[-2]
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
        root.from_json(payload[0])  # Process first object
        self.send_response(200)
        self.end_headers()
        self.wfile.write(json.dumps([root.to_json()]).encode())

def run_server(port=9001):
    """Embedded HTTP server (HTTPS for production)."""
    httpd = socketserver.TCPServer(("", port), STEPPPSHandler)
    print(f"Server running on http://localhost:{port}")
    threading.Thread(target=httpd.serve_forever).start()

if __name__ == "__main__":
    run_server()
    root = Pixel("root", 0, 0)
    root.bootstrap(source="storage")  # Use steppps_config.json or default
    child = Pixel("child1", 1, 1)
    root.children.append(child)
    root.orchestrate_children()
    root.render()
    # Simulate global comm: root.communicate("https://remote-device.com/monitor/camera/", [root.to_json()])
