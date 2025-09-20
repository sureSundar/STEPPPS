# RISC-V 64-bit Architecture Guide

## Boot Process

### Standard Boot Flow
1. Hardware reset at reset vector (0x1000)
2. Boot ROM initializes minimal hardware
3. Loads OpenSBI (Supervisor Binary Interface)
4. OpenSBI initializes M-mode
5. Loads kernel image to 0x80000000
6. Jumps to kernel entry point in S-mode

### Privilege Levels
- **M-mode (Machine)**: Highest privilege, required
- **S-mode (Supervisor)**: OS kernel runs here
- **U-mode (User)**: User applications

## Memory Layout

### Physical Memory
```
0x00000000 - 0x7FFFFFFF: Memory Mapped I/O (2GB)
0x80000000 - 0x801FFFFF: Kernel (2MB)
0x80200000 - 0x8FFFFFFF: Kernel Heap (382MB)
0x90000000 - 0xFFFFFFFF: User Space (1.75GB)
```

### Virtual Memory
- SV39 (3-level paging, 39-bit virtual)
- 4KB page size (standard)
- 2MB and 1GB huge pages
- Kernel in upper half (0xFFFFFFC0_00000000)

## CPU Features

### Required Extensions
- I (Base Integer)
- M (Integer Multiplication/Division)
- A (Atomic Operations)
- F (Single-Precision Floating-Point)
- D (Double-Precision Floating-Point)
- C (Compressed Instructions)

### Optional Extensions
- B (Bit Manipulation)
- V (Vector Operations)
- H (Hypervisor)
- Zicsr (Control and Status Registers)

## Interrupts and Exceptions

### Interrupt Types
- **Timer Interrupt (MTI/STI)**: Machine/Supervisor timer
- **Software Interrupt (MSI/SSI)**: Inter-processor
- **External Interrupt (MEI/SEI)**: External devices

### PLIC (Platform-Level Interrupt Controller)
- Priority-based interrupt handling
- Context-based interrupt routing
- MSI support

## SBI (Supervisor Binary Interface)

### Core Functions
- Console I/O
- Timer management
- IPI (Inter-Processor Interrupts)
- Remote FENCE.I
- System reset

### Extensions
- Timer (TIME)
- IPI (IPI)
- RFENCE (Remote FENCE)
- Hart State Management (HSM)
- System Reset (SRST)

## Power Management

### WFI (Wait For Interrupt)
- Core idle state
- Implementation defined wake-up events
- May enter lower power states

### Clock Gating
- Peripherals can be individually gated
- Dynamic clock scaling support

## Debugging

### QEMU
```bash
qemu-system-riscv64 -machine virt -bios none \
  -kernel tbos.elf -nographic -smp 4 -m 2G
```

### GDB Commands
```
target remote localhost:1234
monitor system_reset
break *0x80000000
continue
```

## Common Issues
- Misaligned memory accesses
- Page table misconfiguration
- SBI call parameter passing
- Endianness confusion

## Performance Considerations
- Align critical code and data
- Use compressed instructions (C extension)
- Optimize for in-order execution
- Minimize pipeline stalls

## Testing

### QEMU
```bash
# Basic boot
qemu-system-riscv64 -machine virt -kernel tbos.elf -nographic

# With multiple harts
qemu-system-riscv64 -machine virt -smp 4 -kernel tbos.elf -nographic
```

### Hardware Testing
- Test on different implementations
- Validate interrupt latency
- Memory consistency checks
- Multi-core synchronization

## References
- RISC-V Instruction Set Manual (Volume I & II)
- RISC-V Privileged Architecture
- RISC-V SBI Specification
- SiFive FU540-C000 Manual
- QEMU RISC-V Documentation
