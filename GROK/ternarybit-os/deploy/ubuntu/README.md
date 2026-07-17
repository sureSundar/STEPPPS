# TBOS on Ubuntu

This profile uses a full Ubuntu desktop for hardware enablement, drivers,
updates, accessibility and application compatibility. TBOS supplies the product
identity: its C desktop, STEPPPS services, filesystem views, launcher and theme.

The first phase is deliberately reversible and user-scoped. It does not replace
GNOME Shell, the display manager, or Ubuntu system packages. The TBOS desktop is
installed as an application and can optionally start when the user signs in.

## Install prerequisites

```sh
sudo apt-get update
sudo apt-get install build-essential pkg-config libsdl2-dev
```

## Build and preview

```sh
make -C deploy/hosted check
make -C deploy/hosted desktop
./deploy/hosted/build/tbos-desktop
```

## Install the TBOS profile for the current user

```sh
deploy/ubuntu/install-user.sh
```

To launch TBOS automatically after GNOME login:

```sh
deploy/ubuntu/install-user.sh --autostart
```

Log out and back in after installation. The normal Ubuntu session remains
available as the recovery environment.

## Remove the user profile

```sh
deploy/ubuntu/uninstall-user.sh
```

## Roadmap

1. Native C desktop as a normal Ubuntu application.
2. TBOS autostart profile over a full GNOME session.
3. Files, Terminal and STEPPPS Inspector backed by Ubuntu services and FUSE.
4. Optional dedicated TBOS login session after crash recovery and accessibility
   behavior have been tested.
5. Image builder for a reproducible Ubuntu-derived TBOS installation image.

Ubuntu remains visible in system recovery and maintenance surfaces. User-facing
TBOS branding must not misrepresent security updates, licensing, or the Linux
kernel provenance.
