# Archived: deploy/linux/

Moved here 2026-07-18 per Guru sign-off on
`docs/TBOS_CANONICAL_MANIFEST.md` ("Hosted deployment").

`tbos-ubuntu-installer.sh` duplicated `deploy/ubuntu/install-user.sh`'s
responsibility (install TBOS for a user on Ubuntu) with a different install
layout and config format, and assumed a `tbos` command already existed on
`PATH` — it had no working binary to install.

Replacement: `deploy/ubuntu/`, which installs the real SDL2-windowed desktop
built under `deploy/hosted/`.
