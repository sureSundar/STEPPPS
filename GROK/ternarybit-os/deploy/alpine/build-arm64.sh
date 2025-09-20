#!/usr/bin/env bash
set -euo pipefail

# Build minimal arm64 Alpine initramfs + provide kernel cmdline. Kernel (Image.gz with EFI stub) should be provided
# by your pipeline, plus DTB(s) if needed. This script focuses on the initramfs.

ARCH=aarch64
ALPINE_VER=3.20
OUT_DIR=$(pwd)/out/${ARCH}
WORK_DIR=$(pwd)/work/${ARCH}
MINIROOT_URL=https://dl-cdn.alpinelinux.org/alpine/v${ALPINE_VER}/releases/${ARCH}/alpine-minirootfs-${ALPINE_VER}.2-${ARCH}.tar.gz

rm -rf "${WORK_DIR}" "${OUT_DIR}"
mkdir -p "${WORK_DIR}" "${OUT_DIR}"

echo "[*] Fetching Alpine minirootfs for ${ARCH}..."
curl -fsSL "${MINIROOT_URL}" -o "${WORK_DIR}/minirootfs.tar.gz"

echo "[*] Extracting..."
mkdir -p "${WORK_DIR}/rootfs"
tar -xzf "${WORK_DIR}/minirootfs.tar.gz" -C "${WORK_DIR}/rootfs"

# Install our init and shell
install -Dm755 common/steppps-init "${WORK_DIR}/rootfs/sbin/steppps-init"
install -Dm755 common/tbos-shell "${WORK_DIR}/rootfs/usr/bin/tbos-shell"
install -Dm755 common/tbos-firstboot "${WORK_DIR}/rootfs/usr/bin/tbos-firstboot"

# Stage FS prototypes and helpers (will be copied to /usr/bin by tbos-firstboot)
install -d "${WORK_DIR}/rootfs/usr/local/lib/tbos/fs/helpers" "${WORK_DIR}/rootfs/usr/local/lib/tbos/fs/fuse"
[ -f fs/pxfs_fuse.py ] && install -Dm755 fs/pxfs_fuse.py "${WORK_DIR}/rootfs/usr/local/lib/tbos/fs/fuse/pxfs_fuse.py"
[ -f fs/ucfs_fuse.py ] && install -Dm755 fs/ucfs_fuse.py "${WORK_DIR}/rootfs/usr/local/lib/tbos/fs/fuse/ucfs_fuse.py"
[ -f fs/rf2s_fuse.py ] && install -Dm755 fs/rf2s_fuse.py "${WORK_DIR}/rootfs/usr/local/lib/tbos/fs/fuse/rf2s_fuse.py"
[ -d fs/helpers ] && cp -a fs/helpers/. "${WORK_DIR}/rootfs/usr/local/lib/tbos/fs/helpers/" 2>/dev/null || true

# Sensors CLI
install -Dm755 sensors/tbos-sensorctl.py "${WORK_DIR}/rootfs/usr/bin/tbos-sensorctl"
install -Dm755 sensors/tbos-sensord-wifi.py "${WORK_DIR}/rootfs/usr/bin/tbos-sensord-wifi"
install -Dm755 sensors/tbos-sensord-mic.py "${WORK_DIR}/rootfs/usr/bin/tbos-sensord-mic"
install -Dm755 sensors/tbos-sensord-bt.py "${WORK_DIR}/rootfs/usr/bin/tbos-sensord-bt"

# Sensors CLI
install -Dm755 sensors/tbos-sensorctl.py "${WORK_DIR}/rootfs/usr/bin/tbos-sensorctl"

# Ensure init points to our init
ln -sf /sbin/steppps-init "${WORK_DIR}/rootfs/init" || true

# Create initramfs
echo "[*] Creating initramfs..."
( cd "${WORK_DIR}/rootfs" && find . -print0 | cpio --null -ov --format=newc | gzip -9 ) > "${OUT_DIR}/initramfs.cpio.gz"

# Provide kernel cmdline example
cp kernel-cmdline-arm64.txt "${OUT_DIR}/kernel-cmdline.txt"

echo "[+] Done. Place your Image.gz and DTBs alongside and boot via EFI or TBOS bare-metal handoff with the cmdline above."
