#!/usr/bin/env python3
import argparse
import hashlib
import json
import os
import shlex
import subprocess
from urllib.parse import urlparse, parse_qs

PX_MOUNT = os.environ.get('TBOS_PXFS_MOUNT', '/mnt/px')
PX_BACK = os.environ.get('TBOS_PXFS_BACKING', '/var/lib/pxfs')
RF_MOUNT = os.environ.get('TBOS_RF2S_MOUNT', '/mnt/rf')
RF_BACK = os.environ.get('TBOS_RF2S_BACKING', '/var/lib/rf2s')
LLM_PROXY = os.environ.get('TBOS_LLM_PROXY', 'http://127.0.0.1:8088')
LOG = '/var/log/tbos/compute.jsonl'

SEP_TOKEN = '__PX_255_255_255__'

def log(obj):
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    with open(LOG, 'a') as f:
        f.write(json.dumps(obj)+"\n")

def ensure_dir(p):
    d = os.path.dirname(p)
    if d:
        os.makedirs(d, exist_ok=True)

# Virtual path resolution

def resolve_px(path: str) -> str:
    p = os.path.join(PX_MOUNT, path)
    try:
        ensure_dir(p)
        return p
    except Exception:
        pass
    comps = [c for c in path.split('/') if c]
    enc = SEP_TOKEN.join(comps)
    p = os.path.join(PX_BACK, enc)
    ensure_dir(p)
    return p

def resolve_rf(freq_path: str) -> str:
    p = os.path.join(RF_MOUNT, freq_path)
    try:
        ensure_dir(p)
        return p
    except Exception:
        pass
    p = os.path.join(RF_BACK, freq_path)
    ensure_dir(p)
    return p

def resolve_vpath(vpath: str) -> str:
    if vpath.startswith('pxfs://'):
        return resolve_px(vpath[len('pxfs://'):])
    if vpath.startswith('rf2s://'):
        return resolve_rf(vpath[len('rf2s://'):])
    ensure_dir(vpath)
    return vpath

# Compute operations

def hash_key(op: str, inputs: dict, params: dict) -> str:
    h = hashlib.sha256()
    h.update(op.encode())
    h.update(json.dumps(inputs, sort_keys=True).encode())
    h.update(json.dumps(params, sort_keys=True).encode())
    return h.hexdigest()


def steppps_transform(inputs: dict, params: dict, out_path: str):
    frame = inputs.get('frame') or inputs.get('input')
    if not frame:
        raise ValueError('missing input frame')
    # Only support pixel.replace_color for v0
    subop = params.get('op') or params.get('subop') or 'pixel.replace_color'
    if subop != 'pixel.replace_color':
        raise ValueError('unsupported subop')
    from_color = params.get('from') or '0,0,255'
    to_color = params.get('to') or '255,64,0'
    cmd = f"steppps-tools transform pixel.replace_color {resolve_vpath(frame)} --from-color {shlex.quote(str(from_color))} --to-color {shlex.quote(str(to_color))}"
    # Capture output
    out = subprocess.check_output(cmd, shell=True)
    with open(out_path, 'wb') as f:
        f.write(out)


def steppps_llm_generate(params: dict, out_path: str):
    intent = params.get('intent') or 'Generate a minimal frame.'
    # Use local proxy
    data = json.dumps({'intent': intent, 'out': None}).encode()
    import urllib.request
    req = urllib.request.Request(LLM_PROXY + '/steppps', data=data, headers={'Content-Type':'application/json'})
    with urllib.request.urlopen(req, timeout=60) as resp:
        res = json.loads(resp.read().decode())
    if not res.get('ok'):
        raise RuntimeError('llm proxy error: ' + str(res))
    with open(out_path, 'w') as f:
        f.write(json.dumps(res.get('frame', {}), indent=2))


def materialize(px_compute_path: str) -> str:
    # Parse pxfs://compute/<op>/<kv-params>/input=<vpath>/out=<name>
    if px_compute_path.startswith('pxfs://'):
        rel = px_compute_path[len('pxfs://'):]
    else:
        rel = px_compute_path
    parts = [p for p in rel.split('/') if p]
    if len(parts) < 2 or parts[0] != 'compute':
        raise ValueError('path must start with pxfs://compute/...')
    op = parts[1]
    kv = {}
    inputs = {}
    out_name = 'out.json'
    for p in parts[2:]:
        if '=' in p:
            k, v = p.split('=', 1)
            if k == 'input' or k == 'frame':
                inputs['frame'] = v
            elif k == 'out':
                out_name = v
            else:
                kv[k] = v
    key = hash_key(op, inputs, kv)
    cache_dir = os.path.join(PX_BACK, 'compute', key)
    os.makedirs(cache_dir, exist_ok=True)
    cached = os.path.join(cache_dir, out_name)
    if os.path.exists(cached):
        return cached
    # Compute
    if op == 'steppps.transform':
        steppps_transform(inputs, kv, cached)
    elif op == 'steppps.llm_generate':
        steppps_llm_generate(kv, cached)
    elif op == 'spectrum.from_pixel':
        # Build local command invoking steppps-tools; expects input pixel JSON (pxfs:// or local)
        pixel = inputs.get('frame') or inputs.get('input')
        profile = kv.get('profile') or ''
        cmd = ['steppps-tools', 'transform', 'spectrum.from_pixel', resolve_vpath(pixel)]
        if profile:
            cmd += ['--profile', profile]
        out = subprocess.check_output(cmd)
        with open(cached, 'wb') as f:
            f.write(out)
    elif op == 'pixel.from_spectrum':
        spec = inputs.get('frame') or inputs.get('input')
        profile = kv.get('profile') or ''
        clip = kv.get('clip') or 'warn'
        cmd = ['steppps-tools', 'transform', 'pixel.from_spectrum', resolve_vpath(spec), '--clip', clip]
        if profile:
            cmd += ['--profile', profile]
        out = subprocess.check_output(cmd)
        with open(cached, 'wb') as f:
            f.write(out)
    elif op == 'spectrum.features':
        spec = inputs.get('frame') or inputs.get('input')
        max_peaks = kv.get('max-peaks') or kv.get('max_peaks') or '5'
        window = kv.get('window') or '3'
        mode = kv.get('mode') or 'minima'
        cmd = ['steppps-tools', 'transform', 'spectrum.features', resolve_vpath(spec),
               '--max-peaks', str(max_peaks), '--window', str(window), '--mode', str(mode)]
        out = subprocess.check_output(cmd)
        with open(cached, 'wb') as f:
            f.write(out)
    else:
        raise ValueError('unsupported op: ' + op)
    return cached


def main():
    ap = argparse.ArgumentParser(prog='tbos-compute', description='Materialize a PXFS compute path into cached output')
    ap.add_argument('path', help='pxfs://compute/... path')
    ap.add_argument('--link', help='Optional target vpath (rf2s:// or pxfs:// or local) to copy result')
    args = ap.parse_args()
    try:
        cached = materialize(args.path)
        if args.link:
            target = resolve_vpath(args.link)
            with open(cached, 'rb') as src, open(target, 'wb') as dst:
                dst.write(src.read())
            print(target)
        else:
            print(cached)
    except Exception as e:
        log({'err': str(e), 'path': args.path})
        raise

if __name__ == '__main__':
    main()
