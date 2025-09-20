#!/usr/bin/env python3
import json
import os
import shlex
import shutil
import subprocess
import time
from datetime import datetime

LOG = "/var/log/tbos/mic.jsonl"
RF2S_LOG = "/var/log/tbos/rf2s.jsonl"
RF2S_BACKING = "/var/lib/rf2s"


def mic_record_raw(path: str, seconds: float, rate: int) -> bool:
    if shutil.which("arecord"):
        cmd = f"arecord -q -f S16_LE -c 1 -r {rate} -d {seconds} {shlex.quote(path)}"
        return subprocess.call(cmd, shell=True) == 0
    if shutil.which("sox"):
        cmd = f"sox -q -d -b 16 -c 1 -r {rate} {shlex.quote(path)} trim 0 {seconds}"
        return subprocess.call(cmd, shell=True) == 0
    return False


def dominant_freq(raw_path: str, rate: int) -> float:
    try:
        with open(raw_path, "rb") as f:
            data = f.read()
    except Exception:
        return 0.0
    import struct, math
    N = len(data)//2
    if N == 0:
        return 0.0
    xs = struct.unpack("<"+"h"*N, data)
    sig = [x/32768.0 for x in xs]
    best_f, best_p = 0.0, 0.0
    for freq in range(100, 4001, 50):
        k = int(0.5 + (N*freq)/rate)
        omega = 2.0*math.pi*k/N
        coef = 2.0*math.cos(omega)
        s1 = s2 = 0.0
        for x in sig:
            s = x + coef*s1 - s2
            s2, s1 = s1, s
        p = s2*s2 + s1*s1 - coef*s1*s2
        if p > best_p:
            best_p, best_f = p, float(freq)
    return best_f


def rf2s_write(raw_path: str, freq_hz: float):
    mhz = freq_hz/1e6
    d = f"{mhz:.6f}"
    dest_dir = os.path.join(RF2S_BACKING, d)
    os.makedirs(dest_dir, exist_ok=True)
    dest = os.path.join(dest_dir, f"audio_{int(time.time())}.raw")
    try:
        shutil.copyfile(raw_path, dest)
        with open(RF2S_LOG, 'a') as f:
            f.write(json.dumps({"ts": datetime.utcnow().isoformat()+"Z", "freq_mhz": mhz, "file": dest})+"\n")
    except Exception:
        pass


def main():
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    os.makedirs(RF2S_BACKING, exist_ok=True)
    interval = float(os.environ.get("TBOS_MIC_INTERVAL", "5"))
    seconds = float(os.environ.get("TBOS_MIC_WINDOW", "1.0"))
    rate = int(os.environ.get("TBOS_MIC_RATE", "16000"))
    bridge = os.environ.get("TBOS_MIC_BRIDGE_RF2S", "1") == "1"
    raw = "/tmp/mic.raw"
    while True:
        ok = mic_record_raw(raw, seconds, rate)
        ts = datetime.utcnow().isoformat()+"Z"
        freq = dominant_freq(raw, rate) if ok else 0.0
        with open(LOG, 'a') as f:
            f.write(json.dumps({"ts": ts, "rate": rate, "seconds": seconds, "dominant_hz": freq})+"\n")
        if bridge and freq > 0:
            rf2s_write(raw, freq)
        time.sleep(interval)

if __name__ == '__main__':
    main()
