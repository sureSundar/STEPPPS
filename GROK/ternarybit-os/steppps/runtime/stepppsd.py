#!/usr/bin/env python3
import json
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse, parse_qs
import sys
import os
from datetime import datetime

LOG = "/var/log/tbos/steppps.jsonl"

FRAMES = {}


def log_event(ev):
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    with open(LOG, "a") as f:
        f.write(json.dumps(ev) + "\n")


class H(BaseHTTPRequestHandler):
    def _send(self, code, obj):
        data = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        u = urlparse(self.path)
        if u.path == "/frames":
            self._send(200, {"ids": list(FRAMES.keys())})
            return
        if u.path.startswith("/frames/"):
            fid = u.path.split("/", 2)[2]
            self._send(200, FRAMES.get(fid, {}))
            return
        self._send(404, {"error": "not found"})

    def do_POST(self):
        u = urlparse(self.path)
        n = int(self.headers.get('Content-Length', '0') or '0')
        body = self.rfile.read(n).decode()
        try:
            payload = json.loads(body) if body else {}
        except Exception:
            payload = {}
        if u.path == "/frames":
            frame = payload
            fid = frame.get('meta', {}).get('id') or str(len(FRAMES)+1)
            FRAMES[fid] = frame
            log_event({"ts": datetime.utcnow().isoformat()+"Z", "type": "frame", "id": fid})
            self._send(200, {"ok": True, "id": fid})
            return
        if u.path == "/transform":
            # passthrough to tools logic: for now, echo payload
            self._send(200, {"ok": True, "result": payload})
            return
        self._send(404, {"error": "not found"})


def main():
    addr = ("127.0.0.1", 8080)
    httpd = HTTPServer(addr, H)
    print(f"stepppsd listening on {addr[0]}:{addr[1]}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
