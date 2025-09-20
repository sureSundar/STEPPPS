# TBOS Micro LLM Integration (Spec)

Goal
- Provide a tiny, offline-capable LLM path for TBOS with constrained generation of valid STEPPPS JSON frames and prompts.
- Work on low-RAM systems by supporting external backends (llama.cpp server) and a rules/template fallback.

Targets
- Primary backend: llama.cpp server (`--server`) reachable at TBOS_LLM_ENDPOINT (default http://127.0.0.1:8081)
- Constrained decoding: GBNF grammar for STEPPPS JSON (`steppps_json.gbnf`)
- Local API: `tbos-llm-proxy` exposes `/generate` and `/steppps` endpoints
- CLI: `tbos-llmctl` (shell-friendly)

Operation
- `/generate` forwards to llama.cpp server with provided prompt, optionally a grammar.
- `/steppps` uses the STEPPPS grammar and validation pipeline to return a valid frame JSON or an error array with auto-fix attempts.
- If the remote endpoint is unavailable, the proxy returns a rules/template-based fallback result with a warning.

Security
- Local-only binding by default (127.0.0.1).
- Size/time limits to avoid resource abuse.

Notes
- Building llama.cpp binaries is out-of-scope for the initramfs; expect model and server to be provisioned externally, or installed post-boot onto persistent storage.
- The grammar is versioned with the STEPPPS schema and should be updated in lockstep.
