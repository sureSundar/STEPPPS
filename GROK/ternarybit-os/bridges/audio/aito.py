#!/usr/bin/env python3
"""
AITO: AI-Tuned Orchestrator for TBOS Audio Bridge

Minimal control plane using MQTT (optional) to advertise and discover audio streams.
Graceful if paho-mqtt is not installed. No auto-install. No destructive actions.

Usage examples:
  # Advertise a stream (from the sender host)
  python3 aito.py advertise --id lobby --group 239.255.0.1 --port 5004 --codec opus --bitrate 128k

  # Monitor/discover streams (on any host)
  python3 aito.py monitor

Environment variables:
  AITO_MQTT_URL (default: mqtt://localhost:1883)
  AITO_MQTT_USER, AITO_MQTT_PASS (optional)

Topics:
  tbos/audio/streams/<id> (retained JSON advert)

By default, this module does not start media processes automatically; it emits and reads adverts.
However, the 'autotune' subcommand can spawn receiver processes for matching stream adverts.
"""
import argparse
import json
import os
import sys
import time
from dataclasses import dataclass, asdict
from typing import Optional, Dict
import re
import subprocess
import signal
import threading
import socket

try:
    import paho.mqtt.client as mqtt  # type: ignore
except Exception:
    mqtt = None  # graceful degradation


def parse_mqtt_url(url: str):
    # very small parser for mqtt://host:port
    if not url.startswith("mqtt://"):
        raise ValueError("MQTT URL must start with mqtt://")
    rest = url[len("mqtt://"):]
    if ":" in rest:
        host, port = rest.split(":", 1)
        return host, int(port)
    return rest, 1883


@dataclass
class StreamAdvert:
    id: str
    group: str
    port: int
    codec: str = "opus"
    bitrate: str = "128k"
    channels: int = 2
    rate: int = 48000
    ts: float = time.time()


def get_client():
    if mqtt is None:
        print("WARNING: paho-mqtt not installed. AITO control plane disabled.")
        return None
    url = os.environ.get("AITO_MQTT_URL", "mqtt://localhost:1883")
    user = os.environ.get("AITO_MQTT_USER")
    pw = os.environ.get("AITO_MQTT_PASS")
    host, port = parse_mqtt_url(url)
    client = mqtt.Client()
    if user:
        client.username_pw_set(user, pw)
    client.connect(host, port, keepalive=30)
    return client


def advertise(args):
    client = get_client()
    advert = StreamAdvert(
        id=args.id,
        group=args.group,
        port=args.port,
        codec=args.codec,
        bitrate=args.bitrate,
        channels=1 if args.mono else 2,
        rate=48000,
        ts=time.time(),
    )
    payload = json.dumps(asdict(advert))
    topic = f"tbos/audio/streams/{args.id}"
    if client is None:
        print("AITO DRY-RUN (no MQTT): would publish")
        print(topic)
        print(payload)
        return 0
    client.loop_start()
    info = client.publish(topic, payload=payload, qos=1, retain=True)
    info.wait_for_publish()
    print(f"Advertised stream on {topic}")
    client.loop_stop()
    client.disconnect()
    return 0


def monitor(args):
    client = get_client()
    topic = "tbos/audio/streams/+"
    if client is None:
        print("AITO monitor requires paho-mqtt. Install paho-mqtt or set AITO_MQTT_URL to your broker.")
        return 1

    def on_connect(c, userdata, flags, rc):
        print("AITO connected, subscribing to", topic)
        c.subscribe(topic, qos=1)

    def on_message(c, userdata, msg):
        try:
            data = json.loads(msg.payload.decode("utf-8"))
        except Exception:
            data = {"raw": msg.payload.decode("utf-8", "ignore")}
        sid = msg.topic.split("/")[-1]
        print(f"[STREAM] id={sid} group={data.get('group')} port={data.get('port')} codec={data.get('codec')} bitrate={data.get('bitrate')}")

    # Optional heartbeat publisher
    hb_stop = threading.Event()

    def hb_loop():
        while not hb_stop.is_set():
            try:
                payload = {
                    "role": "aito",
                    "mode": "monitor",
                    "host": socket.gethostname(),
                    "pid": os.getpid(),
                    "ts": time.time(),
                }
                client.publish("tbos/audio/health/aito", json.dumps(payload), qos=0, retain=False)
            except Exception:
                pass
            hb_stop.wait(10)

    client.on_connect = on_connect
    client.on_message = on_message
    t = threading.Thread(target=hb_loop, daemon=True)
    t.start()
    try:
        client.loop_forever()
    finally:
        hb_stop.set()
    return 0


def autotune(args):
    client = get_client()
    topic = "tbos/audio/streams/+"
    if client is None:
        print("AITO autotune requires paho-mqtt. Install paho-mqtt or set AITO_MQTT_URL to your broker.")
        return 1

    id_re = re.compile(args.match) if args.match else None
    procs: Dict[str, subprocess.Popen] = {}
    last_payload: Dict[str, str] = {}

    receiver = args.receiver or os.path.join(os.path.dirname(__file__), "receiver.py")
    if not os.path.exists(receiver):
        print(f"ERROR: receiver not found at {receiver}. Pass --receiver <path> or ensure receiver.py exists.")
        return 1

    def spawn_or_respawn(sid: str, advert: dict):
        nonlocal procs, last_payload
        payload_str = json.dumps(advert, sort_keys=True)
        # Filter by id regex
        if id_re and not id_re.search(sid):
            # stop if running but no longer matches
            if sid in procs:
                stop_proc(sid)
            return
        # Dedup: if unchanged, do nothing
        if last_payload.get(sid) == payload_str:
            return
        # Restart if exists
        if sid in procs:
            stop_proc(sid)
        # Spawn
        group = advert.get("group")
        port = str(advert.get("port"))
        cmd = [sys.executable, receiver, "--group", group, "--port", port]
        if args.backend:
            cmd += ["--backend", args.backend]
        print(f"[AITO] starting receiver for id={sid} -> {group}:{port}")
        procs[sid] = subprocess.Popen(cmd)
        last_payload[sid] = payload_str

    def stop_proc(sid: str):
        p = procs.get(sid)
        if not p:
            return
        try:
            print(f"[AITO] stopping receiver for id={sid}")
            p.terminate()
            try:
                p.wait(timeout=3)
            except subprocess.TimeoutExpired:
                p.kill()
        except Exception:
            pass
        finally:
            procs.pop(sid, None)

    def on_connect(c, userdata, flags, rc):
        print("AITO connected, subscribing to", topic)
        c.subscribe(topic, qos=1)

    def on_message(c, userdata, msg):
        try:
            data = json.loads(msg.payload.decode("utf-8"))
        except Exception:
            return
        sid = msg.topic.split("/")[-1]
        spawn_or_respawn(sid, data)

    def handle_sigint(signum, frame):
        print("\nAITO autotune shutting down...")
        for sid in list(procs.keys()):
            stop_proc(sid)
        os._exit(0)

    signal.signal(signal.SIGINT, handle_sigint)

    client.on_connect = on_connect
    client.on_message = on_message

    # Heartbeat thread for autotune mode
    hb_stop = threading.Event()

    def hb_loop():
        while not hb_stop.is_set():
            try:
                payload = {
                    "role": "aito",
                    "mode": "autotune",
                    "host": socket.gethostname(),
                    "pid": os.getpid(),
                    "ts": time.time(),
                    "managed": list(procs.keys()),
                }
                client.publish("tbos/audio/health/aito", json.dumps(payload), qos=0, retain=False)
            except Exception:
                pass
            hb_stop.wait(10)

    t = threading.Thread(target=hb_loop, daemon=True)
    t.start()
    try:
        client.loop_forever()
    finally:
        hb_stop.set()
    return 0


def main():
    parser = argparse.ArgumentParser(description="AITO control plane for TBOS Audio Bridge")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_adv = sub.add_parser("advertise", help="Publish a stream advert (retained)")
    p_adv.add_argument("--id", required=True, help="Stream id (e.g., lobby)")
    p_adv.add_argument("--group", default="239.255.0.1", help="Multicast group")
    p_adv.add_argument("--port", type=int, default=5004, help="UDP port")
    p_adv.add_argument("--codec", default="opus", help="Codec (default opus)")
    p_adv.add_argument("--bitrate", default="128k", help="Bitrate (e.g., 96k)")
    p_adv.add_argument("--mono", action="store_true", help="Advertise mono channel")

    p_mon = sub.add_parser("monitor", help="Discover and list available streams")

    p_auto = sub.add_parser("autotune", help="Auto-subscribe to matching streams and spawn receivers")
    p_auto.add_argument("--match", help="Regex to match stream ids (e.g., 'lobby|floor[12]')")
    p_auto.add_argument("--receiver", help="Path to receiver.py (defaults to sibling file)")
    p_auto.add_argument("--backend", choices=["auto", "ffplay", "gst"], help="Receiver backend override")

    args = parser.parse_args()

    if args.cmd == "advertise":
        sys.exit(advertise(args))
    elif args.cmd == "monitor":
        sys.exit(monitor(args))
    elif args.cmd == "autotune":
        sys.exit(autotune(args))


if __name__ == "__main__":
    main()
