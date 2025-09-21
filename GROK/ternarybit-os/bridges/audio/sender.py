#!/usr/bin/env python3
"""
TBOS Audio Bridge - Sender

Broadcasts audio (file or microphone) over LAN using RTP/Opus via UDP multicast.
Requires ffmpeg in PATH. Optionally supports GStreamer if ffmpeg is missing.

TBOS principles: predictable behavior, graceful degradation, minimal deps.
"""
import argparse
import os
import signal
import subprocess
import sys
import time
from shutil import which
import json
import socket

try:
    import paho.mqtt.client as mqtt  # type: ignore
except Exception:
    mqtt = None


def has(cmd: str) -> bool:
    return which(cmd) is not None


def build_ffmpeg_cmd(args) -> list:
    base = [
        "ffmpeg",
        "-hide_banner",
        "-loglevel", "warning",
        "-re",  # read input at native rate for live-like pacing
    ]
    if args.mic:
        # Linux default ALSA input; may need to adapt per OS
        # For PulseAudio: -f pulse -i default
        audio_in = ["-f", "alsa", "-i", args.mic_device or "default"]
    else:
        audio_in = ["-stream_loop", "-1", "-i", args.input]

    opus = [
        "-c:a", "libopus",
        "-b:a", args.bitrate,
        "-ar", "48000",
        "-ac", "1" if args.mono else "2",
    ]
    if args.frame:
        opus += ["-frame_duration", str(args.frame)]
    if args.lowdelay:
        opus += ["-application", "lowdelay"]

    out = [
        "-f", "rtp",
        f"rtp://{args.group}:{args.port}"
    ]

    return base + audio_in + opus + out


def build_gst_cmd(args) -> list:
    src = []
    if args.mic:
        # Auto-detect default microphone
        src = [
            "autoaudiosrc",
        ]
    else:
        src = [
            "filesrc", f"location={args.input}", "!", "decodebin",
        ]

    chan = "channels=1" if args.mono else "channels=2"
    pipe = src + [
        "!", "audioconvert",
        "!", "audioresample",
        "!", "audio/x-raw,rate=48000,%s" % chan,
        "!", "opusenc", f"bitrate={int(args.bitrate.replace('k',''))*1000}",
    ]
    if args.frame:
        pipe += [f"frame-size={args.frame}"]
    pipe += [
        "!", "rtpopuspay",
        "!", "udpsink", f"host={args.group}", f"port={args.port}", "auto-multicast=true",
    ]

    return ["gst-launch-1.0"] + pipe


def main():
    parser = argparse.ArgumentParser(description="TBOS Audio Bridge Sender (RTP/Opus multicast)")
    src = parser.add_mutually_exclusive_group(required=True)
    src.add_argument("--input", help="Audio file to loop and stream (e.g., input.mp3)")
    src.add_argument("--mic", action="store_true", help="Use microphone as source")
    parser.add_argument("--mic-device", help="ALSA/Pulse device for mic (e.g., 'default' or 'hw:0,0')")
    parser.add_argument("--group", default="239.255.0.1", help="Multicast group (default 239.255.0.1)")
    parser.add_argument("--port", type=int, default=5004, help="UDP port (default 5004)")
    parser.add_argument("--bitrate", default="128k", help="Opus bitrate (e.g., 96k, 128k)")
    parser.add_argument("--frame", type=int, default=20, help="Opus frame duration ms (default 20)")
    parser.add_argument("--mono", action="store_true", help="Force mono (saves bandwidth)")
    parser.add_argument("--lowdelay", action="store_true", help="Use lowdelay Opus application mode")
    parser.add_argument("--backend", choices=["auto", "ffmpeg", "gst"], default="auto", help="Select media backend")

    args = parser.parse_args()

    if args.backend == "auto":
        backend = "ffmpeg" if has("ffmpeg") else ("gst" if has("gst-launch-1.0") else None)
    else:
        backend = args.backend

    if backend is None:
        print("ERROR: Neither ffmpeg nor GStreamer is available. Install ffmpeg or gstreamer.")
        sys.exit(1)

    if args.input and not os.path.exists(args.input):
        print(f"ERROR: Input file not found: {args.input}")
        sys.exit(1)

    if backend == "ffmpeg":
        cmd = build_ffmpeg_cmd(args)
    else:
        cmd = build_gst_cmd(args)

    print("TBOS Audio Sender starting...")
    print(f"Backend: {backend}")
    print(f"Source: {'mic' if args.mic else args.input}")
    print(f"Multicast: {args.group}:{args.port}")
    print(f"Bitrate: {args.bitrate}, Frame: {args.frame} ms, Mono: {args.mono}")

    # Handle SIGINT for clean teardown
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
            print("Telemetry: MQTT connected for sender heartbeats")
        except Exception as e:
            print(f"Telemetry disabled (MQTT error): {e}")
            client = None

    def handle_sigint(signum, frame):
        print("\nStopping sender...")
        if proc and proc.poll() is None:
            try:
                proc.terminate()
                try:
                    proc.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    proc.kill()
            except Exception:
                pass
        sys.exit(0)

    signal.signal(signal.SIGINT, handle_sigint)

    heartbeat_t = time.time()
    try:
        proc = subprocess.Popen(cmd)
        while True:
            # simple heartbeat every 5s
            if time.time() - heartbeat_t > 5:
                print("[HB] sender alive", time.strftime("%H:%M:%S"))
                if client is not None:
                    payload = {
                        "role": "sender",
                        "host": socket.gethostname(),
                        "pid": os.getpid(),
                        "ts": time.time(),
                        "group": args.group,
                        "port": args.port,
                        "source": "mic" if args.mic else (args.input or ""),
                        "bitrate": args.bitrate,
                        "frame": args.frame,
                        "mono": bool(args.mono),
                        "backend": backend,
                    }
                    try:
                        client.publish("tbos/audio/health/sender", json.dumps(payload), qos=0, retain=False)
                    except Exception:
                        pass
                heartbeat_t = time.time()
            rc = proc.poll()
            if rc is not None:
                print(f"Sender exited with code {rc}")
                sys.exit(rc)
            time.sleep(0.5)
    finally:
        try:
            if client is not None:
                client.loop_stop()
                client.disconnect()
        except Exception:
            pass
        handle_sigint(None, None)


if __name__ == "__main__":
    main()
