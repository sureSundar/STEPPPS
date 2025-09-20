#!/usr/bin/env python3
import errno
import logging
import os
import stat
import urllib.parse
from typing import Optional

try:
    from fuse import FUSE, Operations  # pip install fusepy
except Exception:
    raise SystemExit("fusepy not available. Ensure python3 and fusepy are installed.")

LOG = logging.getLogger("ucfs")

ENC_PREFIX = "__UC_"  # __UC_<hex>__ where <hex> is UTF-8 bytes hex-encoded


def enc_delim(ch: str) -> str:
    b = ch.encode("utf-8")
    return f"{ENC_PREFIX}{b.hex()}__"


class UCFS(Operations):
    def __init__(self, backing: str, delim: str):
        if len(delim) == 0:
            raise ValueError("delimiter must be one UTF-8 character")
        self.backing = os.path.abspath(backing)
        self.delim = delim
        self.token = enc_delim(delim)
        os.makedirs(self.backing, exist_ok=True)

    def _b(self, path: str) -> str:
        comps = [c for c in path.split("/") if c]
        if not comps:
            return self.backing
        encoded = self.token.join(comps)
        return os.path.join(self.backing, encoded)

    def getattr(self, path, fh=None):
        if path == "/":
            st = os.lstat(self.backing)
        else:
            b = self._b(path)
            if not os.path.exists(b):
                raise OSError(errno.ENOENT, "not found")
            st = os.lstat(b)
        return {k: getattr(st, k) for k in ("st_mode","st_ino","st_dev","st_nlink","st_uid","st_gid","st_size","st_atime","st_mtime","st_ctime")}

    def readdir(self, path, fh):
        yield from [".", ".."]
        if path == "/":
            # list first components
            for entry in os.listdir(self.backing):
                parts = entry.split(self.token)
                if parts:
                    yield parts[0]
        else:
            b = self._b(path)
            name = os.path.basename(b)
            prefix = name + self.token if name else ""
            seen = set()
            for entry in os.listdir(self.backing):
                if not entry.startswith(prefix):
                    continue
                remainder = entry[len(prefix):]
                parts = remainder.split(self.token)
                if parts:
                    seen.add(parts[0])
            for p in sorted(seen):
                yield p

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
        b = self._b(path)
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
    p.add_argument("--backing", default="/var/lib/ucfs")
    p.add_argument("--delim", default="·", help="Unicode delimiter char (default middle dot)")
    args = p.parse_args()

    fs = UCFS(args.backing, args.delim)
    FUSE(fs, args.mountpoint, nothreads=True, foreground=True)

if __name__ == "__main__":
    main()
