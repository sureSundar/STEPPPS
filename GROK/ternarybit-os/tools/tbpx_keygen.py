#!/usr/bin/env python3
"""
TBPX key generator for Ed25519 (raw 32-byte keys)

Generates a raw 32-byte secret key and a raw 32-byte public key suitable for
use with tools/tbpx_codec.py --sign/--verify.

Usage:
  python3 tools/tbpx_keygen.py --out-dir keys
  python3 tools/tbpx_keygen.py --secret keys/ed25519_sk.bin --public keys/ed25519_pk.bin

Notes:
- Requires PyNaCl: pip install pynacl
- Files are written in binary (raw 32 bytes each). Keep the secret key safe.
"""

import argparse
import os
import sys

try:
    from nacl.signing import SigningKey
    _PNACL = True
except Exception:
    SigningKey = None  # type: ignore
    _PNACL = False


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description="Generate Ed25519 keys for TBPX signing")
    p.add_argument("--out-dir", help="Directory to write ed25519_sk.bin and ed25519_pk.bin")
    p.add_argument("--secret", help="Path to write secret key (raw 32 bytes)")
    p.add_argument("--public", help="Path to write public key (raw 32 bytes)")
    args = p.parse_args(argv)

    if not _PNACL:
        print("Error: PyNaCl is required. Install with: pip install pynacl", file=sys.stderr)
        return 2

    sk = SigningKey.generate()
    pk = sk.verify_key
    sk_raw = bytes(sk)[:32]
    pk_raw = bytes(pk)

    if args.out_dir:
        os.makedirs(args.out_dir, exist_ok=True)
        secret_path = os.path.join(args.out_dir, "ed25519_sk.bin")
        public_path = os.path.join(args.out_dir, "ed25519_pk.bin")
    else:
        if not args.secret or not args.public:
            print("Specify --out-dir OR both --secret and --public", file=sys.stderr)
            return 2
        secret_path = args.secret
        public_path = args.public

    with open(secret_path, "wb") as f:
        f.write(sk_raw)
    with open(public_path, "wb") as f:
        f.write(pk_raw)

    print(f"Wrote secret key (32 bytes): {secret_path}")
    print(f"Wrote public key (32 bytes): {public_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
