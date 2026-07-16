#!/usr/bin/env python3
import json
import os
import shlex
import subprocess
import time
from datetime import datetime

LOG = "/var/log/tbos/wifi.jsonl"


def run(cmd: str, timeout=10) -> str:
    try:
        return subprocess.check_output(shlex.split(cmd), stderr=subprocess.STDOUT, timeout=timeout).decode()
    except Exception:
        return ""


def scan_once():
    devs = run("iw dev")
    iface = None
    for line in devs.splitlines():
        line = line.strip()
        if line.startswith("Interface "):
            iface = line.split()[1]
            break
    if not iface:
        return None
    out = run(f"iw dev {iface} scan", timeout=15)
    nets = []
    cur = {}
    for ln in out.splitlines():
        s = ln.strip()
        if s.startswith("BSS "):
            if cur:
                nets.append(cur)
                cur = {}
        elif s.startswith("SSID:"):
            cur['ssid'] = s.split("SSID:",1)[1].strip()
        elif s.startswith("signal:"):
            cur['signal'] = s.split("signal:",1)[1].strip()
    if cur:
        nets.append(cur)
    return {"iface": iface, "count": len(nets), "nets": nets[:50]}


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    interval = int(os.environ.get("TBOS_WIFI_INTERVAL", "30"))
    while True:
        ts = datetime.utcnow().isoformat()+"Z"
        data = scan_once()
        if data:
            data['ts'] = ts
            with open(LOG, 'a') as f:
                f.write(json.dumps(data) + "\n")
        time.sleep(interval)

if __name__ == '__main__':
    main()
