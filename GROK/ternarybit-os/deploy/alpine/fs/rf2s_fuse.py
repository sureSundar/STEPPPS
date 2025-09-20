#!/usr/bin/env python3
import errno
import logging
import os
from typing import Optional

try:
    from fuse import FUSE, Operations  # pip install fusepy
except Exception:
    raise SystemExit("fusepy not available. Ensure python3 and fusepy are installed.")

LOG = logging.getLogger("rf2s")


def normalize_freq(name: str, unit: str) -> str:
    # Accept numeric names; normalize to fixed decimals depending on unit
    try:
        v = float(name)
    except ValueError:
        raise OSError(errno.ENOENT, "invalid frequency dir")
    if unit == "Hz":
        return f"{v:.0f}"
    if unit == "kHz":
        return f"{v:.3f}"
    if unit == "MHz":
        return f"{v:.6f}"
    if unit == "GHz":
        return f"{v:.9f}"
    return f"{v:.6f}"


class RF2S(Operations):
    def __init__(self, backing: str, unit: str = "MHz"):
        self.backing = os.path.abspath(backing)
        self.unit = unit
        os.makedirs(self.backing, exist_ok=True)

    def _b(self, path: str) -> str:
        comps = [c for c in path.split("/") if c]
        if not comps:
            return self.backing
        # first component must be frequency
        norm = normalize_freq(comps[0], self.unit)
        if len(comps) == 1:
            return os.path.join(self.backing, norm)
        return os.path.join(self.backing, norm, *comps[1:])

    def getattr(self, path, fh=None):
        if path == "/":
            st = os.lstat(self.backing)
        else:
            b = self._b(path)
            if not os.path.exists(b):
                # If referencing a frequency dir that doesn't exist yet, treat as dir
                comps = [c for c in path.split("/") if c]
                if len(comps) == 1:
                    # virtual freq dir
                    import stat as pystat
                    return {"st_mode": pystat.S_IFDIR | 0o755, "st_nlink": 2}
                raise OSError(errno.ENOENT, "not found")
            st = os.lstat(b)
        return {k: getattr(st, k) for k in ("st_mode","st_ino","st_dev","st_nlink","st_uid","st_gid","st_size","st_atime","st_mtime","st_ctime")}

    def readdir(self, path, fh):
        yield from [".", ".."]
        if path == "/":
            # list available frequency dirs in backing
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
            raise OSError(errno.ENOENT, "not found")
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
    p.add_argument("mountpoint")
    p.add_argument("--backing", default="/var/lib/rf2s")
    p.add_argument("--unit", choices=["Hz","kHz","MHz","GHz"], default="MHz")
    args = p.parse_args()

    fs = RF2S(args.backing, args.unit)
    FUSE(fs, args.mountpoint, nothreads=True, foreground=True)

if __name__ == "__main__":
    main()
