#!/usr/bin/env python3
import json
import os
import time
import glob
import shutil
from datetime import datetime
import urllib.request

LOG = "/var/log/tbos/compute.jsonl"
LOCAL_ONLY = os.environ.get('TBOS_JOBD_LOCAL_ONLY', '1') == '1'
INTERVAL = float(os.environ.get('TBOS_JOBD_INTERVAL', '5'))
JOBS_LOCAL = "/var/lib/tbos/jobs"
JOBS_PX = "/mnt/px/jobs"
OFFLOAD_CFG = "/etc/tbos/offload-peers.json"


def log(obj):
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    obj.setdefault('ts', datetime.utcnow().isoformat()+"Z")
    with open(LOG, 'a') as f:
        f.write(json.dumps(obj)+"\n")


def load_peers():
    try:
        with open(OFFLOAD_CFG) as f:
            cfg = json.load(f)
        return cfg.get('peers', []), cfg.get('shared_key')
    except Exception:
        return [], None


def offload_job(job):
    peers, key = load_peers()
    if not peers:
        return False
    data = json.dumps({'job': job, 'key': key}).encode()
    for p in peers:
        try:
            req = urllib.request.Request(p.rstrip('/') + '/tbos-offload/job', data=data, headers={'Content-Type':'application/json'})
            with urllib.request.urlopen(req, timeout=5) as resp:
                res = json.loads(resp.read().decode())
            if res.get('ok'):
                log({'event':'offload.sent', 'peer': p, 'job': job.get('id')})
                return True
        except Exception as e:
            log({'event':'offload.error', 'peer': p, 'err': str(e)})
    return False


def claim(path):
    lock = path + '.lock'
    try:
        fd = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        os.close(fd)
        return True
    except FileExistsError:
        return False


def finish(path, status: str, result_href: str|None, info: dict|None=None):
    try:
        with open(path) as f:
            job = json.load(f)
    except Exception:
        job = {}
    job['status'] = status
    if result_href:
        job['result_href'] = result_href
    if info:
        job['info'] = info
    tmp = path + '.tmp'
    with open(tmp, 'w') as f:
        f.write(json.dumps(job, indent=2))
    os.replace(tmp, path)
    open(path + '.done', 'w').close()


def materialize(job):
    # Execute via tbos-compute
    from subprocess import check_output
    op = job.get('op')
    inputs = job.get('inputs', {})
    params = job.get('params', {})
    out = job.get('out')
    # Build pxfs compute path
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
        raise ValueError('unsupported op: ' + str(op))
    args = ['tbos-compute', path]
    if out:
        args += ['--link', out]
    cached = check_output(args).decode().strip()
    return out or cached


def process_job(path):
    if not claim(path):
        return
    try:
        with open(path) as f:
            job = json.load(f)
    except Exception as e:
        log({'event':'job.invalid', 'path': path, 'err': str(e)})
        return
    jid = job.get('id') or os.path.basename(path)
    # Offload logic
    if not LOCAL_ONLY:
        persona = open('/run/tbos/shell_persona').read().strip() if os.path.exists('/run/tbos/shell_persona') else ''
        if persona in ('calc','tiny'):
            if offload_job(job):
                finish(path, 'offloaded', None, {'peer':'sent'})
                return
    try:
        href = materialize(job)
        finish(path, 'done', href)
        log({'event':'job.done', 'job': jid, 'href': href})
    except Exception as e:
        finish(path, 'error', None, {'err': str(e)})
        log({'event':'job.error', 'job': jid, 'err': str(e)})


def scan_dir(d):
    if not os.path.exists(d):
        return
    for path in glob.glob(os.path.join(d, '*.json')):
        if os.path.exists(path + '.done'):
            continue
        process_job(path)


def main():
    os.makedirs(JOBS_LOCAL, exist_ok=True)
    while True:
        scan_dir(JOBS_LOCAL)
        scan_dir(JOBS_PX)
        time.sleep(INTERVAL)

if __name__ == '__main__':
    main()
