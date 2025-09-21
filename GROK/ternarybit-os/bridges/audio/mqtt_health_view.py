#!/usr/bin/env python3
"""
TBOS Audio Bridge - MQTT Health Dashboard

Subscribes to tbos/audio/health/# and prints a compact live view of sender/receiver/AITO heartbeats.
Degrades gracefully if paho-mqtt is missing.

Usage:
  export AITO_MQTT_URL=mqtt://localhost:1883
  python3 mqtt_health_view.py
"""
import os
import sys
import time
import json
import threading
from collections import defaultdict

try:
    import paho.mqtt.client as mqtt  # type: ignore
except Exception:
    print("paho-mqtt not installed. pip install paho-mqtt")
    sys.exit(1)


def parse_mqtt_url(url: str):
    if not url.startswith("mqtt://"):
        raise ValueError("AITO_MQTT_URL must start with mqtt://")
    rest = url[len("mqtt://"):]
    if ":" in rest:
        host, port = rest.split(":", 1)
        return host, int(port)
    return rest, 1883


def main():
    url = os.environ.get("AITO_MQTT_URL", "mqtt://localhost:1883")
    host, port = parse_mqtt_url(url)

    state = {
        "sender": {},
        "receiver": {},
        "aito": {},
    }
    lock = threading.Lock()

    def on_connect(c, userdata, flags, rc):
        c.subscribe("tbos/audio/health/#", qos=0)

    def on_message(c, userdata, msg):
        try:
            data = json.loads(msg.payload.decode("utf-8"))
        except Exception:
            return
        topic = msg.topic
        with lock:
            if topic.endswith("/sender"):
                state["sender"][data.get("host")] = data
            elif topic.endswith("/receiver"):
                key = f"{data.get('host')}:{data.get('pid')}"
                state["receiver"][key] = data
            elif topic.endswith("/aito"):
                state["aito"][data.get("host")] = data

    client = mqtt.Client()
    user = os.environ.get("AITO_MQTT_USER")
    pw = os.environ.get("AITO_MQTT_PASS")
    if user:
        client.username_pw_set(user, pw)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(host, port, keepalive=30)

    client.loop_start()

    try:
        while True:
            with lock:
                print("\n==== TBOS Audio Health (", time.strftime("%H:%M:%S"), ") ====")
                # senders
                print("Senders:")
                if state["sender"]:
                    for host, s in state["sender"].items():
                        print(f"  - {host}:{s.get('pid')} {s.get('group')}:{s.get('port')} src={s.get('source')} br={s.get('bitrate')} frame={s.get('frame')} mono={s.get('mono')} backend={s.get('backend')}")
                else:
                    print("  (none)")
                # receivers
                print("Receivers:")
                if state["receiver"]:
                    for key, r in state["receiver"].items():
                        print(f"  - {key} {r.get('group')}:{r.get('port')} backend={r.get('backend')}")
                else:
                    print("  (none)")
                # aito
                print("AITO:")
                if state["aito"]:
                    for host, a in state["aito"].items():
                        mode = a.get("mode")
                        managed = a.get("managed")
                        extra = f" managed={managed}" if managed else ""
                        print(f"  - {host}:{a.get('pid')} mode={mode}{extra}")
                else:
                    print("  (none)")
            time.sleep(3)
    except KeyboardInterrupt:
        pass
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
