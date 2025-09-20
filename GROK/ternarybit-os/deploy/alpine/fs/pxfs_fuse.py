#!/usr/bin/env python3
import errno
import logging
import os
import stat
from datetime import datetime
from typing import Tuple

try:
    from fuse import FUSE, Operations  # pip install fusepy
except Exception as e:
    raise SystemExit("fusepy not available. Ensure python3 and fusepy are installed.")

LOG = logging.getLogger("pxfs")

SEP_TOKEN_PREFIX = "__PX_"  # e.g., __PX_255_0_0__


def parse_sep(sep: str) -> Tuple[int, int, int]:
    parts = [int(x) for x in sep.split(",")]
    if len(parts) != 3:
        raise ValueError("sep must be R,G,B")
    for p in parts:
        if p < 0 or p > 255:
            raise ValueError("RGB out of range")
    return tuple(parts)  # type: ignore


def sep_token(rgb: Tuple[int, int, int]) -> str:
    return f"{SEP_TOKEN_PREFIX}{rgb[0]}_{rgb[1]}_{rgb[2]}__"


class PXFS(Operations):
    def __init__(self, backing: str, sep_rgb: Tuple[int, int, int]):
        self.backing = os.path.abspath(backing)
        self.sep = sep_token(sep_rgb)
        os.makedirs(self.backing, exist_ok=True)

    def _b(self, path: str) -> str:
        # Translate path components back into backing names using pixel token
        comps = [c for c in path.split("/") if c]
        if not comps:
            return self.backing
        encoded = self.sep.join(comps)
        return os.path.join(self.backing, encoded)

    def getattr(self, path, fh=None):
        b = self._b(path)
        st = os.lstat(self.backing) if path == "/" else os.lstat(b) if os.path.exists(b) else None
        if path == "/":
            # fake root dir attrs based on backing
            st = os.lstat(self.backing)
            return dict((key, getattr(st, key)) for key in ("st_mode", "st_ino", "st_dev", "st_nlink", "st_uid", "st_gid", "st_size", "st_atime", "st_mtime", "st_ctime"))
        if st is None:
            raise OSError(errno.ENOENT, "not found")
        return dict((key, getattr(st, key)) for key in ("st_mode", "st_ino", "st_dev", "st_nlink", "st_uid", "st_gid", "st_size", "st_atime", "st_mtime", "st_ctime"))

    def readdir(self, path, fh):
        b = self._b(path)
        yield from [".", ".."]
        if not os.path.exists(b):
            return
        name = os.path.basename(b)
        if path == "/":
            # root lists top-level entries by splitting encoded names on sep
            for entry in os.listdir(self.backing):
                # show first component of each encoded name
                parts = entry.split(self.sep)
                if parts:
                    yield parts[0]
        else:
            # deeper levels: list next component(s) available under this prefix
            prefix = name + self.sep if name else ""
            seen = set()
            for entry in os.listdir(self.backing):
                if not entry.startswith(prefix):
                    continue
                remainder = entry[len(prefix):]
                parts = remainder.split(self.sep)
                if parts:
                    seen.add(parts[0])
            for p in sorted(seen):
                yield p

    # File operations map to backing encoded filename
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

    def truncate(self, path, length, fh=None):
        b = self._b(path)
        with open(b, "r+") as f:
            f.truncate(length)

    def unlink(self, path):
        os.unlink(self._b(path))

    def mkdir(self, path, mode):
        # Translate directories by creating a marker file to anchor the prefix
        b = self._b(path)
        os.makedirs(os.path.dirname(b), exist_ok=True)
        # Create a directory by creating a placeholder directory with suffix ".d"
        os.makedirs(b + ".d", exist_ok=True)

    def rmdir(self, path):
        b = self._b(path)
        try:
            os.rmdir(b + ".d")
        except FileNotFoundError:
            pass


def main():
    import argparse
    logging.basicConfig(level=logging.INFO, format='[%(levelname)s] %(message)s')
    p = argparse.ArgumentParser()
    p.add_argument("mountpoint")
    p.add_argument("--backing", default="/var/lib/pxfs")
    p.add_argument("--sep", default="255,255,255", help="RGB separator, e.g., 255,0,0")
    args = p.parse_args()

    fs = PXFS(args.backing, parse_sep(args.sep))
    FUSE(fs, args.mountpoint, nothreads=True, foreground=True)

if __name__ == "__main__":
    main()
