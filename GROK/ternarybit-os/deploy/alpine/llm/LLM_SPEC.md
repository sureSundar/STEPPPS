# TBOS Micro LLM Integration (Spec)

Goal
- Provide a tiny, offline-capable LLM path for TBOS with constrained generation of valid STEPPPS JSON frames and prompts.
- Work on low-RAM systems by supporting external backends (llama.cpp server) and a rules/template fallback.

Targets
- Primary backend: llama.cpp server (`--server`) reachable at TBOS_LLM_ENDPOINT (default http://127.0.0.1:8081)
- Constrained decoding: GBNF grammar for STEPPPS JSON (`steppps_json.gbnf`)
- Local API: `tbos-llm-proxy` exposes `/generate` and `/steppps` endpoints
- CLI: `tbos-llmctl` (shell-friendly)
- Prompt interaction: `/interact` executes the `prompt` or `P_prompt` dimension
  from a STEPPPS JSON entity against the configured local LLM endpoint.

Example:

```sh
export TBOS_LLM_ENDPOINT=http://127.0.0.1:8081
python3 deploy/alpine/llm/tbos-llm-proxy.py

# In a second terminal:
deploy/alpine/llm/tbos-llmctl interact \
  --file steppps/v2/examples/hello-secure.steppps.json
```

The JSON entity may use v2 `prompt`, v1 `P_prompt`, or development-action
fields such as `text`, `creation_prompt`, and `next_genai_prompt`. API keys and
provider credentials are never read from the STEPPPS entity.

### Deterministic offline provider

Exercise the complete proxy, CLI, Prompt-dimension, and STEPPPS validation path
without a model, API key, or network connection:

```sh
export TBOS_LLM_PROVIDER=mock
python3 deploy/alpine/llm/tbos-llm-proxy.py

# In a second terminal:
deploy/alpine/llm/tbos-llmctl interact \
  --file steppps/v2/examples/hello-secure.steppps.json \
  --json
```

The mock output is stable for identical input and contains a short SHA-256
digest, making it suitable for repeatable tests. Grammar-backed `/steppps`
requests return a minimal valid mock frame.

The proxy binds only to `127.0.0.1:8088` by default. Tests may select another
local port with `TBOS_LLM_PROXY_PORT`; clients select it with `TBOS_LLM_PROXY`.

### OpenAI provider

Use the OpenAI Responses API instead of a local llama.cpp model:

```sh
export TBOS_LLM_PROVIDER=openai
export OPENAI_API_KEY='your-api-key'
export OPENAI_MODEL=gpt-5.4-mini
python3 deploy/alpine/llm/tbos-llm-proxy.py
```

Then use the same `tbos-llmctl interact --file ...` command. The API key is
read only from the process environment and must not be stored in a STEPPPS JSON
entity or committed to the repository.

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
