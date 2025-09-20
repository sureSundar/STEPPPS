# TBDS → TBOS Integration Spec

TBDS (Ternary Boot Debug/Diagnostic Service) exists in the bootloader stage. TBOS can leverage it as a unified control and telemetry channel from the first instruction after kernel entry through full userspace.

Goals
- Early-boot parameter handoff (features, policy, identity) via TBDS.
- Bi-directional, line-oriented control channel for diagnostics and provisioning.
- Transport-agnostic: serial UART, EFI config table, DTB node, or reserved memory mailbox.
- Safe-by-default: read-only telemetry unless explicitly escalated via policy.

Transports
1) Serial UART (minimal baseline)
   - Bootloader opens a UART and speaks TBDS over plain text/NDJSON.
   - Linux cmdline: `tbds=serial,ttyS0,115200` (or `ttyAMA0` on arm64).
   - TBOS `tbds-agent` opens the TTY and exchanges framed JSON lines with a small prefix.

2) DTB Node (arm64 bare-metal)
   - DT node `/tbds` with properties:
     - `compatible = "tbos,tbds-1"`
     - `serial = "ttyAMA0,115200"` or
     - `mailbox-base = <0x... 0x...>; mailbox-size = <...>`
   - TBOS parses FDT at `/proc/device-tree/tbds/*` to pick transport.

3) UEFI Config Table (x86_64/arm64)
   - Expose TBDS config via a vendor GUID pointing to a small struct or JSON blob in memory.
   - TBOS reads via efivarfs or an EFI runtime call wrapper.

4) Reserved Memory Mailbox
   - Bootloader fills a ring buffer with a magic header `TBDS0\0` and writes initial K/V parameters.
   - TBOS locates mailbox via Linux cmdline `tbds=mem,0xADDR,SIZE` or DTB node.

Message Format
- Line-oriented NDJSON for simplicity. Each message is a JSON object with fields:
  - `ts`: ISO-8601 UTC
  - `type`: `hello|log|cmd|resp|hb|file`
  - `seq`: monotonically increasing sequence id (optional)
  - `data`: arbitrary object (command args, log lines, etc.)
- Example hello:
```json
{"ts":"2025-09-20T14:00:00Z","type":"hello","data":{"role":"tbos","version":"0.1","arch":"x86_64"}}
```

Recommended Commands
- `cmd:get` with keys: `features`, `radio-policy`, `ssh-authorized-keys`
- `cmd:put` for small files (base64-encoded), e.g., update `/etc/tbos/features.conf`
- `cmd:exec` for whitelisted commands only (e.g., `tbos-sensorctl wifi scan`) — disabled by default
- `cmd:reboot`

Security
- Default: read-only telemetry (hello, hb, logs). `cmd:*` disabled unless `/etc/tbos/tbds-policy.json` enables it with allowlists.
- Add HMAC optional signing later.

Integration Points in TBOS
- Early: `steppps-init` starts `tbds-agent` if configured (via cmdline or `/etc/tbos/features.conf`).
- Telemetry: tail `/var/log/tbos/*.jsonl` and forward as `log` frames to TBDS.
- Provisioning: accept `cmd:get` and (optionally) `cmd:put` for small configs.

Kernel Cmdline Examples
- x86_64: `console=ttyS0,115200n8 console=tty0 tbds=serial,ttyS0,115200`
- arm64:  `console=ttyAMA0,115200 console=tty0 tbds=serial,ttyAMA0,115200`
- mailbox: `tbds=mem,0x40000000,65536`

Minimal Acceptance
- TBOS emits `hello` and periodic `hb` messages over chosen transport.
- Optional: TBDS host sends `cmd:get features` and receives `/etc/tbos/features.conf` content.
