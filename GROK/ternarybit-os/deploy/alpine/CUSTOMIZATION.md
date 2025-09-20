# TBOS × Alpine Customization Spec

Fill this doc to specify all customizations you want. I will translate it into build scripts, kernel configs, initramfs layout, and runtime configs. Keep PRs focused per section when possible.

---

## 0) Target Matrix
- Architectures: [ ] x86_64  [ ] arm64  [ ] riscv64  [ ] armv7  [ ] mips  [ ] other: ______
- Boot style per arch:
  - x86_64: [ ] Linux boot protocol (bzImage)  [ ] Multiboot2  [ ] UEFI stub  [ ] other: ______
  - arm64:  [ ] EFI stub (preferred)  [ ] Direct Image+DTB  [ ] other: ______
  - riscv64: [ ] OpenSBI + Linux  [ ] UEFI  [ ] other: ______
- Boards/Platforms (per arch):
  - x86_64: ______________________________________________
  - arm64: ______________________________________________
  - riscv64: ____________________________________________

- Min RAM / storage per class:
  - Tiny: RAM ____MB, storage ____MB  |  Typical: RAM ____GB, storage ____GB

---

## 1) Boot & Console
- Primary console:
  - x86_64: [ ] ttyS0@115200  [ ] tty0 (VGA/EFI fb)  [ ] other: ______
  - arm64:  [ ] ttyAMA0@115200  [ ] ttyS0@115200  [ ] framebuffer  [ ] other: ______
- Early console/earlyprintk: [ ] enable  [ ] disable
- Kernel cmdline additions (space-separated):
```
<your extra cmdline>
```
- Init system: [ ] steppps-init (PID 1, default)  [ ] busybox init  [ ] openrc  [ ] other: ______

---

## 2) Kernel Features
- Filesystems: [ ] ext4  [ ] vfat  [ ] squashfs  [ ] overlayfs  [ ] xfs  [ ] btrfs  [ ] ntfs3
- Networking: [ ] IPv6  [ ] WireGuard  [ ] nftables  [ ] iptables  [ ] VLAN  [ ] Bonding
- Crypto: [ ] dm-crypt/LUKS  [ ] FIPS-ish  [ ] TPM 2.0
- Compression: [ ] gzip (initramfs)  [ ] xz  [ ] zstd
- Device support (drivers/modules of note):
```
<list NICs, storage, USB, serial, etc>
```

---

## 3) Userspace Packages (v1)
- Core (default): BusyBox, apk, ca-certificates, tzdata, util-linux, e2fsprogs, dosfstools, blkid
- Networking (default): iproute2, udhcpc (BusyBox), curl, wget, openssl
- SSH: [ ] Dropbear (default)  [ ] OpenSSH  [ ] none
- Editors: [ ] vim  [ ] nano  [ ] BusyBox vi (default)
- Session: [ ] tmux (default)  [ ] screen  [ ] none
- Debugging: [ ] strace  [ ] lsof  [ ] tcpdump  [ ] ethtool  [ ] pciutils  [ ] usbutils
- Dev tools (optional): [ ] git  [ ] python3  [ ] make  [ ] gcc/musl-dev  [ ] go  [ ] rust
- Your additions:
```
<list any additional packages>
```

---

## 4) TBOS/STEPPPS Shell
- Default user shell: [ ] tbos-shell (recommended)  [ ] /bin/sh  [ ] other: ______
- Shell features to enable:
  - [ ] Command alias set compatible with tiny RTOS tier
  - [ ] STEPPPS object tools (parse/validate/render)
  - [ ] Network probing commands
  - [ ] Process supervisor hooks
  - [ ] Logging: JSONL to /var/log/tbos/*.log
- Startup scripts:
```
<commands to run on first boot and every boot>
```

---

## 5) Networking & Identity
- Hostname format: __________________
- DHCP client: [ ] udhcpc (default)  [ ] dhcpcd  [ ] static only
- Static config example (if needed):
```
IFACE=eth0 IP=___ MASK=___ GW=___ DNS=___
```
- SSH policy: [ ] enable server  [ ] client only  [ ] disable SSH
- SSH keys provisioning:
  - [ ] pre-baked in initramfs
  - [ ] fetched from URL: __________
  - [ ] user inject via TBOS channel

---

## 6) Storage & Persistence
- Mode: [ ] Stateless initramfs (default)  [ ] Overlayfs on persistent storage  [ ] Writable rootfs
- Persistence locations to probe (order):
```
/dev/mmcblk0p1, /dev/sda1, LABEL=TBOS_DATA, etc
```
- Files/dirs to persist (if overlay):
```
/etc, /var, /home/tbos
```
- Encryption: [ ] none  [ ] LUKS (prompt/passfile/Tang): __________

---

## 7) Security
- Kernel/initramfs signing: [ ] required  [ ] optional  [ ] none
- SSH hardening: [ ] only keys  [ ] disable root  [ ] 2FA  [ ] FIPS-ish crypto
- Read-only rootfs: [ ] yes  [ ] no
- Exec lockdown: [ ] no new privs  [ ] seccomp profile  [ ] apparmor

---

## 8) Telemetry & Logging
- System logs: [ ] busybox syslogd  [ ] busybox klogd  [ ] none
- Log shipping: [ ] tail to TBOS channel  [ ] HTTP(S) push  [ ] MQTT  [ ] none
- Health checks: [ ] tbos-watchdog  [ ] process heartbeats  [ ] disk/net probes

---

## 9) Update Strategy
- Runtime updates: [ ] apk (online)  [ ] disable
- OS updates: [ ] A/B images  [ ] reflash-only  [ ] TBOS-provided bundles
- Rollback: [ ] auto on failure  [ ] manual

---

## 10) UX & Localization
- Default locale/timezone: ______ / ______
- Prompt theme and MOTD: ______________________
- Banners/branding: ASCII art path or text

---

## 11) Integration Hooks
- TBOS → Linux handoff params (key/value to pass on cmdline or env):
```
TBOS_NODE_ID=____ TBOS_FLEET=____ STEPPPS_MODE=____
```
- Linux → TBOS callbacks (files, sockets, or serial protocol):
```
/path/to/callback.sock or /dev/ttyS0 protocol spec
```

---

## 12) Hardware Addenda
List per-platform quirks (DTB requirements, special drivers, firmware blobs, GPU/FB settings, storage peculiarities):
```
<board1>:
  - DTB: ____
  - UART: ____
  - NIC: ____
  - Storage: ____

<board2>:
  ...
```

---

## 13) Acceptance Criteria
- Boot time target (cold-to-shell): ____ seconds
- Memory footprint (idle): ____ MB
- Disk footprint (bundled): ____ MB
- Must-pass tests:
  - [ ] Console accessible
  - [ ] Network up (DHCP)
  - [ ] SSH reachable (if enabled)
  - [ ] TBOS shell functional
  - [ ] Persistence works (if enabled)

---

## 14) Phase & Priority
Enumerate items with P0/P1/P2 and phase them (v1, v1.1, v2):
```
P0 v1:
- ...
P1 v1.1:
- ...
P2 v2:
- ...
```
