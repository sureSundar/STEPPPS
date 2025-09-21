# TBOS Audio Bridge (Resonance + Pub/Sub-ready)

This module provides a minimal, reliable audio bridge for TernaryBit OS (TBOS) that supports
“resonance” mode (LAN multicast: one sender, many receivers) and can be extended to a brokered
pub/sub control plane.

Core principles (TBOS): predictable behavior, graceful degradation, monitoring hooks.

## Features
- Multicast RTP/Opus streaming (low-latency, scalable on LAN)
- One-liner sender (file or mic) and multiple receivers
- Health logging and graceful shutdown
- Extensible control plane (future: MQTT/NATS to advertise/discover streams)

## Requirements
- Linux/macOS recommended (Windows works with proper ffmpeg install)
- `ffmpeg` and `ffplay` in PATH
- Optional: `gst-launch-1.0` if you prefer GStreamer
- LAN must permit multicast (e.g., 239.255.0.0/16). Adjust firewall accordingly.

## Quick start

Terminal 1 (Sender from file):
```
python3 sender.py --input input.mp3 --group 239.255.0.1 --port 5004 --bitrate 128k
```

Terminal 1 (Sender from microphone):
```
python3 sender.py --mic --group 239.255.0.1 --port 5004 --bitrate 96k
```

Terminal 2..N (Receiver):
```
python3 receiver.py --group 239.255.0.1 --port 5004
```

If `ffplay` is not available, the receiver will try GStreamer if present.

## Latency tuning
- Opus frame: `--frame 20` (ms), lower can reduce latency
- Receiver jitter buffer: `--latency 60` (ms). Increase on lossy networks.
- `ffplay` uses `-fflags nobuffer -flags low_delay` for low-latency playback.

## Security
- For trusted LANs, multicast is fine. For untrusted networks, prefer WebRTC (SRTP) or QUIC/TLS.
- To isolate multicast, consider a VPN or mDNS/SD advert within a secured segment.

## Extensibility (Pub/Sub)
- Add a control plane (e.g., MQTT) to advertise SDP/stream URLs on topics like:
  `tbos/audio/streams/<id>`.
- Receivers subscribe to discover and tune to the media plane automatically.

## TBOS Reliability Hooks
- Sender emits periodic heartbeats to stdout; can be redirected to a monitor.
- Receiver collects simple QoS metrics (packet drops inferred from playback stalls, optional).
- Both handle SIGINT for graceful teardown.

## Files
- `sender.py` — multicast RTP/Opus sender (file or microphone)
- `receiver.py` — tuner for RTP/Opus multicast streams
- `aito.py` — AI-Tuned Orchestrator (optional MQTT control plane)

## AITO (AI-Tuned Orchestrator)
`aito.py` provides a minimal MQTT-based control plane to advertise and discover streams. It is optional and degrades gracefully if `paho-mqtt` is not installed.

Environment variables:
- `AITO_MQTT_URL` (default: `mqtt://localhost:1883`)
- `AITO_MQTT_USER`, `AITO_MQTT_PASS` (optional)

Topics used:
- `tbos/audio/streams/<id>` (retained JSON advert)

Examples:
```
# Advertise the multicast stream (run near the sender)
python3 aito.py advertise --id lobby --group 239.255.0.1 --port 5004 --codec opus --bitrate 128k

# Monitor/discover available streams
python3 aito.py monitor
```

Note: AITO does not automatically start media processes; it only publishes and reads adverts so that other tooling (or humans) can connect receivers to the published streams.

### Autotune (auto-spawn receivers)
Automatically subscribe to matching stream adverts and spawn a `receiver.py` per stream. Requires `paho-mqtt`.

```
# Auto-subscribe to all streams
python3 aito.py autotune

# Filter by id using regex (e.g., 'lobby' or 'floor1'/'floor2')
python3 aito.py autotune --match "lobby|floor[12]"

# Force backend for receivers (ffplay or gst) and custom receiver path
python3 aito.py autotune --backend ffplay --receiver ./receiver.py
```

Stop with Ctrl+C; AITO cleans up spawned receivers.

## Notes
- Some routers/switches block or limit multicast by default. If you don’t hear audio, check:
  - OS firewall rules (allow UDP port you chose, default 5004)
  - IGMP snooping and multicast routing on network gear
  - Multicast route on Linux: `ip route add 239.0.0.0/8 dev <iface>`

## License
Apache 2.0 (inherit from TBOS repo policy, adjust as needed).
