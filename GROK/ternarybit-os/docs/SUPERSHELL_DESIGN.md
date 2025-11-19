# TBOS Supershell Architecture (Prototype)

The supershell acts as a persona orchestrator layered on top of the existing
conscious shell. Instead of hard-coding a single shell entry point, TBOS now
registers persona modes (`calc_4k`, `mobile_64k`, `desktop_512m`, `super_1g`)
and routes launch requests through a tiny mode registry. This enables:

1. **Runtime morphing** – we can switch from one persona shell to another
   without rebuilding, as each mode is a struct containing metadata and an
   entry function.
2. **Extensible adapters** – packages or future subsystems can register custom
   personas (e.g., `quantum_lab`, `edge_gateway`) by calling
   `tbos_supershell_register`.
3. **Observability hooks** – the registry tracks the active persona so we can
   expose it to diagnostics or future network protocols.

## Key entry points

* `tbos_supershell_init()` – idempotent setup that registers the default modes.
* `tbos_supershell_register(const tbos_supershell_mode_t*)` – add a new mode at
  runtime (e.g., from a package or plugin).
* `tbos_supershell_launch(const char* persona_id)` – morph into the requested
  persona; falls back to the conscious shell if the persona is unknown.
* `tbos_supershell_list()` – debugging helper to enumerate registered personas.

The prototype simply delegates every persona to `tbos_shell_run()`, but the
hook architecture is ready for persona-specific entry points (different shell
command sets, UI layers, or network adapters). The integrated build now calls
`tbos_supershell_launch(getenv("TBOS_PERSONA_NAME"))`, so future packages can
override personas without touching the main program.

Next steps:

1. Allow packages to call `tbos_supershell_register` at init time (e.g., the
   network package can register a persona that preloads Sangha commands).
2. Expose a shell command (`persona morph <id>`) that uses the registry so a
   running system can switch personas live.
3. Persist per-persona shell state (history, karma counters) so morphing keeps
   context rather than restarting the entire shell.

CLI helpers:

```
# List personas known to the supershell registry
./tools/tbos supershell list

# Launch an already-built persona binary directly in a given supershell mode
./tools/tbos supershell morph --persona desktop_512m

# Inside TBOS shell
persona list
persona morph network_sangha
sh        # Launch BusyBox subshell (set TBOS_SUBSHELL_BIN to override path)
```

Auto Rebirth
------------
When the lifecycle policy ends a session (idle, time, karma), the supershell
reads the queued rebirth persona (e.g., `network_sangha` after karma loss) and
automatically restarts the shell with a fresh genesis. A console banner
documents the chain so users see why the shell came back. See
`docs/LIFECYCLE_POLICY.md` for the mapping from demise reasons to rebirth
personas.
