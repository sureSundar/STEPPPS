#!/usr/bin/env python3
import json
import os
import shlex
import subprocess
import time
from datetime import datetime

LOG = "/var/log/tbos/bt.jsonl"


def run(cmd: str, timeout=8) -> str:
    try:
        return subprocess.check_output(shlex.split(cmd), stderr=subprocess.STDOUT, timeout=timeout).decode()
    except Exception:
        return ""


def scan_once():
    # bluetoothctl scan on for a short period
    out = run("bluetoothctl --timeout 5 scan on", timeout=7)
    devs = []
    for line in out.splitlines():
        line = line.strip()
        if "Device" in line and ":" in line:
            # [NEW] Device AA:BB:CC:DD:EE:FF Name
            parts = line.split("Device ")
            if len(parts) > 1:
                rest = parts[1]
                toks = rest.split(" ")
                if len(toks) >= 1:
                    addr = toks[0]
                    name = " ".join(toks[1:]).strip()
                    devs.append({"addr": addr, "name": name})
    return {"count": len(devs), "devices": devs}


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    interval = int(os.environ.get("TBOS_BT_INTERVAL", "60"))
    while True:
        ts = datetime.utcnow().isoformat()+"Z"
        data = scan_once()
        data['ts'] = ts
        with open(LOG, 'a') as f:
            f.write(json.dumps(data) + "\n")
        time.sleep(interval)

if __name__ == '__main__':
    main()
