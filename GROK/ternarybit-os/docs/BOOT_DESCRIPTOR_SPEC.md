# TBOS Boot Descriptor Specification

**Version:** 0.1 (Draft)

TBOS relies on a modular, extensible descriptor stream passed from boot stages to the
kernel. The mechanism supports heterogeneous architectures (x86 BIOS, x86 UEFI,
ARM SPL, user-space guardians) without forcing a single monolithic structure.

## Guiding Principles

- **Modular:** Each loader emits only the descriptors it understands.
- **Extensible:** Unknown descriptor types are skipped; forward/backward
  compatibility is preserved through versioned types.
- **Verifiable:** Streams may be signed/hashed; descriptors can include
  provenance fields.
- **Explainable:** Descriptors map naturally to STEPPPS dimensions so higher-level
  components can reason about boot decisions.

## Terminology

- **Boot Descriptor Stream (BDS):** A linear byte buffer beginning with a common
  header followed by a sequence of TLV (Type-Length-Value) records.
- **Descriptor:** A TLV record whose type identifies the semantic payload, length
  gives its size, and value carries the payload.
- **Producer:** Firmware/loader component constructing descriptors.
- **Consumer:** TBOS kernel or later stages interpreting descriptors.

## Binary Layout

```
+----------------------+---------------------------+
| Field                | Description               |
+======================+===========================+
| Signature (4 bytes)  | ASCII "TBDS"              |
| Total Length (4)     | Size of entire stream     |
| Version (2)          | Major/minor (BDSv1 = 0x0100)
| Descriptor Count (2) | Number of TLVs following  |
| Reserved (8)         | Future use / alignment     |
+----------------------+---------------------------+
| TLV 0                | ...                       |
| TLV 1                | ...                       |
| ...                  | ...                       |
+----------------------+---------------------------+
```

Each TLV has the form:

```
struct tbds_tlv {
    uint16_t type;        // Descriptor ID
    uint16_t flags;       // Field attributes (compression, encoding, critical)
    uint32_t length;      // Payload length in bytes
    uint8_t  value[length];
};
```

*All multi-byte integers use little-endian encoding.* Alignment is naturally
handled by reading `length` bytes; padding may be inserted but must be accounted
for in `length`.

## Descriptor Types (Initial Set)

| Type ID | Name                  | Typical Producer       | Notes                              |
|---------|----------------------|-------------------------|-------------------------------------|
| 0x0001  | ARCH_INFO            | All loaders             | Architecture, word size, endianness |
| 0x0002  | FIRMWARE_INFO        | BIOS/UEFI/SPL           | Vendor, version, entry mode         |
| 0x0003  | MEMORY_MAP           | BIOS e820 / DT / UEFI   | One or more memory region records   |
| 0x0004  | BOOT_DEVICE          | BIOS, UEFI              | Media identifiers, partitions       |
| 0x0005  | VIDEO_INFO           | BIOS VBE / UEFI GOP     | Framebuffer base, resolution        |
| 0x0006  | CONSOLE_INFO         | All                      | Keyboard/UART availability          |
| 0x0007  | STEPPPS_TELEMETRY    | Boot guardian agents    | Serialized JSON snapshot            |
| 0x0008  | SECURITY_ATTESTATION| TPM / Secure Boot       | PCR values, signatures              |
| 0x7FFF  | VENDOR_CUSTOM        | Any                     | Free-form data, non-critical        |

Additional IDs may be registered in `docs/BOOT_DESCRIPTOR_REGISTRY.md`.

### Example: MEMORY_MAP Payload

```
struct tbds_memory_map_header {
    uint32_t entry_size;    // Size of one entry struct
    uint32_t entry_count;
};

struct tbds_memory_map_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;          // 1=RAM, 2=Reserved, etc.
    uint32_t attributes;    // Architecture-specific
};
```

### Example: STEPPPS_TELEMETRY Payload

UTF-8 JSON document describing the last telemetry broadcast, mirroring the
STEPPPS dimensions, signed separately if needed.

## Critical Descriptors

Flags bit `0x0001` marks a descriptor as *critical*. Consumers that do not
recognize a critical descriptor must abort boot and surface the reason. This is
useful for mandatory security policy descriptors.

## Transport & Storage

- BDS may reside in low physical memory (for BIOS), in a reserved UEFI memory
  pool, in a shared buffer provided via register, or handed to the kernel by
  pointer in a register.
- A 64-bit pointer to the BDS plus the stream length should be supplied in a
  well-defined register or through an architecture-specific hand-off block.

### Suggested Hand-off

| Architecture | Register / Mechanism                                  |
|--------------|--------------------------------------------------------|
| x86 BIOS     | `EAX = &tbds_header`, `EBX = total_length`             |
| x86 UEFI     | `RDI = &tbds_header`, `RSI = total_length`             |
| ARM64        | `x0 = &tbds_header`, `x1 = total_length`               |

## Integrity & Signatures

For environments requiring tamper evidence, append a descriptor of type
`SECURITY_ATTESTATION` containing signature algorithm IDs, hashes, and signer
metadata. Loaders may also store the entire stream in write-protected memory and
export references.

## Next Steps

- Implement helper routines in `include/tbos_boot_descriptor.h` for constructing
  and parsing descriptor streams.
- Update BIOS stage 2 loader to emit ARCH_INFO + MEMORY_MAP + BOOT_DEVICE.
- Implement UEFI loader to emit ARCH_INFO + FIRMWARE_INFO + VIDEO_INFO + optional
  STEPPPS telemetry blob.
- Add kernel-side parser that funnels descriptor data into STEPPPS dimensions.

