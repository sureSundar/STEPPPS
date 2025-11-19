# TBOS Package Overlays

Persona builds already tailor memory/page sizes. Packages let you stack
feature-specific overlays (network, filesystem, etc.) on top of any persona
without inventing a new config format. Each package is a tiny `.mk` fragment
inside `config/packages/` that sets environment variables consumed by the
build scripts.

Current conventions:

| Variable                 | Purpose |
|--------------------------|---------|
| `TBOS_PACKAGE_DEFINES`   | Extra compiler defines appended to persona defines. |
| `TBOS_PACKAGE_NOTES`     | Human-readable notes printed by build scripts. |

Usage:

```
make persona-build PERSONA=desktop_512m PACKAGES="network_core filesystem_universal"
```

This Make invocation includes `config/packages/network_core.mk` and
`config/packages/filesystem_universal.mk`, exports their defines/notes, and
then calls `build_tbos_system.sh` with those settings. Future tooling (npm or
JSON metadata) can generate these `.mk` fragments automatically, so the make
interface remains stable.

Available packages (extend as needed):

| Package | Purpose |
|---------|---------|
| `network_core` | Conscious TCP/IP stack, HTTP client, Sangha bridges. |
| `filesystem_universal` | Mount UCFS/PXFS overlays with VFS glue. |
| `scheduler_rt` | Real-time scheduler queues, metrics, and tracing. |
| `drivers_io` | Driver framework plus baseline I/O drivers. |
| `security_guardian` | Dharmic security enforcement + karmic audit hooks. |
