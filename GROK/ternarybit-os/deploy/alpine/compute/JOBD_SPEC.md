# TBOS Job Daemon (tbos-jobd) and Offload (tbos-offload)

Goal
- Execute PXFS Compute jobs opportunistically on low-CPU devices with caching, or offload to peers when configured.

Job format (JSON)
```
{
  "id": "job-001",
  "op": "steppps.transform",        // or steppps.llm_generate
  "inputs": {"frame": "pxfs://A/B/frame.json"},
  "params": {"op": "pixel.replace_color", "from": "0,0,255", "to": "255,64,0"},
  "out": "pxfs://A/B/out.json",
  "priority": 5
}
```

Queues
- Local queue dir (default): `/var/lib/tbos/jobs/`
- Optional PXFS queue: `/mnt/px/jobs/` (if mounted)

Behavior
- tbos-jobd polls queue dirs, claims a job (creates `.lock`), materializes via `tbos-compute` and writes `result_href` and `status` back into the job file, then creates `.done`.
- If offload is enabled and local policy says "skip" (e.g., persona `calculator`) it forwards the job to a peer via `tbos-offload`.

Offload config
- `/etc/tbos/offload-peers.json`:
```
{
  "peers": ["http://192.168.1.10:9090", "http://10.0.0.5:9090"],
  "shared_key": "changeme"
}
```

Environment
- `TBOS_JOBD_INTERVAL` (default 5s)
- `TBOS_JOBD_LOCAL_ONLY` (default 1) — set 0 to enable offload
- `TBOS_LLM_PROXY` for LLM operations via `tbos-compute`

Security
- Offload is best-effort demo; add signatures/HMAC in production.
