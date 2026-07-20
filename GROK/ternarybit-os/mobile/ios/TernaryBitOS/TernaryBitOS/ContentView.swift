import SwiftUI

struct ContentView: View {
    @State private var message: String = "TernaryBit OS on iPhone 14 Pro Max"
    @State private var compressedHex: String = ""
    @State private var summary: String = ""
    @State private var roundTripResult: String = ""

    var body: some View {
        NavigationView {
            Form {
                Section("Real TBOS codec, not a mockup") {
                    Text("Calls the same pxfs_lossless_compress()/decompress() C functions the tier-5 shell's pxcompress/pxdecompress commands use (src/core/compression/pxfs_lossless.c). Not reimplemented in Swift.")
                        .font(.footnote)
                        .foregroundColor(.secondary)
                }

                Section("Input") {
                    TextField("Message to compress", text: $message)
                }

                Section {
                    Button("Compress") { compress() }
                    Button("Decompress & verify round trip") { decompressAndVerify() }
                        .disabled(compressedHex.isEmpty)
                }

                if !summary.isEmpty {
                    Section("Result") {
                        Text(summary)
                            .font(.system(.body, design: .monospaced))
                    }
                }

                if !roundTripResult.isEmpty {
                    Section("Verification") {
                        Text(roundTripResult)
                            .font(.system(.body, design: .monospaced))
                    }
                }
            }
            .navigationTitle("TernaryBit OS")
        }
    }

    private func compress() {
        let bytes = Array(message.utf8)
        guard !bytes.isEmpty, bytes.count <= Int(PXFS_LOSSLESS_MAX_INPUT) else {
            summary = "Input empty or exceeds \(PXFS_LOSSLESS_MAX_INPUT) bytes"
            compressedHex = ""
            return
        }
        var output = [UInt8](repeating: 0, count: Int(PXFS_LOSSLESS_MAX_ENCODED))
        let encodedLen = bytes.withUnsafeBufferPointer { inPtr -> Int in
            output.withUnsafeMutableBufferPointer { outPtr -> Int in
                Int(pxfs_lossless_compress(inPtr.baseAddress, bytes.count, outPtr.baseAddress, outPtr.count))
            }
        }
        guard encodedLen > 0 else {
            summary = "Compression failed"
            compressedHex = ""
            return
        }
        let encoded = Array(output.prefix(encodedLen))
        compressedHex = encoded.map { String(format: "%02x", $0) }.joined()
        let mode: String
        switch Int(encoded[0]) {
        case PXFS_LOSSLESS_PERIODIC: mode = "periodic"
        case PXFS_LOSSLESS_BLOCKS_V1: mode = "mixed runs + CRC32"
        default: mode = "literal"
        }
        let ratio = Double(bytes.count) / Double(encodedLen)
        summary = """
        Original:   \(bytes.count) bytes
        Compressed: \(encodedLen) bytes
        Ratio:      \(String(format: "%.1f", ratio)):1
        Mode:       \(mode)
        Hex:        \(compressedHex)
        """
        roundTripResult = ""
    }

    private func decompressAndVerify() {
        guard let encoded = hexToBytes(compressedHex) else {
            roundTripResult = "FAIL: could not parse hex"
            return
        }
        var output = [UInt8](repeating: 0, count: Int(PXFS_LOSSLESS_MAX_INPUT))
        let decodedLen = encoded.withUnsafeBufferPointer { inPtr -> Int in
            output.withUnsafeMutableBufferPointer { outPtr -> Int in
                Int(pxfs_lossless_decompress(inPtr.baseAddress, encoded.count, outPtr.baseAddress, outPtr.count))
            }
        }
        guard decodedLen > 0 else {
            roundTripResult = "FAIL: decompress rejected the encoded bytes"
            return
        }
        let decoded = Array(output.prefix(decodedLen))
        let original = Array(message.utf8)
        if decoded == original {
            roundTripResult = "PASS: \(decodedLen) bytes decoded, byte-for-byte identical to the original"
        } else {
            roundTripResult = "FAIL: decoded \(decodedLen) bytes did not match the \(original.count)-byte original"
        }
    }

    private func hexToBytes(_ hex: String) -> [UInt8]? {
        guard hex.count % 2 == 0 else { return nil }
        var bytes = [UInt8]()
        var index = hex.startIndex
        while index < hex.endIndex {
            let next = hex.index(index, offsetBy: 2)
            guard let byte = UInt8(hex[index..<next], radix: 16) else { return nil }
            bytes.append(byte)
            index = next
        }
        return bytes
    }
}

#Preview {
    ContentView()
}
