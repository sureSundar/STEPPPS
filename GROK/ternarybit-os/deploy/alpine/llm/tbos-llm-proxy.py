#!/usr/bin/env python3
import json
import hashlib
import os
import sys
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse
from datetime import datetime

# Config via env
LLM_ENDPOINT = os.environ.get('TBOS_LLM_ENDPOINT', 'http://127.0.0.1:8081')
LLM_PROVIDER = os.environ.get('TBOS_LLM_PROVIDER', 'llama').lower()
OPENAI_BASE_URL = os.environ.get('OPENAI_BASE_URL', 'https://api.openai.com/v1').rstrip('/')
OPENAI_MODEL = os.environ.get('OPENAI_MODEL', 'gpt-5.4-mini')
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


def openai_generate(prompt: str, n_predict=512, model: str=''):
    api_key = os.environ.get('OPENAI_API_KEY', '')
    if not api_key:
        raise RuntimeError('OPENAI_API_KEY is not set')

    payload = {
        'model': model or OPENAI_MODEL,
        'input': prompt,
        'max_output_tokens': n_predict,
    }
    req = urllib.request.Request(
        OPENAI_BASE_URL + '/responses',
        data=json.dumps(payload).encode(),
        headers={
            'Authorization': 'Bearer ' + api_key,
            'Content-Type': 'application/json',
        },
    )
    with urllib.request.urlopen(req, timeout=90) as resp:
        result = json.loads(resp.read().decode())

    # SDKs expose output_text as a convenience; raw HTTP responses contain
    # message content items, so support both representations.
    if isinstance(result.get('output_text'), str):
        return result['output_text']
    texts = []
    for item in result.get('output', []):
        if item.get('type') != 'message':
            continue
        for content in item.get('content', []):
            if content.get('type') == 'output_text' and isinstance(content.get('text'), str):
                texts.append(content['text'])
    if texts:
        return '\n'.join(texts)
    raise RuntimeError('OpenAI response contained no output text')


def mock_generate(prompt: str, grammar: str|None=None):
    """Return deterministic offline output for pipeline and device testing."""
    digest = hashlib.sha256(prompt.encode()).hexdigest()[:12]
    if grammar:
        ts = '1970-01-01T00:00:00Z'
        return json.dumps({
            'meta': {'version': '0.1', 'profile': 'mock'},
            'time': {'ts': ts},
            'events': [{
                'time': {'ts': ts},
                'kind': 'mock-generation',
                'data': {'intent': prompt, 'digest': digest},
            }],
        })
    return f'TBOS mock response [{digest}]: prompt accepted for offline execution.'


def generate_text(prompt: str, grammar: str|None=None, n_predict=256,
                  temperature=0.6, model: str=''):
    if LLM_PROVIDER == 'mock':
        return mock_generate(prompt, grammar=grammar)
    if LLM_PROVIDER == 'openai':
        if grammar:
            raise RuntimeError('grammar-constrained generation is not yet available for OpenAI provider')
        return openai_generate(prompt, n_predict=n_predict, model=model)
    if LLM_PROVIDER == 'llama':
        return llama_generate(prompt, grammar=grammar, n_predict=n_predict,
                              temperature=temperature)
    raise RuntimeError(f'unsupported TBOS_LLM_PROVIDER: {LLM_PROVIDER}')


def extract_steppps_prompt(entity: dict) -> dict:
    """Normalize v2, v1, and dev-action Prompt dimensions."""
    prompt = entity.get('prompt') or entity.get('P_prompt') or {}
    if not isinstance(prompt, dict):
        raise ValueError('Prompt dimension must be an object')

    system = prompt.get('system', '')
    user = (
        prompt.get('user')
        or prompt.get('text')
        or prompt.get('creation_prompt')
        or ''
    )
    next_prompt = prompt.get('next') or prompt.get('next_genai_prompt') or ''
    interaction = prompt.get('interaction', '')
    model = prompt.get('model', '')
    temperature = prompt.get('temperature', 0.6)

    if not user and next_prompt:
        user = next_prompt
    if not user:
        raise ValueError('Prompt dimension has no user text')
    if not isinstance(temperature, (int, float)) or not 0 <= temperature <= 2:
        raise ValueError('Prompt temperature must be between 0 and 2')

    return {
        'system': str(system),
        'user': str(user),
        'next': str(next_prompt),
        'interaction': str(interaction),
        'model': str(model),
        'temperature': float(temperature),
    }


def compose_interaction_prompt(entity: dict, prompt: dict) -> str:
    parts = []
    entity_id = entity.get('id', '')
    if isinstance(entity_id, dict):
        entity_id = entity_id.get('uri', '')
    if entity_id:
        parts.append(f'STEPPPS entity: {entity_id}')
    if prompt['system']:
        parts.append(f'System instruction:\n{prompt["system"]}')
    if prompt['interaction']:
        parts.append(f'Interaction contract:\n{prompt["interaction"]}')
    parts.append(f'User request:\n{prompt["user"]}')
    if prompt['next']:
        parts.append(f'Continuation context:\n{prompt["next"]}')
    return '\n\n'.join(parts)

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

    def do_GET(self):
        u = urlparse(self.path)
        if u.path == '/health':
            backend_ok = False
            backend_error = ''
            if LLM_PROVIDER == 'mock':
                backend_ok = True
            elif LLM_PROVIDER == 'openai':
                backend_ok = bool(os.environ.get('OPENAI_API_KEY'))
                if not backend_ok:
                    backend_error = 'OPENAI_API_KEY is not set'
            else:
                try:
                    with urllib.request.urlopen(LLM_ENDPOINT + '/health', timeout=2) as resp:
                        backend_ok = 200 <= resp.status < 300
                except Exception as e:
                    backend_error = str(e)
            self._send(200, {
                'ok': True,
                'proxy': 'ready',
                'provider': LLM_PROVIDER,
                'backend': 'ready' if backend_ok else 'unavailable',
                'backend_endpoint': (
                    'built-in://mock' if LLM_PROVIDER == 'mock'
                    else LLM_ENDPOINT if LLM_PROVIDER == 'llama'
                    else OPENAI_BASE_URL
                ),
                'backend_error': backend_error,
            })
            return
        self._send(404, {'ok': False, 'error': 'not found'})

    def do_POST(self):
        u = urlparse(self.path)
        body = self._json()
        if u.path == '/generate':
            prompt = body.get('prompt','')
            use_grammar = body.get('use_grammar', False)
            grammar = open(GBNF_PATH).read() if use_grammar else None
            try:
                text = generate_text(prompt, grammar=grammar)
                self._send(200, {'ok': True, 'text': text})
            except Exception as e:
                self._send(500, {'ok': False, 'error': str(e)})
            return
        if u.path == '/interact':
            entity = body.get('entity', body)
            if not isinstance(entity, dict):
                self._send(400, {'ok': False, 'error': 'entity must be an object'})
                return
            try:
                prompt = extract_steppps_prompt(entity)
                request_text = compose_interaction_prompt(entity, prompt)
                text = generate_text(
                    request_text,
                    n_predict=int(body.get('n_predict', 512)),
                    temperature=prompt['temperature'],
                    model=prompt['model'],
                )
                entity_id = entity.get('id', '')
                if isinstance(entity_id, dict):
                    entity_id = entity_id.get('uri', '')
                self._send(200, {
                    'ok': True,
                    'entity_id': entity_id,
                    'model': prompt['model'],
                    'text': text,
                    'next_prompt': prompt['next'],
                })
            except ValueError as e:
                self._send(400, {'ok': False, 'error': str(e)})
            except Exception as e:
                self._send(502, {'ok': False, 'error': str(e)})
            return
        if u.path == '/steppps':
            intent = body.get('intent','')
            out_path = body.get('out')  # rf2s://... or pxfs://...
            grammar = open(GBNF_PATH).read()
            try:
                text = generate_text(intent, grammar=grammar)
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
    host = os.environ.get('TBOS_LLM_PROXY_HOST', '127.0.0.1')
    port = int(os.environ.get('TBOS_LLM_PROXY_PORT', '8088'))
    addr = (host, port)
    httpd = HTTPServer(addr, H)
    print(f'tbos-llm-proxy listening on {addr[0]}:{addr[1]} provider={LLM_PROVIDER}')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
