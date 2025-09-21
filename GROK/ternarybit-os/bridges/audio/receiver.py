#!/usr/bin/env python3
"""
TBOS Audio Bridge - Receiver

Tunes into LAN multicast RTP/Opus and plays audio with minimal latency.
Prefers ffplay; falls back to GStreamer if available.
"""
import argparse
import signal
import subprocess
import sys
import time
from shutil import which
import json
import os
import socket

try:
    import paho.mqtt.client as mqtt  # type: ignore
except Exception:
    mqtt = None


def has(cmd: str) -> bool:
    return which(cmd) is not None


def build_ffplay_cmd(args) -> list:
    return [
        "ffplay",
        "-nodisp",
        "-autoexit",
        "-hide_banner",
        "-loglevel", "warning",
        "-fflags", "nobuffer",
        "-flags", "low_delay",
        "-protocol_whitelist", "file,udp,rtp",
        f"rtp://{args.group}:{args.port}",
    ]


def build_gst_cmd(args) -> list:
    caps = (
        "application/x-rtp, "
        "media=(string)audio, "
        "clock-rate=(int)48000, "
        "encoding-name=(string)OPUS"
    )
    return [
        "gst-launch-1.0",
        "udpsrc", f"address={args.group}", f"port={args.port}", "auto-multicast=true",
        "caps=\"%s\"" % caps,
        "!", "rtpjitterbuffer", f"latency={args.latency}",
        "!", "rtpopusdepay",
        "!", "opusdec",
        "!", "autoaudiosink",
    ]


def main():
    parser = argparse.ArgumentParser(description="TBOS Audio Bridge Receiver (RTP/Opus multicast)")
    parser.add_argument("--group", default="239.255.0.1", help="Multicast group (default 239.255.0.1)")
    parser.add_argument("--port", type=int, default=5004, help="UDP port (default 5004)")
    parser.add_argument("--latency", type=int, default=60, help="Jitter buffer ms (GStreamer)")
    parser.add_argument("--backend", choices=["auto", "ffplay", "gst"], default="auto")

    args = parser.parse_args()

    if args.backend == "auto":
        backend = "ffplay" if has("ffplay") else ("gst" if has("gst-launch-1.0") else None)
    else:
        backend = args.backend

    if backend is None:
        print("ERROR: Neither ffplay nor GStreamer is available. Install ffmpeg or gstreamer.")
        sys.exit(1)

    cmd = build_ffplay_cmd(args) if backend == "ffplay" else build_gst_cmd(args)

    print("TBOS Audio Receiver starting...")
    print(f"Backend: {backend}")
    print(f"Tuning: {args.group}:{args.port}")
    if backend == "gst":
        print(f"Latency (ms): {args.latency}")

    proc = None
    client = None

    # Optional MQTT telemetry
    url = os.environ.get("AITO_MQTT_URL")
    if url and mqtt is not None:
        try:
            if not url.startswith("mqtt://"):
                raise ValueError("AITO_MQTT_URL must start with mqtt://")
            hostport = url[len("mqtt://"):]
            host, port = (hostport.split(":", 1) + ["1883"])[:2]
            client = mqtt.Client()
            user = os.environ.get("AITO_MQTT_USER")
            pw = os.environ.get("AITO_MQTT_PASS")
            if user:
                client.username_pw_set(user, pw)
            client.connect(host, int(port), keepalive=30)
            client.loop_start()
            print("Telemetry: MQTT connected for receiver heartbeats")
        except Exception as e:
            print(f"Telemetry disabled (MQTT error): {e}")
            client = None

    def handle_sigint(signum, frame):
        print("\nStopping receiver...")
        if proc and proc.poll() is None:
            try:
                proc.terminate()
                try:
                    proc.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    proc.kill()
            except Exception:
                pass
        try:
            if client is not None:
                client.loop_stop()
                client.disconnect()
        except Exception:
            pass
        sys.exit(0)

    signal.signal(signal.SIGINT, handle_sigint)

    try:
        proc = subprocess.Popen(cmd)
        last = time.time()
        while True:
            if time.time() - last > 5:
                print("[HB] receiver alive", time.strftime("%H:%M:%S"))
                if client is not None:
                    payload = {
                        "role": "receiver",
                        "host": socket.gethostname(),
                        "pid": os.getpid(),
                        "ts": time.time(),
                        "group": args.group,
                        "port": args.port,
                        "backend": backend,
                    }
                    try:
                        client.publish("tbos/audio/health/receiver", json.dumps(payload), qos=0, retain=False)
                    except Exception:
                        pass
                last = time.time()
            rc = proc.poll()
            if rc is not None:
                print(f"Receiver exited with code {rc}")
                sys.exit(rc)
            time.sleep(0.5)
    finally:
        handle_sigint(None, None)


if __name__ == "__main__":
    main()
