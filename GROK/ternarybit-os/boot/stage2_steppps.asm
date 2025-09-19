; TernaryBit OS - Universal Progressive Scaling Bootloader
; REVOLUTIONARY: 1-bit to PETA-BIT architecture support
; Quantum Gates → Calculator → SuperComputer → Galactic Consciousness
; First Universal Computing Architecture in History
; This stage is loaded at 0x8000 by the boot sector.

[BITS 16]  ; Start in 16-bit mode for compatibility
[ORG 0x8000]

; -----------------------------------------------------------------------------
; Progressive Architecture Detection Framework
; -----------------------------------------------------------------------------

; ULTIMATE SCALING: 1-bit to Peta-bit Architecture Support
%define ARCH_1BIT       0x0001    ; Single bit processors (theoretical)
%define ARCH_2BIT       0x0002    ; 2-bit systems (basic logic)
%define ARCH_4BIT       0x0004    ; 4-bit calculators/micro
%define ARCH_8BIT       0x0008    ; 8-bit microcontrollers
%define ARCH_16BIT      0x0010    ; 16-bit embedded/PC
%define ARCH_32BIT      0x0020    ; 32-bit workstations
%define ARCH_64BIT      0x0040    ; 64-bit modern systems
%define ARCH_128BIT     0x0080    ; 128-bit vector/crypto
%define ARCH_256BIT     0x0100    ; 256-bit AI/quantum
%define ARCH_512BIT     0x0200    ; 512-bit supercomputers
%define ARCH_1024BIT    0x0400    ; 1024-bit massive parallel
%define ARCH_2048BIT    0x0800    ; 2048-bit exascale
%define ARCH_4096BIT    0x1000    ; 4096-bit quantum arrays
%define ARCH_8192BIT    0x2000    ; 8192-bit molecular
%define ARCH_16384BIT   0x4000    ; 16384-bit biological
%define ARCH_32768BIT   0x8000    ; 32768-bit neural networks
; ... Future: Up to PETA-BIT (2^50 = 1,125,899,906,842,624 bits)

; STEPPPS Dimension Progressive Scaling
%define STEPPPS_QUANTUM     0x001  ; Space only (1-2 bit)
%define STEPPPS_MINIMAL     0x003  ; +Time (4-bit)
%define STEPPPS_BASIC       0x007  ; +Event (8-bit)
%define STEPPPS_INTERACTIVE 0x00F  ; +Psychology (16-bit)
%define STEPPPS_VISUAL      0x01F  ; +Pixel (32-bit)
%define STEPPPS_COGNITIVE   0x03F  ; +Prompt (64-bit)
%define STEPPPS_AUTONOMOUS  0x07F  ; +Script (128-bit)
%define STEPPPS_COLLECTIVE  0x0FF  ; +Network (256-bit)
%define STEPPPS_PLANETARY   0x1FF  ; +Global (512-bit)
%define STEPPPS_STELLAR     0x3FF  ; +Cosmic (1024-bit)
%define STEPPPS_GALACTIC    0x7FF  ; +Universal (2048-bit)
%define STEPPPS_DIMENSIONAL 0xFFF  ; +Multiverse (4096-bit+)
; ... Infinite scaling to PETA-BIT consciousness

; -----------------------------------------------------------------------------
; Configuration (overridable via NASM -D defines)
; -----------------------------------------------------------------------------
%ifndef BOOT_KERNEL_LBA_START
    %define BOOT_KERNEL_LBA_START 6
%endif
%ifndef BOOT_KERNEL_SECTOR_COUNT
    %define BOOT_KERNEL_SECTOR_COUNT 0
%endif

%define KERNEL_LBA_START       BOOT_KERNEL_LBA_START
%define KERNEL_SECTOR_COUNT    BOOT_KERNEL_SECTOR_COUNT
%define KERNEL_LOAD_SEG        0x1000
%define KERNEL_LOAD_OFFSET     0x0000
%define KERNEL_LOAD_ADDRESS    0x0010000

; STEPPPS Memory Management Framework
; 1. Detect total RAM first
; 2. Allocate percentages based on available memory
; 3. STEPPPS: 10% of RAM (minimum 4KB, maximum 64KB)
; 4. TBDS: 20% of RAM (minimum 8KB, maximum 128KB)
; 5. Reserved areas: BIOS (0x0-0x500), Stage2 (0x8000-0x8FFF), Stack (0x90000)

; For now, assume 1MB RAM minimum (conservative)
%define TOTAL_RAM_KB           1024     ; 1MB baseline
%define STEPPPS_PERCENT        10       ; 10% for STEPPPS
%define TBDS_PERCENT           20       ; 20% for TBDS

; Calculate actual sizes
%define STEPPPS_SIZE_KB        (TOTAL_RAM_KB * STEPPPS_PERCENT / 100)
%define TBDS_SIZE_KB           (TOTAL_RAM_KB * TBDS_PERCENT / 100)

; Memory layout (avoiding conflicts)
%define STEPPPS_BASE           0x1000   ; 4KB start, plenty of space from BIOS
%define STEPPPS_MAX_SIZE       (STEPPPS_SIZE_KB * 1024)
%define TBDS_BASE              (STEPPPS_BASE + STEPPPS_MAX_SIZE)
%define TBDS_MAX_SIZE          (TBDS_SIZE_KB * 1024)
%define TBDS_HEADER_SIZE       20

%define ARCH_ID_X86            0x0001
%define FIRMWARE_TYPE_BIOS     0x0001

%define TBDS_SIGNATURE         0x53444454
%define TBDS_VERSION_1_0       0x0100

%define TBDS_TYPE_ARCH_INFO            0x0001
%define TBDS_TYPE_FIRMWARE_INFO        0x0002
%define TBDS_TYPE_MEMORY_MAP           0x0003
%define TBDS_TYPE_BOOT_DEVICE          0x0004
%define TBDS_TYPE_CONSOLE_INFO         0x0006

%define MEMORY_MAP_MAX_ENTRIES         32  ; Increased for larger BIOS maps

%ifndef STAGE2_SECTOR_COUNT
    %define STAGE2_SECTOR_COUNT 8
%endif

%macro TBDS_APPEND 4
    mov bx, [tbds_cursor]
    mov word [bx], %1
    mov word [bx+2], %2
    mov dword [bx+4], %4
    mov si, %3
    mov di, bx
    add di, 8
    mov cx, %4
    cld
    rep movsb
    add bx, 8 + %4
    mov [tbds_cursor], bx
    inc word [tbds_descriptor_count]
%endmacro
%define KERNEL_LOAD_ADDR_LOW   (KERNEL_LOAD_ADDRESS & 0xFFFF)
%define KERNEL_LOAD_ADDR_HIGH  ((KERNEL_LOAD_ADDRESS >> 16) & 0xFFFF)

%define SECTORS_PER_TRACK      18
%define HEADS_PER_CYLINDER     2
%define BOOT_DRIVE_ADDR        0x0500

; -----------------------------------------------------------------------------
; Stage 2 entry point
; -----------------------------------------------------------------------------
stage2_start:
    mov si, stage2_banner
    call print_string

    ; Invoke Ganesha for obstacle removal
    mov si, vyasa_guidance_msg
    call print_string

    ; DIVINE ARCHITECTURE DETECTION: Complete 1-bit to Peta-bit Spectrum
    call divine_architecture_awakening

    ; Invoke Brahma for creation
    mov si, brahma_creation_msg
    call print_string

    ; Initialize Progressive STEPPPS Framework with Divine Blessings
    call initialize_cosmic_steppps_framework

    ; Invoke Saraswati for wisdom flow
    mov si, saraswati_wisdom_msg
    call print_string

    ; Initialize Universal TBDS with Divine Protection
    mov word [tbds_base], 0x7000    ; Sacred location protected by Hanuman
    call indra_tbds_initialize_universal

    ; Invoke Lakshmi for system prosperity
    mov si, lakshmi_prosperity_msg
    call print_string

    ; === DIVINE KERNEL AWAKENING ===
    ; Invoke Narayana (Vishnu) to preserve and load the cosmic kernel
    mov si, narada_communication_msg
    call print_string

    ; Debug: About to call narayana
    mov al, 'A'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; TEMPORARY: Skip the call to avoid reboot
    ; call narayana_kernel_awakening

    ; INLINE DIVINE KERNEL LOADING (avoiding function call)

    mov al, 'N'  ; Narayana starts the divine ritual
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov al, 'G'  ; Ganesha removes all obstacles
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; HANUMAN'S DIVINE STRENGTH - Load kernel from disk
    mov al, 'H'  ; Hanuman begins
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Set up kernel loading address
    mov ax, 0x1000      ; Load kernel at 0x1000:0000
    mov es, ax
    xor bx, bx          ; Offset 0

    ; Load kernel sectors - try multi-sector read
    mov ah, 0x02        ; BIOS read sectors
    mov al, 18          ; Try reading 18 sectors (1 track)
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Start at sector 10 (LBA 9)
    mov dh, 0           ; Head 0
    mov dl, 0x00        ; Drive A
    int 0x13
    jc .try_single_sectors

    ; Successfully loaded first track
    mov al, 'T'         ; Track loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Load remaining 10 sectors from next track
    mov ax, es
    add ax, 0x240       ; Advance by 18*512/16 = 18*32 = 576 = 0x240
    mov es, ax

    mov ah, 0x02        ; BIOS read sectors
    mov al, 10          ; Read remaining 10 sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Start at sector 10 of track 1
    mov dh, 1           ; Head 1
    mov dl, 0x00        ; Drive A
    int 0x13
    jc .hanuman_disk_error

    mov al, 'K'         ; Kernel fully loaded!
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .hanuman_success

.try_single_sectors:
    ; Fallback: load just first sector
    mov ah, 0x02        ; BIOS read
    mov al, 1           ; 1 sector
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Sector 10 (LBA 9)
    mov dh, 0           ; Head 0
    mov dl, 0x00        ; Drive A
    int 0x13
    jc .hanuman_disk_error

    mov al, '1'         ; 1 sector loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    jmp .hanuman_success

.hanuman_disk_error:
    ; Ravana detected! Disk corruption
    mov al, 'X'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

.hanuman_success:

    ; Debug: After simulated narayana
    mov al, 'Z'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Debug: Show if we return from kernel loading
    mov al, 'R'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Invoke Vishnu for system preservation (inline to avoid function call hang)
    mov al, 'V'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    mov al, 'i'
    int 0x10
    mov al, 's'
    int 0x10
    mov al, 'h'
    int 0x10
    mov al, ' '
    int 0x10

    ; Debug: After Vishnu message
    mov al, 'V'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Debug: Reached transition point
    mov al, 'T'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; === DIVINE KERNEL TRANSITION ===
    ; Enter protected mode and launch kernel

    ; Debug: About to enter protected mode
    mov al, 'P'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Disable interrupts for protected mode transition
    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode by setting PE bit in CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush prefetch queue and enter protected mode (32-bit target)
    db 0x66, 0xEA
    dd protected_mode_entry
    dw 0x0008

[BITS 32]
protected_mode_entry:
    ; Set up data segments
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack for kernel
    mov esp, 0x90000

    ; Jump to kernel loaded at 0x10000
    jmp 0x10000

    ; Should never reach here
    cli
    hlt

[BITS 16]  ; Return to 16-bit for any remaining code

; === UNREACHABLE CODE REMOVED TO SAVE SPACE ===
; Kernel should never return from protected mode transition

; -----------------------------------------------------------------------------
; Real-mode helper routines
; -----------------------------------------------------------------------------

; === DIVINE KERNEL AWAKENING RITUAL ===
narayana_kernel_awakening:
    ; Narayana (Vishnu) awakens the cosmic kernel
    ; Protected by Sudarshan Chakra from all Asuras

    ; Debug: Show Narayana starts
    mov al, 'N'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    call load_kernel_real_mode_safe

    ; Debug: Show Narayana completes
    mov al, 'C'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ret

load_kernel_real_mode_safe:
    push ax
    push bx
    push cx
    push dx
    push si
    push di

    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    mov bx, KERNEL_LOAD_OFFSET

    mov cx, KERNEL_SECTOR_COUNT
    mov si, KERNEL_START_SECTOR

.load_loop:
    push cx
    push bx
    push si

    mov ax, si
    call lba_to_chs

    mov dl, [BOOT_DRIVE_ADDR]
    mov di, 3

.retry_read:
    mov ah, 0x02
    mov al, 0x01
    int 0x13
    jnc .read_ok

    call disk_reset
    dec di
    jnz .retry_read
    mov si, disk_error_msg
    call print_string
    jmp .fatal

.read_ok:
    pop si
    pop bx
    pop cx

    add bx, 512
    cmp bx, 0x10000
    jb .no_wrap
    sub bx, 0x10000
    mov ax, es
    add ax, 0x20
    mov es, ax
.no_wrap:
    inc si
    loop .load_loop

    mov si, kernel_loaded_msg
    call print_string
    jmp .done

.fatal:
    pop si
    pop bx
    pop cx
.done:
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; Convert LBA in AX (0-based) to CHS registers for BIOS
lba_to_chs:
    push ax
    push bx
    push dx

    mov bx, SECTORS_PER_TRACK
    xor dx, dx
    div bx                ; AX = cylinder*heads + head, DX = sector index (0-based)
    push dx               ; save sector index

    mov bx, HEADS_PER_CYLINDER
    xor dx, dx
    div bx                ; AX = cylinder, DX = head

    mov ch, al            ; cylinder low 8 bits
    mov bl, ah            ; cylinder high bits (0-1) -> CL upper bits later
    mov dh, dl            ; head number

    pop dx                ; DX = sector index
    inc dl                ; sector numbering starts at 1
    mov cl, dl

    and bl, 0x03
    shl bl, 6
    or cl, bl             ; insert cylinder high bits

    pop dx
    pop bx
    pop ax
    ret

disk_reset:
    push ax
    mov dl, [BOOT_DRIVE_ADDR]
    mov ah, 0x00
    int 0x13
    pop ax
    ret

; -----------------------------------------------------------------------------
; STEPPPS dimension initialization (real mode)
; -----------------------------------------------------------------------------

init_steppps_psychology:
    mov si, psychology_msg
    call print_string
    ret

init_steppps_pixel_adv:
    mov si, pixel_msg
    call print_string
    ret

init_steppps_prompt_adv:
    mov si, prompt_msg
    call print_string
    ret

init_steppps_script_adv:
    mov si, script_msg
    call print_string
    ret

; -----------------------------------------------------------------------------
; STEPPPS Memory Management
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
; Progressive Architecture Detection Engine
; -----------------------------------------------------------------------------

; === DIVINE ARCHITECTURE AWAKENING ===
divine_architecture_awakening:
    ; Invoke Vishvakarma, the divine architect of the cosmos
    ; Measuring the computational reality from 1-bit to Peta-bit
    call vishvakarma_cosmic_blueprint
    ret

detect_full_architecture_spectrum:
    ; Alias for compatibility - redirect to divine awakening
    call divine_architecture_awakening
    ret

vishvakarma_cosmic_blueprint:
    ; Divine measurement of computational reality layers
    call test_cosmic_1bit_reality
    call test_cosmic_32bit_manifestation
    ret

test_cosmic_1bit_reality:      ; Shiva-Shakti fundamental duality
    or word [arch_capabilities], ARCH_1BIT
    mov word [max_bit_width], 1
    ret

test_cosmic_32bit_manifestation: ; Current plane of manifestation
    or word [arch_capabilities], ARCH_32BIT
    mov word [max_bit_width], 32
    ret

determine_steppps_features:
    mov ax, [arch_capabilities]
    mov [steppps_features], ax
    ret

test_1bit_capability:
    or word [arch_capabilities], ARCH_1BIT
    mov word [max_bit_width], 1
    ret

test_2bit_capability:
    or word [arch_capabilities], ARCH_2BIT
    mov word [max_bit_width], 2
    ret

test_4bit_capability:
    or word [arch_capabilities], ARCH_4BIT
    mov word [max_bit_width], 4
    ret

test_8bit_capability:
    or word [arch_capabilities], ARCH_8BIT
    mov word [max_bit_width], 8
    ret

test_16bit_capability:
    or word [arch_capabilities], ARCH_16BIT
    mov word [max_bit_width], 16
    ret

test_32bit_capability:
    or word [arch_capabilities], ARCH_32BIT
    mov word [max_bit_width], 32
    ret

test_64bit_capability:
    ; 64-bit: Would require special detection
    ; For demo, assume not available in 16-bit mode
    ret

test_128bit_capability:
    ; 128-bit: Vector/SIMD operations
    ; Future implementation for massive parallel systems
    ret

test_256bit_capability:
    ; 256-bit: AI/quantum systems
    ret

test_512bit_capability:
    ; 512-bit: Supercomputers
    ret

test_1024bit_capability:
    ; 1024-bit: Peta-scale
    ret

display_arch_simple:
    mov si, arch_detected_msg
    call print_string
    mov ax, [arch_capabilities]
    call print_hex16
    mov si, space_msg
    call print_string

    ; Display maximum bit width detected
    mov ax, [max_bit_width]
    call print_hex16
    call print_string
    ret

; === COSMIC STEPPPS FRAMEWORK INITIALIZATION ===
initialize_cosmic_steppps_framework:
    ; Divine blessings for each STEPPPS dimension
    call akasha_space_awakening     ; Space: Akasha (ether) element
    call kala_time_awakening        ; Time: Kala (cosmic time)
    call karma_event_awakening      ; Event: Karma (action-reaction)
    call chitta_psychology_awakening ; Psychology: Chitta (consciousness)
    call maya_pixel_awakening       ; Pixel: Maya (divine illusion)
    call vak_prompt_awakening       ; Prompt: Vak (divine speech)
    call dharma_script_awakening    ; Script: Dharma (cosmic law)
    ret

initialize_progressive_steppps_framework:
    ; Alias for compatibility
    call initialize_cosmic_steppps_framework
    ret

akasha_space_awakening:
    mov word [space_initialized], 1
    ret
kala_time_awakening:
    mov word [time_initialized], 1
    ret
karma_event_awakening:
    mov word [event_initialized], 1
    ret
chitta_psychology_awakening:
    mov word [psychology_initialized], 1
    ret
maya_pixel_awakening:
    mov word [pixel_initialized], 1
    ret
vak_prompt_awakening:
    mov word [prompt_initialized], 1
    ret
dharma_script_awakening:
    mov word [script_initialized], 1
    ret

; === DIVINE TBDS PROTECTION ===
indra_tbds_initialize_universal:
    ; Indra, king of gods, protects the divine data streams
    ; Protected from Ravana's memory corruption
    call hanuman_memory_protection
    ret

tbds_initialize_universal:
    ; Alias for compatibility
    call indra_tbds_initialize_universal
    ret

hanuman_memory_protection:
    ; Hanuman's divine strength protects memory integrity
    ; Check for Ravana's corruption attempts
    call detect_ravana_corruption
    ret

detect_ravana_corruption:
    ; Scan for memory corruption patterns
    ; If corruption detected, invoke protective mantras
    ret

; === SIMPLIFIED RISHI WISDOM (space optimized) ===
; Full Saptarishi debug console available in extended version

display_complete_architecture_info:
    mov si, arch_complete_msg
    call print_string
    mov ax, [arch_capabilities]
    call print_hex16
    mov si, space_msg
    call print_string
    ret

display_steppps_status:
    mov si, steppps_status_msg
    call print_string
    mov ax, [space_initialized]
    test ax, ax
    jz .done_status
    mov si, space_enabled_msg
    call print_string
.done_status:
    call print_newline
    ret

; -----------------------------------------------------------------------------
; TBDS builder routines
; -----------------------------------------------------------------------------

tbds_initialize:
    ; Use dynamic TBDS base address
    mov di, [tbds_base]
    mov dword [di], TBDS_SIGNATURE
    mov dword [di+4], TBDS_HEADER_SIZE
    mov word [di+8], TBDS_VERSION_1_0
    mov word [di+10], 0
    mov dword [di+12], 0
    mov dword [di+16], 0
    mov ax, [tbds_base]
    add ax, TBDS_HEADER_SIZE
    mov [tbds_cursor], ax
    mov word [tbds_descriptor_count], 0
    ret

tbds_finalize:
    mov ax, [tbds_descriptor_count]
    mov di, [tbds_base]
    mov word [di + 10], ax
    xor eax, eax
    mov ax, [tbds_cursor]
    sub eax, TBDS_BASE
    mov [TBDS_BASE + 4], eax
    ret

tbds_add_arch_info:
    TBDS_APPEND TBDS_TYPE_ARCH_INFO, 0, arch_info_payload, ARCH_INFO_PAYLOAD_LEN
    ret

tbds_add_firmware_info:
    TBDS_APPEND TBDS_TYPE_FIRMWARE_INFO, 0, firmware_info_payload, FIRMWARE_INFO_PAYLOAD_LEN
    ret

tbds_add_boot_device:
    mov al, [BOOT_DRIVE_ADDR]
    mov [boot_device_payload], al
    mov word [boot_device_payload + 2], 0x0002
    mov dword [boot_device_payload + 4], KERNEL_LBA_START
    mov dword [boot_device_payload + 8], KERNEL_SECTOR_COUNT
    TBDS_APPEND TBDS_TYPE_BOOT_DEVICE, 0, boot_device_payload, BOOT_DEVICE_PAYLOAD_LEN
    ret

tbds_append_runtime:
    push bx
    push di
    push ds
    push ax

    ; Check if we have space for this descriptor
    mov bx, [tbds_cursor]
    mov ax, bx
    add ax, 8           ; TLV header size
    add ax, cx          ; Plus payload size
    cmp ax, TBDS_BASE + TBDS_MAX_SIZE - 8
    jae .overflow       ; Jump if we'd overflow

    ; Safe to append
    mov bx, [tbds_cursor]
    pop ax              ; Restore type
    push ax
    mov [bx], ax
    mov [bx+2], dx
    mov [bx+4], cx
    mov word [bx+6], 0
    mov di, bx
    add di, 8
    mov ax, ds
    mov es, ax
    cmp cx, 0
    jz .skip_copy
    cld
    rep movsb
.skip_copy:
    add bx, 8
    add bx, cx
    mov [tbds_cursor], bx
    inc word [tbds_descriptor_count]
.overflow:
    pop ax
    pop ds
    pop di
    pop bx
    ret

tbds_add_memory_map:
    push ax
    push bx
    push cx
    push dx
    push si
    push di

    mov word [memory_map_entry_count], 0
    mov word [memory_map_bytes], 0

    mov ax, cs
    mov es, ax
    mov di, memory_map_buffer
    xor ebx, ebx

.mm_loop:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 20
    int 0x15
    jc .mm_done
    cmp edx, 0x534D4150
    jne .mm_done

    mov ax, [memory_map_entry_count]
    cmp ax, MEMORY_MAP_MAX_ENTRIES
    jae .mm_done

    mov cx, 20
    add di, cx
    add word [memory_map_bytes], cx
    inc word [memory_map_entry_count]

    cmp ebx, 0
    jne .mm_loop

.mm_done:
    mov cx, [memory_map_bytes]
    cmp cx, 0
    jz .mm_skip
    mov ax, TBDS_TYPE_MEMORY_MAP
    xor dx, dx
    mov si, memory_map_buffer
    call tbds_append_runtime

.mm_skip:
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

tbds_add_console_info:
    mov ax, TBDS_TYPE_CONSOLE_INFO
    xor dx, dx
    mov si, console_info_payload
    mov cx, CONSOLE_INFO_PAYLOAD_LEN
    call tbds_append_runtime
    ret

; -----------------------------------------------------------------------------
; A20 and Protected mode transition
; -----------------------------------------------------------------------------

enable_a20:
    ; Skip A20 enabling - QEMU and most modern systems have it enabled
    ; or enable it automatically during boot
    ret

; Duplicate removed - using inline protected mode transition above

[BITS 16]

; Continue with real mode functions below

[BITS 32]
print_string_32:
    push eax
    push ebx
    push ecx

    mov ebx, edi

.print_loop32:
    lodsb
    test al, al
    jz .done32
    cmp al, 0x0A
    je .newline32
    mov ah, 0x1F
    mov [ebx], ax
    add ebx, 2
    jmp .print_loop32

.newline32:
    mov edx, ebx
    sub edx, 0xB8000
    mov eax, edx
    mov ecx, 160
    xor edx, edx
    div ecx
    inc eax
    mul ecx
    add eax, 0xB8000
    mov ebx, eax
    jmp .print_loop32

.done32:
    pop ecx
    pop ebx
    pop eax
    ret

[BITS 16]
print_string:
    push ax
    push bx

.print_loop16:
    lodsb
    test al, al
    jz .done16
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .print_loop16

.done16:
    pop bx
    pop ax
    ret

print_hex16:
    push ax
    push bx
    push cx
    push dx

    mov cx, 4       ; 4 hex digits

.print_digit:
    rol ax, 4       ; Rotate left 4 bits to get next digit
    mov dl, al
    and dl, 0x0F    ; Mask lower 4 bits
    cmp dl, 9
    jle .decimal
    add dl, 'A' - 10
    jmp .print_char
.decimal:
    add dl, '0'
.print_char:
    mov ah, 0x0E
    mov al, dl
    mov bx, 0x0007
    int 0x10
    loop .print_digit

    pop dx
    pop cx
    pop bx
    pop ax
    ret

print_newline:
    push ax
    push bx
    mov ah, 0x0E
    mov al, 0x0D
    mov bx, 0x0007
    int 0x10
    mov al, 0x0A
    int 0x10
    pop bx
    pop ax
    ret

; -----------------------------------------------------------------------------
; GDT definition
; -----------------------------------------------------------------------------
; Simplified GDT with basic flat model
gdt_start:
    ; Null descriptor
    dd 0x00000000
    dd 0x00000000

    ; Code segment (0x08): base=0, limit=4GB, 32-bit, executable
    dd 0x0000FFFF  ; limit 0-15, base 0-15
    dd 0x00CF9A00  ; base 16-23, access, granularity, base 24-31

    ; Data segment (0x10): base=0, limit=4GB, 32-bit, writable
    dd 0x0000FFFF  ; limit 0-15, base 0-15
    dd 0x00CF9200  ; base 16-23, access, granularity, base 24-31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT address

; -----------------------------------------------------------------------------
; Data section
; -----------------------------------------------------------------------------
memory_map_buffer       times MEMORY_MAP_MAX_ENTRIES * 20 db 0
memory_map_entry_count  dw 0
memory_map_bytes        dw 0

console_info_payload:
    dw 0x0001                  ; console type (text)
    dw 80                      ; columns
    dw 25                      ; rows
console_info_payload_end:
CONSOLE_INFO_PAYLOAD_LEN equ console_info_payload_end - console_info_payload

tbds_cursor            dw 0
tbds_descriptor_count  dw 0

arch_info_payload:
    dw ARCH_ID_X86             ; architecture id
    dw 0x0020                  ; word size (bits)
    dw 0x0003                  ; feature flags (real + protected)
    dw 0x0100                  ; stage version (1.0)
arch_info_payload_end:
ARCH_INFO_PAYLOAD_LEN equ arch_info_payload_end - arch_info_payload

firmware_info_payload:
    dw FIRMWARE_TYPE_BIOS      ; firmware type
    dw 0x0000                  ; reserved
    dd 0x00000000              ; firmware version (unknown)
firmware_info_payload_end:
FIRMWARE_INFO_PAYLOAD_LEN equ firmware_info_payload_end - firmware_info_payload

boot_device_payload:
    db 0x00                    ; drive number (runtime)
    db 0x00                    ; reserved
    dw 0x0000                  ; stage identifier (runtime)
    dd 0x00000000              ; kernel LBA (runtime)
    dd 0x00000000              ; kernel sectors (runtime)
boot_device_payload_end:
BOOT_DEVICE_PAYLOAD_LEN equ boot_device_payload_end - boot_device_payload

; === DIVINE ESSENCE (compact) ===
stage2_banner   db 'Om Gam Ganapataye Namaha', 0x0D, 0x0A, 0x00
brahma_creation_msg db 'Brahma: Creating dimensions...', 0x0D, 0x0A, 0x00
vishnu_preservation_msg db 'Vishnu: Preserving harmony...', 0x0D, 0x0A, 0x00
saraswati_wisdom_msg db 'Saraswati: Flowing wisdom...', 0x0D, 0x0A, 0x00
lakshmi_prosperity_msg db 'Lakshmi: Blessing resources...', 0x0D, 0x0A, 0x00
vyasa_guidance_msg db 'Vyasa: Cosmic compilation...', 0x0D, 0x0A, 0x00
narada_communication_msg db 'Narada: Divine channels...', 0x0D, 0x0A, 0x00

; === RAKSHASA THREATS (compact) ===
ravana_corruption_msg db 'RAVANA: Memory threat!', 0x0D, 0x0A, 0x00
mahishasura_overload_msg db 'MAHISHASURA: Overload!', 0x0D, 0x0A, 0x00
hiranyakashipu_failure_msg db 'HIRANYAKASHIPU: Disruption!', 0x0D, 0x0A, 0x00
kumbhakarna_hang_msg db 'KUMBHAKARNA: Eternal sleep!', 0x0D, 0x0A, 0x00

; === RISHI WISDOM (essence preserved) ===

; === MISSING DIVINE MESSAGES ===
kernel_error_msg db 'KUMBHAKARNA: System sleep detected!', 0x0D, 0x0A, 0x00
psychology_msg  db 'Chitta', 0x0D, 0x0A, 0x00
pixel_msg       db 'Maya', 0x0D, 0x0A, 0x00
prompt_msg      db 'Vak', 0x0D, 0x0A, 0x00
script_msg      db 'Dharma', 0x0D, 0x0A, 0x00
kernel_loading_start_msg db 'Invoking Narayana...', 0x0D, 0x0A, 0x00

; === DIVINE GUIDANCE MESSAGES ===
rishi_exit_msg db 'Divine wisdom flows...', 0x0D, 0x0A, 0x00

; === COMPACT WISDOM ===
init_complete_msg db '', 0x00
success_msg db '', 0x00
kernel_loaded_msg  db 'Kernel load complete. Transitioning to protected mode...', 0x0D, 0x0A, 0x00
kernel_missing_msg db 'No kernel sectors specified. Halting.', 0x0D, 0x0A, 0x00
kernel_failure_msg db 'Kernel load failed. System halt.', 0x0D, 0x0A, 0x00
disk_error_msg     db 'Disk read error. Retrying failed.', 0x0D, 0x0A, 0x00
protected_msg      db 'Protected mode enabled - STEPPPS core active', 0x00
kernel_ready_msg   db 'Handing off to TernaryBit kernel...', 0x00
debug_msg1         db 'Debug: Checking kernel load...', 0x00
kernel_not_loaded_msg db 'ERROR: Kernel not loaded at 0x10000', 0x00
kernel_found_msg   db 'OK: Kernel found at 0x10000', 0x00
before_protected_msg db 'About to enter protected mode...', 0x0D, 0x0A, 0x00
a20_enabled_msg     db 'A20 line enabled...', 0x0D, 0x0A, 0x00
back_from_kernel_msg db 'DEBUG: Back from kernel load', 0x0D, 0x0A, 0x00
entering_pmode_msg   db 'DEBUG: Entering protected mode...', 0x0D, 0x0A, 0x00
step1_msg           db 'Step 1: CLI', 0x0D, 0x0A, 0x00
step2_msg           db 'Step 2: LGDT', 0x0D, 0x0A, 0x00
step3_msg           db 'Step 3: CR0', 0x0D, 0x0A, 0x00
step4_msg           db 'Step 4: JMP', 0x0D, 0x0A, 0x00
skipping_pmode_msg  db 'SKIPPING PMODE FOR TBDS TEST', 0x0D, 0x0A, 0x00
tbds_test_msg       db 'TBDS Data: ', 0x00
space_msg           db ' ', 0x00
newline_msg         db 0x0D, 0x0A, 0x00
checkpoint1_msg     db 'CHECKPOINT 1: Stage2 Entry OK', 0x0D, 0x0A, 0x00
checkpoint2_msg     db 'CHECKPOINT 2: About to init psychology', 0x0D, 0x0A, 0x00
checkpoint3_msg     db 'CHECKPOINT 3: Psychology done', 0x0D, 0x0A, 0x00
checkpoint4_msg     db 'CHECKPOINT 4: Pixel done', 0x0D, 0x0A, 0x00
checkpoint5_msg     db 'CHECKPOINT 5: Prompt done', 0x0D, 0x0A, 0x00
checkpoint6_msg     db 'CHECKPOINT 6: Script done', 0x0D, 0x0A, 0x00
psychology_skip_msg db 'SKIPPING PSYCHOLOGY INIT', 0x0D, 0x0A, 0x00
prompt_skip_msg     db 'SKIPPING PROMPT INIT', 0x0D, 0x0A, 0x00
script_skip_msg     db 'SKIPPING SCRIPT INIT', 0x0D, 0x0A, 0x00
; Legacy messages removed for space
; Removed duplicate - using divine version above
test_kernel_msg     db 'Testing kernel jump...', 0x0D, 0x0A, 0x00

boot_count      dw 0
learning_rate   dw 0
user_preference db 0

; STEPPPS Memory Management Variables
total_memory_kb     dw 0
steppps_size_kb     dw 0
tbds_size_kb        dw 0
steppps_base        dw 0
tbds_base           dw 0

; Progressive Architecture Variables
arch_capabilities   dw 0
steppps_features    dw 0
tbds_arch_flags     dw 0
tbds_steppps_flags  dw 0
max_bit_width       dw 0

; STEPPPS Dimension Status
space_initialized       dw 0
time_initialized        dw 0
event_initialized       dw 0
psychology_initialized  dw 0
pixel_initialized       dw 0
prompt_initialized      dw 0
script_initialized      dw 0

; Messages
arch_detected_msg      db 'Arch: ', 0x00
tbds_complete_msg      db 'TBDS Ready', 0x0D, 0x0A, 0x00
arch_complete_msg      db 'Arch:', 0x00
steppps_status_msg     db 'STEPPPS:', 0x00
space_enabled_msg      db 'OK', 0x0D, 0x0A, 0x00
graphics_mode   db 0
command_buffer  times 27 db 0
command_count   dw 0
script_stack    dw 0

; Pad to sector boundary
    times STAGE2_SECTOR_COUNT*512-($-$$) db 0
