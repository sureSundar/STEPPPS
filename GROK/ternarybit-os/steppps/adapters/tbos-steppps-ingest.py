#!/usr/bin/env python3
import time
import json
import os
import sys
import threading
import urllib.request
from datetime import datetime

LOGDIR = "/var/log/tbos"
API = "http://127.0.0.1:8080"

files = {
    "wifi": os.path.join(LOGDIR, "wifi.jsonl"),
    "bt": os.path.join(LOGDIR, "bt.jsonl"),
    "mic": os.path.join(LOGDIR, "mic.jsonl"),
    "rf2s": os.path.join(LOGDIR, "rf2s.jsonl"),
}


def http_post(path, obj):
    data = json.dumps(obj).encode()
    req = urllib.request.Request(API + path, data=data, headers={'Content-Type': 'application/json'})
    with urllib.request.urlopen(req, timeout=2) as resp:
        resp.read()


def make_frame(kind: str, payload: dict):
    ts = datetime.utcnow().isoformat()+"Z"
    frame = {
        "meta": {"version": "0.1", "profile": "edge"},
        "time": {"ts": ts},
        "events": [
            {"time": {"ts": ts}, "kind": f"sensor.{kind}", "data": payload}
        ]
    }
    return frame


def tail_file(path, cb):
    try:
        with open(path, 'r') as f:
            f.seek(0, os.SEEK_END)
            while True:
                line = f.readline()
                if not line:
                    time.sleep(0.5)
                    continue
                try:
                    obj = json.loads(line)
                    cb(obj)
                except Exception:
                    pass
    except FileNotFoundError:
        # create directory and wait
        os.makedirs(os.path.dirname(path), exist_ok=True)
        while not os.path.exists(path):
            time.sleep(1)
        return tail_file(path, cb)


def run_sensor(kind):
    path = files[kind]
    def cb(obj):
        try:
            frame = make_frame(kind, obj)
            http_post('/frames', frame)
        except Exception:
            pass
    tail_file(path, cb)


def main():
    threads = []
    for kind in files.keys():
        t = threading.Thread(target=run_sensor, args=(kind,), daemon=True)
        t.start(); threads.append(t)
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
