#!/usr/bin/env python3
import json
import os
import sys
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse
from datetime import datetime

# Config via env
LLM_ENDPOINT = os.environ.get('TBOS_LLM_ENDPOINT', 'http://127.0.0.1:8081')
RF2S_MOUNT = os.environ.get('TBOS_RF2S_MOUNT', '/mnt/rf')
RF2S_BACKING = os.environ.get('TBOS_RF2S_BACKING', '/var/lib/rf2s')
PXFS_MOUNT = os.environ.get('TBOS_PXFS_MOUNT', '/mnt/px')
PXFS_BACKING = os.environ.get('TBOS_PXFS_BACKING', '/var/lib/pxfs')
GBNF_PATH = os.path.join(os.path.dirname(__file__), 'steppps_json.gbnf')

# Optional jsonschema validate
try:
    import jsonschema
    SCHEMA_DIR = os.path.join(os.path.dirname(__file__), '..', '..', '..', 'steppps', 'schemas')
    FRAME_SCHEMA = json.load(open(os.path.join(SCHEMA_DIR, 'steppps_frame.json')))
    def validate_frame(obj):
        jsonschema.validate(instance=obj, schema=FRAME_SCHEMA)
except Exception:
    def validate_frame(obj):
        # Minimal fallback
        if not isinstance(obj, dict) or 'meta' not in obj or 'time' not in obj:
            raise ValueError('invalid frame')

# HTTP client (stdlib)
import urllib.request

def llama_generate(prompt: str, grammar: str|None=None, n_predict=256, temperature=0.6):
    payload = {
        'prompt': prompt,
        'n_predict': n_predict,
        'temperature': temperature,
    }
    if grammar:
        payload['grammar'] = grammar
    data = json.dumps(payload).encode()
    req = urllib.request.Request(LLM_ENDPOINT + '/completion', data=data, headers={'Content-Type': 'application/json'})
    with urllib.request.urlopen(req, timeout=60) as resp:
        out = resp.read().decode()
    try:
        jobj = json.loads(out)
        return jobj.get('content', out)
    except Exception:
        return out

# Virtual path resolver to RF2S/PXFS
# rf2s://<freq_mhz>/<name> -> prefer mounted /mnt/rf/<freq>/<name>, else /var/lib/rf2s/<freq>/<name>
# pxfs://<logical/segments> -> prefer /mnt/px/<path>, else /var/lib/pxfs using token encoding "__PX_255_255_255__"
SEP_TOKEN = '__PX_255_255_255__'

def resolve_path(vpath: str) -> str:
    if vpath.startswith('rf2s://'):
        rest = vpath[len('rf2s://'):]
        parts = rest.split('/', 1)
        freq = parts[0]
        name = parts[1] if len(parts) > 1 else 'out.json'
        p = os.path.join(RF2S_MOUNT, freq, name)
        if os.path.exists(os.path.join(RF2S_MOUNT, freq)):
            os.makedirs(os.path.dirname(p), exist_ok=True)
            return p
        # fallback backing
        p = os.path.join(RF2S_BACKING, freq, name)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        return p
    if vpath.startswith('pxfs://'):
        rest = vpath[len('pxfs://'):]
        # Try mounted path first
        p = os.path.join(PXFS_MOUNT, rest)
        d = os.path.dirname(p)
        try:
            os.makedirs(d, exist_ok=True)
            return p
        except Exception:
            pass
        # fallback to backing with encoding
        comps = [c for c in rest.split('/') if c]
        enc = SEP_TOKEN.join(comps)
        p = os.path.join(PXFS_BACKING, enc)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        return p
    # default: treat as local filesystem path
    d = os.path.dirname(vpath)
    if d:
        os.makedirs(d, exist_ok=True)
    return vpath

class H(BaseHTTPRequestHandler):
    def _json(self):
        n = int(self.headers.get('Content-Length','0') or '0')
        if n <= 0:
            return {}
        try:
            return json.loads(self.rfile.read(n).decode())
        except Exception:
            return {}

    def _send(self, code, obj):
        data = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header('Content-Type','application/json')
        self.send_header('Content-Length', str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self):
        u = urlparse(self.path)
        body = self._json()
        if u.path == '/generate':
            prompt = body.get('prompt','')
            use_grammar = body.get('use_grammar', False)
            grammar = open(GBNF_PATH).read() if use_grammar else None
            try:
                text = llama_generate(prompt, grammar=grammar)
                self._send(200, {'ok': True, 'text': text})
            except Exception as e:
                self._send(500, {'ok': False, 'error': str(e)})
            return
        if u.path == '/steppps':
            intent = body.get('intent','')
            out_path = body.get('out')  # rf2s://... or pxfs://...
            grammar = open(GBNF_PATH).read()
            try:
                text = llama_generate(intent, grammar=grammar)
            except Exception:
                # Fallback minimal valid frame
                ts = datetime.utcnow().isoformat()+"Z"
                text = json.dumps({
                    'meta': {'version':'0.1','profile':'edge'},
                    'time': {'ts': ts},
                    'events': [{'time': {'ts': ts}, 'kind': 'fallback', 'data': {'intent': intent}}]
                })
            # Parse + validate + optional save
            try:
                obj = json.loads(text)
                validate_frame(obj)
            except Exception as e:
                self._send(200, {'ok': False, 'error': f'validation: {e}', 'text': text})
                return
            if out_path:
                try:
                    p = resolve_path(out_path)
                    with open(p, 'w') as f:
                        f.write(json.dumps(obj, indent=2))
                except Exception as e:
                    self._send(200, {'ok': True, 'warning': f'could not write to {out_path}: {e}', 'frame': obj})
                    return
            self._send(200, {'ok': True, 'frame': obj})
            return
        self._send(404, {'ok': False, 'error': 'not found'})


def main():
    addr = ('127.0.0.1', 8088)
    httpd = HTTPServer(addr, H)
    print(f'tbos-llm-proxy listening on {addr[0]}:{addr[1]} -> {LLM_ENDPOINT}')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
