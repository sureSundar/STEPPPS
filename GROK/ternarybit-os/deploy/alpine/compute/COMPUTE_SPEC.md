# PXFS Compute (Dataflow via Filesystem)

Idea
- Expose compute as files under a PXFS tree. Reading a path triggers compute on cache miss, then returns the cached result. Subsequent reads are free.

Path convention
- `pxfs://compute/<op>/<kv-params>/input=<vpath>/out=<name>`
  - Example:
    - `pxfs://compute/steppps.transform/op=pixel.replace_color/from=0,0,255/to=255,64,0/input=pxfs://A/B/frame.json/out=out.json`

Cache
- Backing cache at `/var/lib/pxfs/compute/<sha256>/...`.
- The FUSE mounts resolve PXFS paths and RF2S paths automatically.

Supported ops (v0)
- `steppps.transform` with sub-op `pixel.replace_color` via `steppps-tools`.
- `steppps.llm_generate` via `tbos-llm-proxy` `/steppps` endpoint.

Failover
- If compute fails, returns ENOENT and logs JSON diagnostic to `/var/log/tbos/compute.jsonl`.

Security
- Local-only operations. Offload to peers handled by `tbos-offload`/`tbos-jobd` (separate).

Env
- `TBOS_PXFS_MOUNT` (default `/mnt/px`), `TBOS_PXFS_BACKING` (default `/var/lib/pxfs`)
- `TBOS_RF2S_MOUNT` (default `/mnt/rf`), `TBOS_RF2S_BACKING` (default `/var/lib/rf2s`)
- `TBOS_LLM_PROXY` (default `http://127.0.0.1:8088`)
