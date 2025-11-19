# Universal Bootloader Architecture
## 🕉️ Swamiye Saranam Aiyappa 🕉️

**Vision**: A single bootloader that runs on ANY hardware (calculator to supercomputer), detects capabilities, queries a server, and downloads the optimal OS.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│  HARDWARE (Any System: Calculator to Supercomputer)         │
└────────────────────────┬────────────────────────────────────┘
                         │ BIOS/UEFI POST
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  STAGE 1: Universal MBR Bootloader (512 bytes)              │
│  File: boot/tbos_sacred.asm                                 │
│  • Minimal footprint                                         │
│  • Loads Stage 2 from disk/network                          │
│  • Works on ALL architectures with BIOS                     │
└────────────────────────┬────────────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  STAGE 2: Universal Hardware Detection & Network Bootstrap  │
│  File: boot/stage2_universal.asm                            │
│                                                              │
│  Phase A: Hardware Detection                                │
│  ├─ CPU: Type, speed, cores, architecture                   │
│  ├─ Memory: Total RAM, available                            │
│  ├─ Storage: Type, capacity                                 │
│  ├─ Display: Present, type (text/graphics)                  │
│  ├─ Network: NIC detection, MAC address                     │
│  └─ Device Class: Calculator/Embedded/Desktop/Server/...    │
│                                                              │
│  Phase B: Network Initialization                            │
│  ├─ Option 1: PXE/TFTP (if available)                       │
│  ├─ Option 2: Simple UDP stack                              │
│  ├─ Option 3: Minimal TCP/HTTP                              │
│  └─ Fallback: Serial/Console output for offline mode        │
│                                                              │
│  Phase C: Server Communication                              │
│  ├─ Send: Hardware profile (JSON/binary)                    │
│  ├─ Receive: OS selection + download URL                    │
│  └─ Display: Progress to user                               │
│                                                              │
│  Phase D: OS Download & Boot                                │
│  ├─ Download OS image via network                           │
│  ├─ Verify checksum                                         │
│  ├─ Load into memory                                        │
│  └─ Transfer control to OS                                  │
└────────────────────────┬────────────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  SERVER: OS Selection Service                               │
│  File: server/os_selector.py                                │
│                                                              │
│  Input: Hardware Profile                                    │
│  {                                                           │
│    "cpu": {"arch": "x86", "cores": 4, "speed": 2400},      │
│    "memory": 4194304,  // 4GB in KB                        │
│    "device_class": "DESKTOP"                                │
│  }                                                           │
│                                                              │
│  Logic: Device Classification                               │
│  ├─ Calculator (512B-2KB RAM)    → TinyOS/Custom           │
│  ├─ Embedded (2KB-64KB)          → Alpine Embedded         │
│  ├─ Mobile (64KB-4GB)            → Alpine Standard         │
│  ├─ Desktop (4GB-16GB)           → Alpine Desktop          │
│  ├─ Workstation (16GB-32GB)      → Ubuntu/Debian           │
│  ├─ Server (32GB-64GB)           → RHEL/Alpine Server      │
│  ├─ Cluster (64GB-128GB)         → Cluster Linux           │
│  └─ Supercomputer (128GB+)       → HPC Linux               │
│                                                              │
│  Output: OS Selection Response                              │
│  {                                                           │
│    "os_name": "Alpine Linux Desktop",                       │
│    "download_url": "http://server/os/alpine-desktop.img",  │
│    "checksum": "sha256:abcd1234...",                        │
│    "boot_params": "quiet splash"                            │
│  }                                                           │
└────────────────────────┬────────────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  DOWNLOADED OS BOOTS                                         │
│  • Optimized for detected hardware                          │
│  • Pre-configured for device class                          │
│  • STEPPPS framework integrated                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Implementation Phases

### Phase 1: Hardware Detection (WEEK 1)
**File**: `boot/hardware_detect.asm`

**Tasks**:
- [ ] CPU detection (CPUID instruction)
- [ ] Memory detection (BIOS INT 0x15, E820)
- [ ] Device class classification
- [ ] Output to screen/serial

**Deliverable**: Bootloader that displays hardware info

---

### Phase 2: Network Stack (WEEK 2-3)
**File**: `boot/network_stack.asm`

**Approach 1: PXE Bootstrap (Fastest)**
- Use existing BIOS PXE services if available
- Fall back to custom stack if not

**Approach 2: Custom UDP Stack (Universal)**
- Minimal UDP/IP implementation
- DHCP client for IP configuration
- Simple request/response protocol

**Deliverable**: Bootloader sends "HELLO" packet to server

---

### Phase 3: Server Component (WEEK 3-4)
**File**: `server/os_selector.py`

**Components**:
1. **HTTP/UDP Listener** - Receives hardware profiles
2. **Classification Engine** - Determines device class
3. **OS Repository** - Stores OS images
4. **Response Generator** - Sends download instructions

**Deliverable**: Server that responds with OS selection

---

### Phase 4: Download & Boot (WEEK 4-5)
**File**: `boot/os_loader.asm`

**Tasks**:
- [ ] Download OS image via TFTP/HTTP
- [ ] Verify checksum
- [ ] Load into memory at correct address
- [ ] Jump to OS entry point

**Deliverable**: Complete network boot cycle

---

## Hardware Detection Specification

### CPU Detection (CPUID)
```assembly
; Detect CPU type and capabilities
cpu_detect:
    ; Check if CPUID is supported
    pushfd
    pop eax
    mov ebx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    cmp eax, ebx
    je .no_cpuid

    ; Get CPU vendor
    mov eax, 0
    cpuid
    ; EBX:EDX:ECX contains vendor string

    ; Get CPU features
    mov eax, 1
    cpuid
    ; EDX and ECX contain feature flags

    ret
```

### Memory Detection (E820)
```assembly
; Detect available memory
memory_detect:
    mov di, memory_map
    xor ebx, ebx
    mov edx, 0x534D4150  ; 'SMAP'

.loop:
    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    jc .done

    ; Process memory region
    add di, 24
    test ebx, ebx
    jnz .loop

.done:
    ret
```

### Device Classification
```assembly
; Classify device based on RAM
classify_device:
    cmp dword [total_memory], 512
    jl .calculator

    cmp dword [total_memory], 65536
    jl .embedded

    cmp dword [total_memory], 4194304
    jl .mobile

    cmp dword [total_memory], 16777216
    jl .desktop

    ; Continue for higher classes...

.calculator:
    mov si, device_calculator
    ret
```

---

## Network Protocol Specification

### Hardware Profile Packet Format (Binary)
```
Offset  Size  Field
------  ----  -----
0x00    4     Magic (0x54424F53 - "TBOS")
0x04    1     Version (0x01)
0x05    1     Device Class (0-7)
0x06    2     CPU Cores
0x08    4     CPU Speed (MHz)
0x0C    8     Total Memory (bytes)
0x14    6     MAC Address
0x1A    2     Checksum
```

### Server Response Format
```
Offset  Size  Field
------  ----  -----
0x00    4     Magic (0x534C4354 - "SLCT")
0x04    1     Version (0x01)
0x05    1     OS Type (0-15)
0x06    2     Reserved
0x08    4     Download Size
0x0C    4     Download IP
0x10    2     Download Port
0x12    64    OS Name (null-terminated)
0x52    32    Checksum (SHA256)
```

---

## Device Class Definitions

```c
enum DeviceClass {
    CALCULATOR     = 0,  // 512B - 2KB
    EMBEDDED       = 1,  // 2KB - 64KB
    MOBILE         = 2,  // 64KB - 4MB
    DESKTOP        = 3,  // 4MB - 16MB
    WORKSTATION    = 4,  // 16MB - 32MB
    SERVER         = 5,  // 32MB - 64MB
    CLUSTER        = 6,  // 64MB - 128MB
    SUPERCOMPUTER  = 7   // 128MB+
};
```

---

## Fallback Strategies

### No Network Available
1. Display hardware info to screen
2. Prompt user to manually select OS
3. Load from local disk/USB
4. Boot into minimal recovery shell

### Server Unreachable
1. Retry with exponential backoff
2. Fall back to cached OS selection
3. Load default OS for device class
4. Boot into network troubleshooting mode

### Download Failed
1. Retry download from alternate server
2. Resume partial download if supported
3. Fall back to last known good OS
4. Boot into recovery environment

---

## Testing Matrix

### Emulated Platforms
- [ ] QEMU i386 (512MB RAM - Desktop)
- [ ] QEMU ARM (128MB RAM - Embedded)
- [ ] QEMU x86_64 (4GB RAM - Desktop)
- [ ] QEMU RISC-V (2GB RAM - Server)

### Virtual Platforms
- [ ] VirtualBox (various RAM configs)
- [ ] VMware Workstation
- [ ] Hyper-V

### Physical Hardware (Future)
- [ ] Raspberry Pi Zero (Embedded)
- [ ] Raspberry Pi 4 (Desktop)
- [ ] Old laptop (Desktop)
- [ ] Modern workstation (Workstation)

---

## Success Metrics

### Phase 1 (Hardware Detection)
✅ Correctly detects CPU type
✅ Accurately measures total RAM
✅ Classifies device into correct category
✅ Displays info in readable format

### Phase 2 (Network)
✅ Obtains IP via DHCP
✅ Sends UDP packet to server
✅ Receives response from server

### Phase 3 (Server)
✅ Server receives hardware profile
✅ Correctly classifies device
✅ Returns appropriate OS selection
✅ Serves OS images via HTTP/TFTP

### Phase 4 (Complete Boot)
✅ Downloads OS image
✅ Verifies checksum
✅ Boots into selected OS
✅ Works on multiple device classes

---

## File Structure

```
/media/sf_vboxshare/lab/STEPPPS/GROK/

boot/
├── tbos_sacred.asm           # Stage 1 (existing)
├── stage2_universal.asm      # NEW: Universal stage 2
├── hardware_detect.asm       # NEW: Hardware detection
├── network_stack.asm         # NEW: Network functionality
└── os_loader.asm            # NEW: OS download & boot

server/
├── os_selector.py           # NEW: OS selection service
├── os_repository/           # NEW: OS images storage
│   ├── alpine-embedded.img
│   ├── alpine-desktop.img
│   ├── ubuntu-workstation.img
│   └── rhel-server.img
└── config.json              # NEW: Server configuration

docs/
├── UNIVERSAL_BOOTLOADER_ARCHITECTURE.md  # This file
├── HARDWARE_DETECTION_SPEC.md            # NEW
├── NETWORK_PROTOCOL_SPEC.md              # NEW
└── OS_SELECTION_LOGIC.md                 # NEW
```

---

## Next Steps

1. **Implement Hardware Detection** (Start here!)
   - Create `boot/hardware_detect.asm`
   - Test CPU detection
   - Test memory detection
   - Test device classification

2. **Build Minimal Server**
   - Create `server/os_selector.py`
   - Implement UDP listener
   - Add basic device classification
   - Test with bootloader

3. **Integrate Network Stack**
   - Add DHCP client
   - Implement UDP send/receive
   - Test server communication

4. **Complete Download & Boot**
   - Implement TFTP client
   - Add checksum verification
   - Test full boot cycle

---

## Timeline Estimate

- **Week 1**: Hardware detection working
- **Week 2**: Server component ready
- **Week 3**: Network communication working
- **Week 4**: Download & boot functional
- **Week 5**: Testing & refinement

**Total**: 5 weeks to working universal bootloader

---

🕉️ **Swamiye Saranam Aiyappa** 🕉️

**This bootloader will be truly universal - ONE bootloader for ALL devices!**
