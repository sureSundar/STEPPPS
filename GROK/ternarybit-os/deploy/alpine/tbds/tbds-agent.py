#!/usr/bin/env python3
import argparse
import json
import os
import sys
import time
import threading
import serial  # pyserial
from datetime import datetime

LOGDIR = "/var/log/tbos"
FEATURES = "/etc/tbos/features.conf"
POLICY = "/etc/tbos/tbds-policy.json"


def now():
    return datetime.utcnow().isoformat() + "Z"


def ensure_dirs():
    os.makedirs(LOGDIR, exist_ok=True)


def read_policy():
    try:
        with open(POLICY) as f:
            return json.load(f)
    except Exception:
        return {"allow_exec": False, "allow_put": False, "allow_get": ["features"]}


def read_features():
    try:
        with open(FEATURES) as f:
            return f.read()
    except Exception:
        return ""


def writer_loop(ser, stop_evt):
    # Send hello once, then heartbeats
    hello = {"ts": now(), "type": "hello", "data": {"role": "tbos", "version": "0.1", "arch": os.uname().machine}}
    ser.write((json.dumps(hello) + "\n").encode())
    ser.flush()
    while not stop_evt.is_set():
        hb = {"ts": now(), "type": "hb", "data": {"uptime": int(time.time())}}
        try:
            ser.write((json.dumps(hb) + "\n").encode())
            ser.flush()
        except Exception:
            pass
        stop_evt.wait(5.0)


def handle_cmd(obj, ser):
    pol = read_policy()
    t = obj.get("type")
    data = obj.get("data", {})
    if t != "cmd":
        return
    name = data.get("name")
    rid = obj.get("seq")

    def reply(ok: bool, payload):
        ser.write((json.dumps({"ts": now(), "type": "resp", "ok": ok, "seq": rid, "data": payload}) + "\n").encode())
        ser.flush()

    if name == "get":
        key = data.get("key")
        if key == "features" and ("features" in pol.get("allow_get", [])):
            reply(True, {"content": read_features()})
        else:
            reply(False, {"error": "not allowed or unknown key"})
        return

    # Future: put, exec – gated by policy
    reply(False, {"error": "unsupported command"})


def reader_loop(ser, stop_evt):
    buf = b""
    while not stop_evt.is_set():
        try:
            line = ser.readline()
            if not line:
                time.sleep(0.1); continue
            try:
                obj = json.loads(line.decode(errors="ignore").strip())
                handle_cmd(obj, ser)
            except Exception:
                # ignore malformed
                pass
        except Exception:
            time.sleep(0.5)


def open_serial(dev: str, baud: int):
    return serial.Serial(dev, baudrate=baud, timeout=1)


def parse_tbds_arg(arg: str):
    # formats: serial,ttyS0,115200
    parts = arg.split(",")
    if len(parts) >= 3 and parts[0] == "serial":
        return {"type": "serial", "dev": f"/dev/{parts[1]}", "baud": int(parts[2])}
    return None


def read_cmdline_tbds() -> str:
    try:
        with open("/proc/cmdline") as f:
            line = f.read().strip()
        for tok in line.split():
            if tok.startswith("tbds="):
                return tok.split("=",1)[1]
    except Exception:
        pass
    return ""


def main():
    ap = argparse.ArgumentParser(description="TBOS TBDS Agent")
    ap.add_argument("--tbds")  # e.g., serial,ttyS0,115200
    args = ap.parse_args()

    ensure_dirs()

    tbds = args.tbds or read_cmdline_tbds()
    if not tbds:
        print("No TBDS transport configured", file=sys.stderr)
        sys.exit(1)
    cfg = parse_tbds_arg(tbds)
    if not cfg or cfg["type"] != "serial":
        print("Unsupported TBDS transport", file=sys.stderr)
        sys.exit(1)

    try:
        ser = open_serial(cfg["dev"], cfg["baud"])
    except Exception as e:
        print(f"Failed to open TBDS serial: {e}", file=sys.stderr)
        sys.exit(1)

    stop_evt = threading.Event()
    tw = threading.Thread(target=writer_loop, args=(ser, stop_evt), daemon=True)
    tr = threading.Thread(target=reader_loop, args=(ser, stop_evt), daemon=True)
    tw.start(); tr.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        stop_evt.set()
        time.sleep(0.5)

if __name__ == "__main__":
    main()
