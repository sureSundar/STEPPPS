#!/usr/bin/env python3
import errno
import logging
import os
from typing import Optional

try:
    from fuse import FUSE, Operations  # pip install fusepy
except Exception:
    raise SystemExit("fusepy not available. Ensure python3 and fusepy are installed.")

LOG = logging.getLogger("pf2s")

# PF2S: Photon Frequency/Wavelength Filesystem
# Directory names are wavelengths in nm (normalized) or frequencies in THz (suffix 'THz').
# Example tree:
#   /mnt/pf/1550.120000/   (nm normalized with 6 decimals)
#   /mnt/pf/193.414THz/    (THz suffix)


def normalize_wavelength(name: str) -> str:
    # Accept forms like '1550.12' (nm) or '193.4THz'
    s = name.strip()
    if s.endswith('THz') or s.endswith('thz'):
        # Keep as-is but normalize float formatting before suffix
        v = float(s[:-3])
        return f"{v:.3f}THz"
    # Default nm
    v = float(s)
    return f"{v:.6f}"


class PF2S(Operations):
    def __init__(self, backing: str):
        self.backing = os.path.abspath(backing)
        os.makedirs(self.backing, exist_ok=True)

    def _b(self, path: str) -> str:
        comps = [c for c in path.split('/') if c]
        if not comps:
            return self.backing
        first = normalize_wavelength(comps[0])
        if len(comps) == 1:
            return os.path.join(self.backing, first)
        return os.path.join(self.backing, first, *comps[1:])

    def getattr(self, path, fh=None):
        if path == '/':
            st = os.lstat(self.backing)
        else:
            b = self._b(path)
            if not os.path.exists(b):
                # Treat top-level wavelength as a virtual dir
                comps = [c for c in path.split('/') if c]
                if len(comps) == 1:
                    import stat as pystat
                    return {"st_mode": pystat.S_IFDIR | 0o755, "st_nlink": 2}
                raise OSError(errno.ENOENT, 'not found')
            st = os.lstat(b)
        return {k: getattr(st, k) for k in ("st_mode","st_ino","st_dev","st_nlink","st_uid","st_gid","st_size","st_atime","st_mtime","st_ctime")}

    def readdir(self, path, fh):
        yield from ['.', '..']
        if path == '/':
            for d in os.listdir(self.backing):
                yield d
        else:
            b = self._b(path)
            if os.path.isdir(b):
                for e in os.listdir(b):
                    yield e

    def mkdir(self, path, mode):
        b = self._b(path)
        os.makedirs(b, exist_ok=True)

    def rmdir(self, path):
        b = self._b(path)
        os.rmdir(b)

    def open(self, path, flags):
        b = self._b(path)
        if not os.path.exists(b):
            raise OSError(errno.ENOENT, 'not found')
        return os.open(b, flags)

    def create(self, path, mode, fi=None):
        b = self._b(path)
        os.makedirs(os.path.dirname(b), exist_ok=True)
        return os.open(b, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, mode)

    def read(self, path, size, offset, fh):
        os.lseek(fh, offset, os.SEEK_SET)
        return os.read(fh, size)

    def write(self, path, data, offset, fh):
        os.lseek(fh, offset, os.SEEK_SET)
        return os.write(fh, data)

    def unlink(self, path):
        os.unlink(self._b(path))


def main():
    import argparse
    logging.basicConfig(level=logging.INFO, format='[%(levelname)s] %(message)s')
    p = argparse.ArgumentParser()
    p.add_argument('mountpoint')
    p.add_argument('--backing', default='/var/lib/pf2s')
    args = p.parse_args()

    fs = PF2S(args.backing)
    FUSE(fs, args.mountpoint, nothreads=True, foreground=True)

if __name__ == '__main__':
    main()
