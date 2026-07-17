# Hosted TBOS desktop

This target runs the TBOS C desktop on a Linux host while keeping the GUI core
independent of Linux. SDL2 is the hosted display/input adapter and can use X11
or Wayland according to the host SDL configuration.

The Linux distribution is the hardware-enablement layer, not the TBOS semantic
layer. PXFS, UCFS and RF2S can later be mounted below `TBOS_FS_ROOT`; the desktop
and STEPPPS services should consume those mount points without knowing whether
the host is Alpine or Ubuntu.

## Build and verify without a display

```sh
make -C deploy/hosted check
```

This builds the portable renderer and writes three PPM screenshots under
`deploy/hosted/build/`.

## Alpine

```sh
apk add build-base pkgconf sdl2-dev
make -C deploy/hosted desktop
doas make -C deploy/hosted install
```

For a small graphical base, install a Wayland compositor such as `cage` or
`sway`, plus the platform's graphics and input packages. The exact packages are
device-specific and intentionally remain outside the portable GUI target.

## Ubuntu

```sh
sudo apt-get install build-essential pkg-config libsdl2-dev
make -C deploy/hosted desktop
sudo make -C deploy/hosted install
```

Launch it from the desktop menu or run:

```sh
tbos-desktop-session
```

## Boundary with FUSE and STEPPPS

The session reserves these hosted mount points by default:

- `$TBOS_FS_ROOT/pxfs`
- `$TBOS_FS_ROOT/ucfs`
- `$TBOS_FS_ROOT/rf2s`

The next integration step is a mount supervisor that starts each FUSE adapter,
then exposes the stable object metadata from the portable STEPPPS sidecar core
to the Files and STEPPPS Inspector applications.
