# ARM64 Architecture Guide

## Boot Process

### Standard Boot Flow
1. Boot ROM (EL3) initializes minimal hardware
2. Loads BL1 (First Stage Bootloader)
3. BL1 loads BL2 (Second Stage Bootloader)
4. BL2 loads ATF (ARM Trusted Firmware)
5. ATF initializes secure world (EL3)
6. Loads and verifies BL31 (EL3 runtime firmware)
7. BL31 loads BL32 (Optional Secure-EL1 Payload)
8. BL31 loads BL33 (Non-trusted firmware, e.g., U-Boot)
9. BL33 loads and executes the kernel

### Exception Levels
- **EL0**: User applications
- **EL1**: OS kernel
- **EL2**: Hypervisor
- **EL3**: Secure monitor

## Memory Layout

### Physical Memory
```
0x00000000 - 0x0000FFFF: Secure Monitor (64KB)
0x00010000 - 0x01FFFFFF: Kernel (32MB)
0x02000000 - 0x0FFFFFFF: Kernel Heap (224MB)
0x10000000 - 0xFFFFFFFF: User Space (3.75GB)
0x100000000+: High Memory (if supported)
```

### Virtual Memory
- 4KB, 16KB, or 64KB page sizes
- 4-level translation (48-bit VA)
- Kernel in higher half (0xFFFFFFC0_00000000)
- User space in lower half (0x00000000_00000000)

## CPU Features

### Required Features
- AArch64 execution state
- ARMv8.0 or later
- Generic Timers
- Generic Interrupt Controller (GIC)
- System register access

### Optional Features
- ARMv8.1 Atomic operations
- ARMv8.2 FP16 support
- ARMv8.4 Secure EL2
- SVE/SVE2 (Scalable Vector Extensions)

## Interrupts and Exceptions

### Exception Levels
- **Synchronous**: Instruction execution
- **IRQ**: Physical interrupt
- **FIQ**: Fast interrupt
- **SError**: System error

### GIC (Generic Interrupt Controller)
- Distributor interface
- CPU interface
- Virtualization extensions
- Message Signaled Interrupts (MSI)

## Power Management

### CPU Idle States
- WFI (Wait For Interrupt)
- CPU power down
- Cluster power down
- System power states

### CPUFreq
- Dynamic voltage and frequency scaling
- OPP (Operating Performance Points)
- EAS (Energy Aware Scheduling)

## Cache Coherency
- Cache maintenance operations
- Multi-cluster topologies
- Shareability domains
- Memory barriers

## Debugging

### QEMU
```bash
qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel tbos.elf
```

### GDB Commands
```
target remote localhost:1234
add-symbol-file tbos.elf
break kernel_main
continue
```

## Common Issues
- Misaligned memory accesses
- Incorrect cache maintenance
- TLB conflicts
- Endianness issues

## Performance Optimization
- Use proper memory barriers
- Align critical data structures
- Optimize for big.LITTLE
- Use appropriate cache policies

## Testing

### QEMU
```bash
# Basic boot test
qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel tbos.elf

# With GDB server
qemu-system-aarch64 -s -S -M virt -cpu cortex-a57 -nographic -kernel tbos.elf
```

### Hardware Testing
- Test on different SoCs
- Verify cache coherency
- Power management validation
- Interrupt latency measurement

## References
- ARM Architecture Reference Manual (ARMv8-A)
- ARM Cortex-A Series Programmer's Guide
- ARM Trusted Firmware Documentation
- SBSA (Server Base System Architecture) Specification
