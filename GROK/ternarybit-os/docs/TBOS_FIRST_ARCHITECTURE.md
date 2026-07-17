# TBOS-First Architecture

## Product invariant

TBOS is the operating environment. It owns the boot narrative, shell, desktop,
application model, filesystem namespace, STEPPPS semantics and user identity.
A hosted operating system may provide replaceable hardware and compatibility
services, but it must not define the TBOS interaction model or visual identity.

This is not a Linux desktop theme and not a renamed distribution.

## Canonical stack

```text
TBOS experience
  desktop, shell, Files, Terminal, STEPPPS Inspector
                         |
TBOS runtime
  supervisor, application model, events, identity, policy
                         |
TBOS core
  VFS, STEPPPS, scheduler contract, networking contract
                         |
TBOS platform interface
  display, input, clock, storage, network, process compatibility
                 /                       \
TBOS native backend                 hosted backend
  framebuffer, devices               Linux kernel services
  bare-metal drivers                 and Linux applications
```

The arrows point from TBOS toward services it consumes. Ubuntu is never a
parent of the TBOS product model, even when a Linux kernel temporarily executes
the hosted backend.

## Existing foundations

- Boot and kernel entry: `boot/` and `kernel/kernel_main.c`
- Hardware abstraction: `include/tbos/hal.h`
- Filesystem namespace: `include/tbos/vfs.h` and `kernel/fs/vfs.c`
- Primary command environment: `shell/shell.c`
- Portable renderer and widgets: `src/gui/tbos_pixel.*` and `src/gui/tbos_gui.*`
- Desktop model: `src/gui/tbos_desktop.*`
- Hosted runtime prototype: `hosted/tbos_hosted_linux.c`

These components must converge behind stable interfaces. New hosted work must
adapt to them rather than create a second Python or distribution-owned TBOS.

## Non-copycat principles

1. Start from TBOS concepts and workflows, not GNOME, macOS or Windows layouts.
2. Reuse protocols and accessibility conventions where interoperability matters;
   do not copy another product's composition or branding.
3. Every visible system object has one identity shared by shell, GUI and VFS.
4. A GUI action and its shell equivalent invoke the same runtime command.
5. STEPPPS is operational metadata and policy, not decorative terminology.
6. Platform-specific code stays behind adapters and is replaceable.
7. A feature is complete only when it has a native/headless test and a hosted
   test, or an explicit documented reason why one environment cannot support it.

## First vertical slice

The first integrated milestone is intentionally small:

```text
TBOS supervisor starts
  -> initializes platform interface
  -> initializes VFS and mounts RAMFS
  -> initializes the TBOS command runtime
  -> starts the TBOS desktop
  -> opens a Terminal surface
  -> executes `pwd`, `ls`, and `steppps` through the real TBOS shell/runtime
  -> shows the same VFS contents in the Files surface
```

No simulated terminal output and no hard-coded file listing qualify for this
milestone.

## Work sequence

### Phase 1: convergence

- Extract a reusable command execution API from the interactive shell.
- Introduce one runtime context shared by shell, GUI and hosted entry points.
- Replace demo-only Terminal and Files data with runtime calls.
- Add deterministic headless integration tests.

### Phase 2: platform boundary

- Extend the platform interface for display, input, time and process launch.
- Keep SDL2 as one hosted display adapter.
- Add framebuffer/DRM and bare-metal adapters without changing desktop logic.
- Treat Linux application execution as an optional compatibility capability.

### Phase 3: filesystem identity

- Make PXFS, UCFS and RF2S VFS drivers visible to the same shell and GUI.
- Add stable STEPPPS object identities and lifecycle events.
- Present metadata through a TBOS Inspector rather than raw companion-file
  clutter.

### Phase 4: system image

- Boot directly into the TBOS supervisor and desktop.
- Keep Linux distribution details outside the normal user experience.
- Provide an explicit recovery/maintenance environment.
- Produce reproducible native and hosted images from versioned manifests.

## Completion rule

A TBOS-first desktop is achieved when removing GNOME Shell or any particular
host distribution does not change the TBOS command model, desktop model,
filesystem identity or application interfaces. Only platform adapters and the
available compatibility applications may change.
