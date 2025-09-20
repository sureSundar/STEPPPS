#!/usr/bin/env python3
import os
import sys
import tempfile
import unittest

# Add tools path to import tbpx_codec
TOOLS_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "tools"))
sys.path.insert(0, TOOLS_DIR)

import tbpx_codec  # type: ignore


class TestTBPXCodec(unittest.TestCase):
    def roundtrip(self, payload: bytes, repeat_header: bool = False, width: int = 256):
        w, h, rgb = tbpx_codec.encode_to_ppm(payload, width=width, repeat_header=repeat_header)
        self.assertGreaterEqual(len(rgb), tbpx_codec.HEADER_SIZE)
        out = tbpx_codec.decode_from_ppm(rgb)
        self.assertEqual(out, payload)
        return w, h, rgb

    def test_empty_payload(self):
        self.roundtrip(b"")

    def test_single_byte(self):
        self.roundtrip(b"A")

    def test_three_bytes(self):
        self.roundtrip(b"XYZ")

    def test_1k_payload(self):
        self.roundtrip(os.urandom(1024))

    def test_repeat_header_recovers(self):
        # Encode with repeat header and corrupt the first header hash byte
        payload = os.urandom(128)
        w, h, rgb = tbpx_codec.encode_to_ppm(payload, repeat_header=True)
        # Corrupt first header
        rgb_mut = bytearray(rgb)
        rgb_mut[10] ^= 0xFF
        out = tbpx_codec.decode_from_ppm(bytes(rgb_mut))
        self.assertEqual(out, payload)

    def test_cli_encode_decode(self):
        payload = b"hello world\n" * 10
        with tempfile.TemporaryDirectory() as td:
            bin_in = os.path.join(td, "in.bin")
            ppm_path = os.path.join(td, "out.ppm")
            bin_out = os.path.join(td, "out.bin")
            with open(bin_in, "wb") as f:
                f.write(payload)
            # Run encode
            rc = tbpx_codec.main(["encode", bin_in, ppm_path, "--repeat-header"]) 
            self.assertEqual(rc, 0)
            # Run decode
            rc = tbpx_codec.main(["decode", ppm_path, bin_out])
            self.assertEqual(rc, 0)
            with open(bin_out, "rb") as f:
                data = f.read()
            self.assertEqual(data, payload)


if __name__ == "__main__":
    unittest.main()
