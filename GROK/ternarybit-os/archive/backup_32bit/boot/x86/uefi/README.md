# TBOS x86 UEFI Loader (Prototype)

This directory houses the prototype UEFI application that participates in the
TBOS boot sequence on machines with UEFI firmware (including modern PCs and
Intel Macs). The loader is responsible for:

1. Broadcasting STEPPPS-aligned telemetry gathered from UEFI services.
2. Subscribing to policy/GenAI/neighbour responses (future work).
3. Presenting a human-confirmable boot plan.
4. Loading the TBOS kernel and passing a TBDS descriptor stream to it.

## Building

The code targets the `gnu-efi` toolchain. A minimal build command looks like:

```bash
# Example (linux host with gnu-efi and clang)
export TBOS_ROOT=$(pwd)/../..
clang -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall \
      -I$TBOS_ROOT/include -I/usr/include/efi -I/usr/include/efi/x86_64 \
      -DEFI_FUNCTION_WRAPPER \
      -c tbos_uefi_loader.c -o tbos_uefi_loader.o
ld -nostdlib -znocombreloc -T /usr/lib/gnu-efi/x86_64/elf_x86_64_efi.lds \
   -shared -Bsymbolic -L/usr/lib -L/usr/lib64 -lgnuefi -lefi \
   tbos_uefi_loader.o -o TBOS_EFI.efi
```

Adjust include/library paths for your distribution. On macOS, use the LLVM
cross-compilation packages or Docker.

## Running in QEMU (experimental)

```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=OVMF_CODE.fd,readonly=on \
  -drive if=pflash,format=raw,file=OVMF_VARS.fd \
  -drive file=fat:rw:./efi_build,format=raw
```

Place `TBOS_EFI.efi` under `efi_build/EFI/BOOT/BOOTX64.EFI`.

## Status

- Descriptor emission for ARCH_INFO and FIRMWARE_INFO implemented.
- Telemetry broadcast / subscription / human confirmation are placeholders.
- Kernel loading logic is stubbed (`TODO: load kernel`).

Future iterations will integrate REST/gRPC/MQTT clients, human approval UIs, and
TBOS kernel hand-off once the subscription workflow is ready.
