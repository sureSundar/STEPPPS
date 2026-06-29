# V4.0 Integration Tests

Integration tests for TernaryBit OS v4.0 Universal Boot.

## Test Files

| Test | Description | Requirements |
|------|-------------|--------------|
| `test_v4_boot_chain.sh` | Full boot chain (Stage1 -> Stage2 -> Kernel) | QEMU, built v4.0 image |
| `test_hosted_linux.sh` | Hosted Linux environment with BCB | Built hosted binary |
| `test_bcb_unit.sh` | BCB unit test wrapper | GCC |

## Running Tests

```bash
# Run all integration tests
./scripts/run-tests.sh --type integration

# Run individual tests
./tests/integration/test_v4_boot_chain.sh
./tests/integration/test_hosted_linux.sh
./tests/integration/test_bcb_unit.sh
```

## Prerequisites

1. Build v4.0 targets first:
   ```bash
   ./build_v4.sh
   ```

2. Install QEMU for boot chain tests:
   ```bash
   # macOS
   brew install qemu

   # Ubuntu/Debian
   sudo apt-get install qemu-system-x86
   ```

## Traceability

These tests implement V4-007 from the [V4.0 Roadmap](../../v4.0/ROADMAP.md).
