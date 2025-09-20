# STEPPPS Core Specification (v0.1)

This document defines the canonical object model and semantics for STEPPPS across device tiers. Versioned to enable compatibility and graceful degradation.

Dimensions
- Space: physical context (geo, room, device locus), optional uncertainty.
- Time: timestamps, durations, schedules, temporal relations.
- Event: observable occurrences (sensor readings, user actions, system signals).
- Psychology: preferences, intent, state vectors (interpretable scores), optional.
- Pixel: visual state (pixels, layers, sprites) and transformations.
- Prompt: AI/LLM intent, instructions, or constraints; inputs and expected outputs.
- Script: executable orchestration of operations, policies, adapters.

Core objects
- Frame: a snapshot binding multiple dimensions.
- Event: a time-bound occurrence that can mutate a Frame or spawn new Frames.
- Prompt: an instruction bundle to generate/transform Frames or Events.

Schemas (see `steppps/schemas/`)
- `steppps_frame.json` — Frame object
- `steppps_event.json` — Event object
- `steppps_prompt.json` — Prompt object

Versioning
- `meta.version`: e.g., `0.1`. Minor increments may add optional fields. Major increments may change required fields.

Profiles (capabilities)
- micro: tiny devices (no heavy arrays; references only)
- edge: moderate capacity (limited arrays)
- full: unrestricted

Minimal Frame (profile micro)
```
{
  "meta": {"version": "0.1", "id": "<uuid>", "profile": "micro"},
  "time": {"ts": "2025-01-01T00:00:00Z"},
  "space": {"device_id": "X", "loc": {"lat": 0.0, "lon": 0.0}},
  "events": [{"kind": "status", "msg": "booted"}],
  "links": [{"rel": "rf2s", "href": "/var/lib/rf2s/99.500000/audio.raw"}]
}
```

Transformations (examples)
- pixel.replace_color: `{from: [0,0,255], to: [255,64,0]}`
- pixel.threshold: `{channel: "R", min: 32, max: 200}`
- event.filter: `{kind: "wifi.ap", min_signal_dbm: -70}`

Validation
- JSON Schema validation; if unavailable, fallback to lightweight static checks (required keys by profile).

Security
- Signed frames optional (`meta.sig`), origin tags (`meta.origin`).

Interoperability
- Artifacts referenced via RF2S/PXFS/UCFS paths.
- TBDS can transport small frames/events in NDJSON form.
