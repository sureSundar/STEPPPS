#!/usr/bin/env python3
import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
from datetime import datetime
from math import cos

LOGDIR = "/var/log/tbos"
RF2S_BACKING = "/var/lib/rf2s"
PF2S_BACKING = "/var/lib/pf2s"


def log_json(fname: str, obj: dict):
    os.makedirs(LOGDIR, exist_ok=True)
    with open(os.path.join(LOGDIR, fname), "a") as f:
        f.write(json.dumps(obj) + "\n")


def run(cmd: str, timeout: int = 10) -> str:
    try:
        out = subprocess.check_output(shlex.split(cmd), stderr=subprocess.STDOUT, timeout=timeout)
        return out.decode(errors="ignore")
    except Exception as e:
        return ""


# ---------------- WiFi -----------------

def wifi_scan(args):
    # detect interface via `iw dev`
    devs = run("iw dev")
    m = re.findall(r"\n\s*Interface\s+(\S+)", devs)
    if not m:
        print("No WiFi interfaces found (need 'iw')", file=sys.stderr)
        sys.exit(1)
    iface = args.iface or m[0]
    out = run(f"iw dev {iface} scan", timeout=15)
    networks = []
    ssid = None
    for line in out.splitlines():
        line = line.strip()
        if line.startswith("SSID:"):
            ssid = line.split("SSID:",1)[1].strip()
        if line.startswith("signal:"):
            sig = line.split("signal:",1)[1].strip()
            if ssid:
                networks.append({"ssid": ssid, "signal": sig})
                ssid = None
    ts = datetime.utcnow().isoformat()+"Z"
    log_json("wifi.jsonl", {"ts": ts, "iface": iface, "networks": networks})
    for n in networks[:20]:
        print(f"{n['ssid']:<32} {n['signal']}")
    if len(networks) > 20:
        print(f"... and {len(networks)-20} more")


# ---------------- Bluetooth -----------------

def bt_scan(args):
    # Prefer bluetoothctl
    cmd = "bluetoothctl --timeout 6 scan on"
    out = run(cmd, timeout=8)
    devs = []
    for line in out.splitlines():
        # Example: [NEW] Device AA:BB:CC:DD:EE:FF Name
        m = re.search(r"Device\s+([0-9A-F:]{17})\s+(.*)$", line)
        if m:
            devs.append({"addr": m.group(1), "name": m.group(2).strip()})
    ts = datetime.utcnow().isoformat()+"Z"
    log_json("bt.jsonl", {"ts": ts, "devices": devs})
    for d in devs:
        print(f"{d['addr']} {d['name']}")


# ---------------- Microphone -----------------

def mic_record_raw(path: str, seconds: int = 1, rate: int = 16000):
    # arecord preferred
    if shutil.which("arecord"):
        cmd = f"arecord -q -f S16_LE -c 1 -r {rate} -d {seconds} {shlex.quote(path)}"
        subprocess.call(cmd, shell=True)
        return rate
    # sox fallback
    if shutil.which("sox"):
        cmd = f"sox -q -d -b 16 -c 1 -r {rate} {shlex.quote(path)} trim 0 {seconds}"
        subprocess.call(cmd, shell=True)
        return rate
    print("arecord/sox not available", file=sys.stderr)
    sys.exit(1)


def mic_dominant_freq(raw_path: str, rate: int) -> float:
    # Simple Goertzel-like scan over 100..4000 Hz with 50 Hz step to avoid numpy dependency
    try:
        with open(raw_path, "rb") as f:
            data = f.read()
    except Exception:
        return 0.0
    import struct
    # Convert little-endian 16-bit signed
    N = len(data)//2
    if N == 0:
        return 0.0
    samples = struct.unpack("<"+"h"*N, data)
    # Normalize to float [-1,1]
    sig = [x/32768.0 for x in samples]
    best_freq = 0.0
    best_power = 0.0
    # Windowing light: rectangular
    for freq in range(100, 4001, 50):
        # Compute bin power via Goertzel
        # k = 0.5 + N*freq/rate ; omega = 2*pi*k/N ; cosine = 2*cos(omega)
        k = int(0.5 + (N*freq)/rate)
        omega = 2.0*3.141592653589793*k/N
        coef = 2.0*cos(omega)
        s_prev = 0.0
        s_prev2 = 0.0
        for x in sig:
            s = x + coef*s_prev - s_prev2
            s_prev2 = s_prev
            s_prev = s
        power = s_prev2*s_prev2 + s_prev*s_prev - coef*s_prev*s_prev2
        if power > best_power:
            best_power = power
            best_freq = float(freq)
    return best_freq


def mic_analyze(args):
    os.makedirs("/tmp", exist_ok=True)
    raw = "/tmp/mic.raw"
    rate = mic_record_raw(raw, seconds=args.seconds, rate=args.rate)
    freq = mic_dominant_freq(raw, rate)
    ts = datetime.utcnow().isoformat()+"Z"
    log_json("mic.jsonl", {"ts": ts, "rate": rate, "seconds": args.seconds, "dominant_hz": freq})
    print(f"Dominant frequency: {freq:.1f} Hz (rate {rate})")
    if args.bridge_rf2s and freq > 0:
        rf2s_bridge(raw, freq)


def rf2s_bridge(raw_path: str, freq_hz: float):
    # Write to RF2S backing structure
    unit = "MHz"
    mhz = freq_hz/1e6
    if unit == "MHz":
        d = f"{mhz:.6f}"
    else:
        d = f"{mhz:.6f}"
    dest_dir = os.path.join(RF2S_BACKING, d)
    os.makedirs(dest_dir, exist_ok=True)
    dest = os.path.join(dest_dir, "audio.raw")
    try:
        shutil.copyfile(raw_path, dest)
        print(f"RF2S: wrote {dest}")
        log_json("rf2s.jsonl", {"ts": datetime.utcnow().isoformat()+"Z", "freq_mhz": mhz, "file": dest})
    except Exception as e:
        print(f"RF2S bridge failed: {e}", file=sys.stderr)

##########################
# Optic (PF2S) utilities  #
##########################

def pf2s_normalize_nm(val: float) -> str:
    return f"{val:.6f}"

def pf2s_resolve(vpath: str) -> str:
    # vpath format: pf2s://<lambda_nm>/<name>
    if vpath.startswith('pf2s://'):
        rest = vpath[len('pf2s://'):]
        parts = rest.split('/', 1)
        lam = pf2s_normalize_nm(float(parts[0]))
        name = parts[1] if len(parts) > 1 else 'spectrum.json'
        mp = os.path.join('/mnt/pf', lam)
        bp = os.path.join(PF2S_BACKING, lam)
        base = mp if os.path.exists(mp) else bp
        os.makedirs(base, exist_ok=True)
        return os.path.join(base, name)
    # fallback to local path
    d = os.path.dirname(vpath)
    if d:
        os.makedirs(d, exist_ok=True)
    return vpath

def optic_scan(args):
    # Generate a synthetic spectrum over range with optional Gaussian peaks
    try:
        rng = args.range.split('-') if args.range else ['400','700']
        start = float(rng[0]); end = float(rng[1])
    except Exception:
        start, end = 400.0, 700.0
    points = int(args.points)
    center = float(args.center)
    out = args.out or f"pf2s://{center}/spectrum.json"
    import math, random
    xs = [start + (end-start)*i/(points-1) for i in range(points)]
    # baseline noise
    ys = [ -50.0 + random.uniform(-2,2) for _ in xs ]
    # add up to 3 peaks near center and +/- offsets
    peaks = [center, center-5, center+7]
    for pk in peaks:
        if pk < start or pk > end:
            continue
        for i,x in enumerate(xs):
            ys[i] += -5.0 * math.exp(-0.5 * ((x-pk)/0.4)**2)
    spectrum = {"range_nm": [start, end], "points": points,
                "samples": [{"lambda_nm": round(xs[i], 4), "power_dbm": round(ys[i], 3)} for i in range(points)]}
    # write to PF2S
    path = pf2s_resolve(out)
    with open(path, 'w') as f:
        json.dump(spectrum, f, indent=2)
    ts = datetime.utcnow().isoformat()+"Z"
    log_json('optic.jsonl', {"ts": ts, "out": out, "range_nm": [start,end], "points": points})
    print(f"PF2S: wrote {path}")

def optic_modulate(args):
    # Stub: record intended modulation parameters into PF2S and log
    try:
        center = float(args.center)
    except Exception:
        center = 550.0
    out = args.out or f"pf2s://{center}/modulate.json"
    desc = {
        'pattern': args.pattern,
        'rate_hz': float(args.rate),
        'duration_s': float(args.duration),
        'area': args.area,
        'color_rgb': [int(x) for x in args.color.split(',')] if args.color else [255,255,255]
    }
    path = pf2s_resolve(out)
    with open(path, 'w') as f:
        json.dump(desc, f, indent=2)
    ts = datetime.utcnow().isoformat()+"Z"
    log_json('optic.jsonl', {"ts": ts, "modulate": desc, "out": out})
    print(f"PF2S: wrote modulation descriptor {path}")

def optic_receive(args):
    # Stub: create a capture artifact under PF2S with synthetic decode summary
    try:
        center = float(args.center)
    except Exception:
        center = 550.0
    out = args.out or f"pf2s://{center}/capture.json"
    capture = {
        'duration_s': float(args.duration),
        'decoded': {'ok': True, 'bits': '10110011', 'ber': 0.01},
    }
    path = pf2s_resolve(out)
    with open(path, 'w') as f:
        json.dump(capture, f, indent=2)
    ts = datetime.utcnow().isoformat()+"Z"
    log_json('optic.jsonl', {"ts": ts, "receive": {'duration_s': args.duration}, "out": out})
    print(f"PF2S: wrote capture {path}")


# ---------------- SDR (stubs + policy) -----------------

def load_radio_policy():
    p = "/etc/tbos/radio-policy.json"
    try:
        with open(p) as f:
            return json.load(f)
    except Exception:
        return {"tx_enabled": False, "whitelist": []}


def sdr_tx(args):
    pol = load_radio_policy()
    if not pol.get("tx_enabled") or not args.i_accept_regulatory_risk:
        print("TX disabled by policy or missing consent flag", file=sys.stderr)
        sys.exit(1)
    print("SDR TX not implemented in this stub. Integrate hackrf_transfer or UHD as needed.")


def sdr_rx(args):n    print("SDR RX not implemented in this stub. Integrate rtl_sdr or hackrf_transfer as needed.")


# ---------------- Main -----------------

def main():
    ap = argparse.ArgumentParser(prog="tbos-sensorctl", description="TBOS Sensor Control Plane")
    sub = ap.add_subparsers(dest="domain")

    # wifi
    ap_wifi = sub.add_parser("wifi", help="WiFi controls")
    subw = ap_wifi.add_subparsers(dest="cmd")
    ap_wifi_scan = subw.add_parser("scan", help="Scan APs")
    ap_wifi_scan.add_argument("--iface")
    ap_wifi_scan.set_defaults(func=wifi_scan)

    # bt
    ap_bt = sub.add_parser("bt", help="Bluetooth controls")
    subb = ap_bt.add_subparsers(dest="cmd")
    ap_bt_scan = subb.add_parser("scan", help="Scan BT devices")
    ap_bt_scan.set_defaults(func=bt_scan)

    # mic
    ap_mic = sub.add_parser("mic", help="Microphone")
    subm = ap_mic.add_subparsers(dest="cmd")
    ap_mic_an = subm.add_parser("analyze", help="Record and analyze dominant frequency")
    ap_mic_an.add_argument("--seconds", type=int, default=1)
    ap_mic_an.add_argument("--rate", type=int, default=16000)
    ap_mic_an.add_argument("--bridge-rf2s", action="store_true")
    ap_mic_an.set_defaults(func=mic_analyze)

    # sdr
    ap_sdr = sub.add_parser("sdr", help="SDR")
    subs = ap_sdr.add_subparsers(dest="cmd")
    ap_sdr_tx = subs.add_parser("tx", help="Transmit (stub; gated by policy)")
    ap_sdr_tx.add_argument("--i-accept-regulatory-risk", action="store_true")
    ap_sdr_tx.set_defaults(func=sdr_tx)
    ap_sdr_rx = subs.add_parser("rx", help="Receive (stub)")
    ap_sdr_rx.set_defaults(func=sdr_rx)

    # optic
    ap_opt = sub.add_parser("optic", help="Optics/Photonics")
    subo = ap_opt.add_subparsers(dest="cmd")
    ap_opt_scan = subo.add_parser("scan", help="Generate or acquire a spectrum and write to PF2S")
    ap_opt_scan.add_argument("--range", default="400-700", help="nm range, e.g., 400-700")
    ap_opt_scan.add_argument("--points", default="128")
    ap_opt_scan.add_argument("--center", default="550.0", help="nm; used for PF2S out and synthetic peaks")
    ap_opt_scan.add_argument("--out", help="pf2s://<lambda_nm>/spectrum.json or local path")
    ap_opt_scan.set_defaults(func=optic_scan)
    ap_opt_mod = subo.add_parser("modulate", help="Stub: write modulation descriptor to PF2S")
    ap_opt_mod.add_argument("--pattern", default="pwm", help="am|pwm|ofdm (stub)")
    ap_opt_mod.add_argument("--rate", default="100.0")
    ap_opt_mod.add_argument("--duration", default="2.0")
    ap_opt_mod.add_argument("--area", default="0,0,100,100", help="x,y,w,h")
    ap_opt_mod.add_argument("--color", default="255,255,255")
    ap_opt_mod.add_argument("--center", default="550.0")
    ap_opt_mod.add_argument("--out", help="pf2s://<lambda_nm>/modulate.json")
    ap_opt_mod.set_defaults(func=optic_modulate)

    ap_opt_recv = subo.add_parser("receive", help="Stub: write synthetic capture to PF2S")
    ap_opt_recv.add_argument("--duration", default="2.0")
    ap_opt_recv.add_argument("--center", default="550.0")
    ap_opt_recv.add_argument("--out", help="pf2s://<lambda_nm>/capture.json")
    ap_opt_recv.set_defaults(func=optic_receive)

    args = ap.parse_args()
    if not args.domain:
        ap.print_help(); sys.exit(1)
    if not hasattr(args, "func"):
        print("Missing command", file=sys.stderr); sys.exit(1)
    args.func(args)

if __name__ == "__main__":
    main()
