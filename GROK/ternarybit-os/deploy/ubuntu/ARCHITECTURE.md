# Ubuntu-first architecture

```text
TBOS identity and applications
  C desktop, Files, Terminal, STEPPPS Inspector, themes
                         |
TBOS hosted services
  sidecar core, mount supervisor, PXFS, UCFS, RF2S
                         |
Ubuntu desktop platform
  GNOME session, portals, audio, network, accessibility
                         |
Linux hardware platform
  kernel, DRM/KMS, GPU, storage, input and device drivers
```

## Ownership boundary

TBOS owns its visual language, navigation, STEPPPS semantics and filesystem
experience. Ubuntu owns hardware support, package updates, login, recovery,
security services and compatibility with existing Linux applications.

The initial profile runs TBOS within GNOME rather than replacing GNOME Shell.
This keeps recovery and accessibility available while the native desktop gains
process supervision, crash recovery, multi-monitor support and system dialogs.
