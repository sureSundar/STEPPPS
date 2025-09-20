#!/usr/bin/env python3
import json
import os
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse
from datetime import datetime
import subprocess

OFFLOAD_CFG = "/etc/tbos/offload-peers.json"
LOG = "/var/log/tbos/compute.jsonl"


def log(obj):
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    obj.setdefault('ts', datetime.utcnow().isoformat()+"Z")
    with open(LOG, 'a') as f:
        f.write(json.dumps(obj)+"\n")


def load_cfg():
    try:
        with open(OFFLOAD_CFG) as f:
            return json.load(f)
    except Exception:
        return {}


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
        if u.path == '/tbos-offload/job':
            body = self._json()
            job = body.get('job', {})
            key = body.get('key')
            cfg = load_cfg()
            expect = cfg.get('shared_key')
            if expect and key != expect:
                self._send(403, {'ok': False, 'error': 'bad key'})
                return
            # Build compute path
            try:
                op = job.get('op')
                inputs = job.get('inputs', {})
                params = job.get('params', {})
                out = job.get('out')
                if op == 'steppps.transform':
                    subop = params.get('op') or 'pixel.replace_color'
                    fromc = params.get('from', '0,0,255')
                    toc = params.get('to', '255,64,0')
                    frame = inputs.get('frame') or inputs.get('input')
                    path = f"pxfs://compute/steppps.transform/op={subop}/from={fromc}/to={toc}/input={frame}/out=out.json"
                elif op == 'steppps.llm_generate':
                    intent = params.get('intent', 'Generate minimal frame')
                    path = f"pxfs://compute/steppps.llm_generate/intent={intent}/input=none/out=frame.json"
                else:
                    self._send(400, {'ok': False, 'error': 'unsupported op'})
                    return
                args = ['tbos-compute', path]
                if out:
                    args += ['--link', out]
                cached = subprocess.check_output(args).decode().strip()
                res = {'ok': True, 'result_href': out or cached}
                log({'event':'offload.done', 'job': job.get('id'), 'href': res['result_href']})
                self._send(200, res)
            except Exception as e:
                log({'event':'offload.error', 'err': str(e)})
                self._send(500, {'ok': False, 'error': str(e)})
            return
        self._send(404, {'ok': False, 'error': 'not found'})


def main():
    addr = ('0.0.0.0', 9090)
    httpd = HTTPServer(addr, H)
    print(f'tbos-offloadd listening on {addr[0]}:{addr[1]}')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
