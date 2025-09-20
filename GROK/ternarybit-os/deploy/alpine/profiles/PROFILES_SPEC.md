# TBOS Scale Profiles

Profiles let you scale the system down/up at runtime to emulate hardware tiers (e.g., calculator, 8-bit, default) and optionally re-chainload different kernels via kexec.

Profile JSON (example)
```
{
  "name": "calculator",
  "cpu": {"quota_pct": 5},
  "memory": {"limit_mb": 32},
  "display": {"cols": 24, "rows": 6, "theme": "mono"},
  "shell": {"persona": "calc", "prompt": "calc> "},
  "features": {"disable": ["wifi","bt","sdr"], "enable": []},
  "kexec": null
}
```

Supported fields
- cpu.quota_pct: limit CPU usage via cgroup v2 cpu.max
- memory.limit_mb: limit memory via cgroup v2 memory.max
- display.cols/rows: set terminal geometry hint; tbos-profile will resize console where possible and adjust UI
- shell.persona: choose tbos-shell personality (e.g., calc, tiny, default)
- features.disable/enable: toggle feature flags in `/etc/tbos/features.conf` and signal daemons
- kexec: optional object `{ "kernel": "/boot/bzImage", "initrd": "/boot/initramfs.cpio.gz", "cmdline": "..." }`

Apply semantics
- cgroups: create `/sys/fs/cgroup/tbos/<name>/` and move shell/session pids into it
- features: update `/etc/tbos/features.conf` and touch `/run/tbos/reload`
- display: best-effort resizing and UI hints; actual physical display limited by hardware/console
- kexec (if present and `--i-accept-reboot`): execute `kexec -l ...` then `kexec -e`
