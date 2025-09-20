#!/usr/bin/env python3
"""
TBPX (TBOS Pixel Codec) reference encoder/decoder - Mode L (Lossless)

- Container: PPM P6 (binary, lossless) to avoid external dependencies
- Packing: 3 bytes per pixel mapped to RGB in row-major order
- Header: 48-byte fixed header stored in first 16 pixels (redundant copy optional)

CLI:
  Encode: tbpx_codec.py encode <input.bin> <output.ppm> [--width 256] [--repeat-header]
  Decode: tbpx_codec.py decode <input.ppm> <output.bin>

This implements the v0 Mode L spec appended to:
  STEPPPS/GROK/ternarybit-os/docs/PIXEL_CODEC_SPEC.md
"""

import argparse
import math
import os
import struct
import sys
import zlib
from typing import Optional, Tuple

# Optional Pillow for PNG support
try:
    from PIL import Image  # type: ignore
    _PIL_AVAILABLE = True
except Exception:
    Image = None  # type: ignore
    _PIL_AVAILABLE = False

MAGIC = b"TBPX"
VERSION = 0x01
FLAG_MODE_L = 0x01
FLAG_MODE_R = 0x02
FLAG_RS_PRESENT = 0x10
# New flags for integrity/authentication (optional)
FLAG_SIG_PRESENT = 0x20
FLAG_SHA256_PRESENT = 0x40
HEADER_SIZE = 48  # bytes
HEADER_PIXELS = HEADER_SIZE // 3  # 16 pixels
COLOR_ORDER = 0  # 0=RGB


class TBPXError(Exception):
    pass


def _crc32(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def _pack_header(payload_length: int, pad_count: int, flags: int, header_repeat_count: int, tile_info: int = 0) -> bytes:
    # Fields:
    # 1. Magic: 4 bytes
    # 2. Version: 1 byte
    # 3. Flags: 1 byte
    # 4. Payload_length: 8 bytes LE
    # 5. Checksum: 4 bytes (CRC32 of raw payload)
    # 6. ECC_descriptor: 2 bytes (0)
    # 7. Tile_info: 4 bytes (0)
    # 8. Color_order: 1 byte (0=RGB)
    # 9. Pad_count: 1 byte (0-2)
    # 10. Header_hash: 4 bytes (CRC32 over fields 2-9)
    # 11. Header_repeat_count: 1 byte
    # 12. Reserved: pad to 48 bytes total (set to 0)
    
    # Compute checksum placeholder (0) now; real checksum set by caller using payload data.
    # We build without header_hash first, then compute header_hash and rebuild.
    checksum = 0
    ecc_desc = 0

    # Build fields 2-9 first (for header_hash)
    fields_2_to_9 = struct.pack(
        "<BBQ I H I BB",
        VERSION,
        flags,
        payload_length,
        checksum,
        ecc_desc,
        tile_info,
        COLOR_ORDER,
        pad_count,
    )
    header_hash = _crc32(fields_2_to_9)

    # Now build full header parts
    head = bytearray()
    head += MAGIC
    head += fields_2_to_9
    head += struct.pack("<I", header_hash)
    head += struct.pack("<B", header_repeat_count)
    # Reserved padding to 48 bytes
    if len(head) > HEADER_SIZE:
        raise TBPXError("Header overrun")
    head += b"\x00" * (HEADER_SIZE - len(head))
    return bytes(head)


def _update_header_checksum(header: bytes, checksum: int) -> bytes:
    # Update the checksum field at offset after MAGIC+VERSION+FLAGS+PAYLOAD_LENGTH
    # Offsets:
    # MAGIC(4), VERSION(1), FLAGS(1), PAYLOAD_LENGTH(8) => 14 bytes
    # Next 4 bytes are checksum
    if len(header) != HEADER_SIZE:
        raise TBPXError("Invalid header size for checksum update")
    head = bytearray(header)
    struct.pack_into("<I", head, 14, checksum)
    # Recompute header_hash over fields 2-9 => starting at offset 4, length of fields_2_to_9 is 1+1+8+4+2+4+1+1 = 22 bytes
    fields_2_to_9 = bytes(head[4:4+22])
    header_hash = _crc32(fields_2_to_9)
    struct.pack_into("<I", head, 4+22, header_hash)  # header_hash position
    return bytes(head)


def _ppm_write(path: str, width: int, height: int, rgb_bytes: bytes) -> None:
    expected = width * height * 3
    if len(rgb_bytes) != expected:
        raise TBPXError(f"PPM write length mismatch: have {len(rgb_bytes)}, expected {expected}")
    with open(path, "wb") as f:
        header = f"P6\n{width} {height}\n255\n".encode("ascii")
        f.write(header)
        f.write(rgb_bytes)


def _ppm_read(path: str) -> Tuple[int, int, bytes]:
    with open(path, "rb") as f:
        data = f.read()
    # Parse simple P6 header, allow comments and flexible whitespace
    if not data.startswith(b"P6"):
        raise TBPXError("Not a P6 PPM file")
    # Tokenize
    i = 2
    def _skip_ws_and_comments(idx: int) -> int:
        while idx < len(data):
            c = data[idx:idx+1]
            if c in b" \t\r\n":
                idx += 1
                continue
            if c == b"#":
                # skip to end of line
                while idx < len(data) and data[idx:idx+1] not in (b"\n", b"\r"):
                    idx += 1
                continue
            break
        return idx
    i = _skip_ws_and_comments(i)
    # Read width
    j = i
    while j < len(data) and data[j:j+1] in b"0123456789":
        j += 1
    width = int(data[i:j])
    i = _skip_ws_and_comments(j)
    # Read height
    j = i
    while j < len(data) and data[j:j+1] in b"0123456789":
        j += 1
    height = int(data[i:j])
    i = _skip_ws_and_comments(j)
    # Read maxval
    j = i
    while j < len(data) and data[j:j+1] in b"0123456789":
        j += 1
    maxval = int(data[i:j])
    if maxval != 255:
        raise TBPXError("PPM maxval must be 255")
    # Next byte should be a single whitespace character separating header from data
    if j >= len(data) or data[j:j+1] not in b" \t\r\n":
        raise TBPXError("Malformed PPM header")
    i = j + 1
    rgb = data[i:]
    if len(rgb) != width * height * 3:
        raise TBPXError("PPM data size mismatch")
    return width, height, rgb


def _pil_write_generic(path: str, width: int, height: int, rgb_bytes: bytes, fmt: str) -> None:
    if not _PIL_AVAILABLE:
        raise TBPXError(f"{fmt} support requires Pillow. Install with: pip install Pillow")
    if len(rgb_bytes) != width * height * 3:
        raise TBPXError(f"{fmt} write length mismatch")
    img = Image.frombytes("RGB", (width, height), rgb_bytes)
    fmt_upper = fmt.upper()
    save_kwargs = {}
    if fmt_upper == "TIFF":
        # Use lossless compression
        save_kwargs = {"compression": "tiff_deflate"}
    elif fmt_upper == "WEBP":
        save_kwargs = {"lossless": True, "quality": 100}
    elif fmt_upper == "PNG":
        save_kwargs = {}
    elif fmt_upper == "BMP":
        save_kwargs = {}
    else:
        # Default: rely on Pillow defaults
        save_kwargs = {}
    img.save(path, format=fmt_upper, **save_kwargs)


def _png_write(path: str, width: int, height: int, rgb_bytes: bytes) -> None:
    if not _PIL_AVAILABLE:
        raise TBPXError("PNG support requires Pillow. Install with: pip install Pillow")
    if len(rgb_bytes) != width * height * 3:
        raise TBPXError("PNG write length mismatch")
    img = Image.frombytes("RGB", (width, height), rgb_bytes)
    img.save(path, format="PNG")


def _png_read(path: str) -> Tuple[int, int, bytes]:
    if not _PIL_AVAILABLE:
        raise TBPXError("PNG support requires Pillow. Install with: pip install Pillow")
    img = Image.open(path)
    img = img.convert("RGB")
    width, height = img.size
    rgb = img.tobytes()
    return width, height, rgb


def _sha256(data: bytes) -> bytes:
    import hashlib
    return hashlib.sha256(data).digest()


def _load_ed25519_priv(path: str):
    try:
        from nacl.signing import SigningKey  # type: ignore
    except Exception as e:
        raise TBPXError("Ed25519 signing requires PyNaCl. Install with: pip install pynacl")
    with open(path, 'rb') as f:
        raw = f.read()
    # Support raw 32-byte secret key or PEM-like? Keep simple: 32 bytes raw
    if len(raw) < 32:
        raise TBPXError("Invalid Ed25519 private key (expected >=32 bytes)")
    sk = SigningKey(raw[:32])
    return sk


def _load_ed25519_pub(path: str):
    try:
        from nacl.signing import VerifyKey  # type: ignore
    except Exception as e:
        raise TBPXError("Ed25519 verification requires PyNaCl. Install with: pip install pynacl")
    with open(path, 'rb') as f:
        raw = f.read()
    # Support raw 32-byte public key
    if len(raw) < 32:
        raise TBPXError("Invalid Ed25519 public key (expected >=32 bytes)")
    vk = VerifyKey(raw[:32])
    return vk


def _build_ext_block(payload: bytes, want_sha256: bool, sign_priv_path: Optional[str]) -> Tuple[bytes, int]:
    """Return (ext_block_bytes, ext_flags_to_or). Uses simple fixed layout:
    TBEX(4) | EXT_FLAGS(1) | [SHA256(32)?] | [KEY_ID(8) | SIG(64)?]
    key_id = first 8 bytes of sha256(pubkey_bytes)
    Signature is Ed25519 over SHA256(payload)
    """
    ext = bytearray()
    ext_flags = 0
    if not want_sha256 and not sign_priv_path:
        return b"", 0
    ext += b"TBEX"
    # Prepare digest
    digest = _sha256(payload) if want_sha256 or sign_priv_path else b""
    if want_sha256:
        ext_flags |= 0x01
    if sign_priv_path:
        ext_flags |= 0x02
    ext += bytes([ext_flags])
    if want_sha256:
        ext += digest
    if sign_priv_path:
        sk = _load_ed25519_priv(sign_priv_path)
        vk = sk.verify_key
        pub = bytes(vk)
        key_id = _sha256(pub)[:8]
        sig = sk.sign(digest).signature  # 64 bytes
        ext += key_id
        ext += sig
    return bytes(ext), ((FLAG_SHA256_PRESENT if want_sha256 else 0) | (FLAG_SIG_PRESENT if sign_priv_path else 0))


def encode_to_ppm(in_bytes: bytes, width: int = 256, repeat_header: bool = False, want_sha256: bool = False, sign_priv_path: Optional[str] = None) -> Tuple[int, int, bytes]:
    payload_len = len(in_bytes)
    pad_count = (3 - (payload_len % 3)) % 3
    checksum = _crc32(in_bytes)

    # Build header with flags
    flags = FLAG_MODE_L
    # Build optional extension block (integrity/signature)
    ext_block, add_flags = _build_ext_block(in_bytes, want_sha256, sign_priv_path)
    flags |= add_flags
    tile_info = len(ext_block)
    header = _pack_header(payload_len, pad_count, flags, 1 if repeat_header else 0, tile_info)
    header = _update_header_checksum(header, checksum)

    # Base stream without considering placement of redundant header
    base_stream = bytearray()
    base_stream += header
    # Append extension block if present
    if ext_block:
        base_stream += ext_block
    base_stream += in_bytes
    if pad_count:
        base_stream += b"\x00" * pad_count

    # Compute total bytes required including trailing header (if any)
    total_bytes = len(base_stream) + (HEADER_SIZE if repeat_header else 0)
    total_pixels = math.ceil(total_bytes / 3)
    height = math.ceil(total_pixels / width)
    total_capacity = width * height * 3

    # Allocate full image buffer (zero-initialized)
    stream = bytearray(total_capacity)
    # Write primary header + payload (+pad) at the start
    stream[0:len(base_stream)] = base_stream
    # If repeat header, write redundant header at the very end of image
    if repeat_header:
        stream[-HEADER_SIZE:] = header

    return width, height, bytes(stream)


def _parse_ext_block(ext: bytes, flags: int, verify_pub_path: Optional[str], payload: bytes) -> None:
    """Validate extension block if present. Raise TBPXError on mismatch when verification requested or hashes present."""
    if len(ext) == 0:
        return
    if not ext.startswith(b"TBEX") or len(ext) < 5:
        raise TBPXError("Invalid extension block")
    ptr = 4
    ext_flags = ext[ptr]
    ptr += 1
    have_sha = (ext_flags & 0x01) != 0
    have_sig = (ext_flags & 0x02) != 0
    # Sanity vs header flags
    if have_sha != bool(flags & FLAG_SHA256_PRESENT):
        raise TBPXError("Extension/header SHA256 flag mismatch")
    if have_sig != bool(flags & FLAG_SIG_PRESENT):
        raise TBPXError("Extension/header SIG flag mismatch")
    # SHA256
    digest = _sha256(payload) if have_sha or have_sig else b""
    if have_sha:
        if len(ext) < ptr + 32:
            raise TBPXError("Truncated SHA256 in extension")
        sha_in = ext[ptr:ptr+32]
        ptr += 32
        if sha_in != digest:
            raise TBPXError("SHA256 mismatch; payload corrupted")
    # Signature
    if have_sig:
        if len(ext) < ptr + 8 + 64:
            raise TBPXError("Truncated signature block in extension")
        key_id = ext[ptr:ptr+8]
        ptr += 8
        sig = ext[ptr:ptr+64]
        ptr += 64
        if verify_pub_path is None:
            # If verification key not provided, we accept but note integrity from SHA if present
            return
        vk = _load_ed25519_pub(verify_pub_path)
        pub = bytes(vk)
        kid_calc = _sha256(pub)[:8]
        if kid_calc != key_id:
            raise TBPXError("Signature key-id mismatch")
        try:
            vk.verify(sig + digest)  # PyNaCl expects signed message form? Actually VerifyKey.verify(signed_message) returns message.
            # However we only have signature and digest; reconstruct signed message = sig + message
        except Exception:
            # Try explicit API: verify(signature, message) not available in PyNaCl; rebuild signed msg
            try:
                from nacl.signing import SignedMessage  # type: ignore
            except Exception:
                pass
            # Fallback manual check
            try:
                vk.verify(digest, sig)  # type: ignore
            except Exception:
                raise TBPXError("Signature verification failed")


def decode_from_ppm(rgb_bytes: bytes, verify_pub_path: Optional[str] = None) -> bytes:
    # Expect at least one header present in the first HEADER_SIZE bytes
    if len(rgb_bytes) < HEADER_SIZE:
        raise TBPXError("Image too small to contain header")
    header_primary = rgb_bytes[:HEADER_SIZE]

    def _parse_header(hdr: bytes) -> Tuple[int, int, int, int, int, int]:
        if len(hdr) != HEADER_SIZE:
            raise TBPXError("Invalid header size")
        if hdr[:4] != MAGIC:
            raise TBPXError("Magic mismatch")
        # Unpack fields 2-9 and then header_hash, header_repeat_count
        version, flags, payload_len, checksum, ecc_desc, tile_info, color_order, pad_count = struct.unpack(
            "<BBQ I H I BB", hdr[4:4+22]
        )
        header_hash = struct.unpack("<I", hdr[4+22:4+22+4])[0]
        repeat_count = hdr[4+22+4]
        # Validate header hash
        if _crc32(hdr[4:4+22]) != header_hash:
            raise TBPXError("Header hash mismatch")
        if version != VERSION:
            raise TBPXError(f"Unsupported version: {version}")
        if (flags & FLAG_MODE_L) == 0:
            raise TBPXError("Not a Mode L image")
        if color_order != COLOR_ORDER:
            raise TBPXError("Unsupported color order")
        if pad_count not in (0, 1, 2):
            raise TBPXError("Invalid pad_count")
        return payload_len, checksum, pad_count, repeat_count, flags, tile_info

    # Try primary header;
    try:
        payload_len, checksum, pad_count, repeat_count, flags, tile_info = _parse_header(header_primary)
        header_ok = True
        header_at_end = False
    except TBPXError:
        header_ok = False
        repeat_count = 0

    # If primary invalid or repeat_count suggests trailing header, try trailing header at end
    if not header_ok or repeat_count > 0:
        # Last HEADER_SIZE bytes form another header candidate
        hdr_end = rgb_bytes[-HEADER_SIZE:]
        try:
            payload_len2, checksum2, pad_count2, _, flags2, tile_info2 = _parse_header(hdr_end)
            # Use trailing header if primary invalid, else prefer primary
            if not header_ok:
                payload_len, checksum, pad_count, flags, tile_info = payload_len2, checksum2, pad_count2, flags2, tile_info2
                header_ok = True
                header_at_end = True
        except TBPXError:
            if not header_ok:
                raise TBPXError("No valid header found (primary and trailing invalid)")

    # Extract payload: header occupies first HEADER_SIZE bytes; if trailing header present, ignore it in payload area
    # Payload starts immediately after primary header. We packed: [header][payload][pad][optional trailing header][final zero fill]
    # So read bytes from after HEADER to (HEADER + payload_len + pad_count).
    ext_len = tile_info if (flags & (FLAG_SHA256_PRESENT | FLAG_SIG_PRESENT | FLAG_RS_PRESENT)) else tile_info
    # For now, tile_info denotes extension length (may be 0)
    start = HEADER_SIZE + ext_len
    end = start + payload_len + pad_count
    if end > len(rgb_bytes):
        raise TBPXError("Image truncated; payload exceeds data size")
    payload_padded = rgb_bytes[start:end]
    payload = payload_padded[:payload_len]
    if _crc32(payload) != checksum:
        raise TBPXError("CRC mismatch; payload corrupted")
    # If extension present, validate according to flags
    if ext_len:
        ext_block = rgb_bytes[HEADER_SIZE:HEADER_SIZE+ext_len]
        _parse_ext_block(ext_block, flags, verify_pub_path, payload)
    return payload


def _read_image_auto(path: str) -> Tuple[int, int, bytes]:
    # Detect PNG by magic, PPM P6 by header, else fall back to extension
    try:
        with open(path, 'rb') as f:
            sig = f.read(8)
    except FileNotFoundError:
        raise TBPXError(f"File not found: {path}")
    # PNG magic
    if sig.startswith(b"\x89PNG\r\n\x1a\n"):
        return _png_read(path)
    # PPM P6 starts with 'P6'
    if sig.startswith(b"P6"):
        return _ppm_read(path)
    # Fallback by extension
    ext = os.path.splitext(path)[1].lower()
    if ext == '.png':
        return _png_read(path)
    if ext in ('.ppm', '.pnm'):
        return _ppm_read(path)
    # If Pillow available, load generically (handles BMP, TIFF, WEBP, PNG, etc.)
    if _PIL_AVAILABLE:
        img = Image.open(path)
        img = img.convert("RGB")
        w, h = img.size
        return w, h, img.tobytes()
    # Try PPM then PNG as last resort
    try:
        return _ppm_read(path)
    except Exception:
        return _png_read(path)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="TBPX codec - Mode L (lossless RGB in standard images)")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_enc = sub.add_parser("encode", help="Encode binary to image")
    p_enc.add_argument("input", help="Input binary file")
    p_enc.add_argument("output", help="Output image file")
    p_enc.add_argument("--width", type=int, default=256, help="Image width (default 256)")
    p_enc.add_argument("--format", choices=["ppm", "png", "bmp", "tiff", "webp"], default="ppm", help="Output image format (default ppm)")
    p_enc.add_argument("--repeat-header", action="store_true", help="Write redundant header at image end")
    p_enc.add_argument("--sha256", action="store_true", help="Embed SHA-256 of payload in extension block")
    p_enc.add_argument("--sign", metavar="PRIVKEY", help="Sign payload hash with Ed25519 key (raw 32-byte secret)")

    p_dec = sub.add_parser("decode", help="Decode image to binary")
    p_dec.add_argument("input", help="Input image file")
    p_dec.add_argument("output", help="Output binary file")
    p_dec.add_argument("--verify", metavar="PUBKEY", help="Verify Ed25519 signature using public key (raw 32 bytes)")

    args = parser.parse_args(argv)

    try:
        if args.cmd == "encode":
            with open(args.input, "rb") as f:
                data = f.read()
            width, height, rgb = encode_to_ppm(
                data,
                width=args.width,
                repeat_header=args.repeat_header,
                want_sha256=args.sha256 or (args.sign is not None),
                sign_priv_path=args.sign,
            )
            # Choose output format
            if args.format == "ppm":
                _ppm_write(args.output, width, height, rgb)
            else:
                _pil_write_generic(args.output, width, height, rgb, args.format)
            print(f"Encoded {len(data)} bytes to {args.output} ({width}x{height}, format={args.format})")
            return 0
        elif args.cmd == "decode":
            w, h, rgb = _read_image_auto(args.input)
            payload = decode_from_ppm(rgb, verify_pub_path=args.verify)
            with open(args.output, "wb") as f:
                f.write(payload)
            print(f"Decoded {len(payload)} bytes to {args.output} from {args.input} ({w}x{h})")
            return 0
    except TBPXError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
