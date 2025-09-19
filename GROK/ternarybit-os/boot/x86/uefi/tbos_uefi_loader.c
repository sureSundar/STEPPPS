#include <efi.h>
#include <efilib.h>

#include "tbos_boot_descriptor.h"

#define TBDS_BUFFER_SIZE 4096

/* Local payload structures ------------------------------------------------ */
typedef struct {
    uint16_t arch_id;
    uint16_t word_size_bits;
    uint16_t feature_flags;   /* bit0=real mode, bit1=protected, bit2=long */
    uint16_t abi_version;
} __attribute__((packed)) tbds_arch_payload_t;

typedef struct {
    uint16_t firmware_type;   /* 1 = BIOS, 2 = UEFI */
    uint16_t reserved;
    uint32_t revision;
    uint16_t vendor_length;
    /* UTF-16 vendor name follows */
} __attribute__((packed)) tbds_fw_payload_t;

static UINT8 *tbds_cursor_advance(tbds_header_t *hdr,
                                  UINT8 *cursor,
                                  uint16_t type,
                                  uint16_t flags,
                                  uint32_t length,
                                  const void *payload)
{
    cursor = tbds_append_descriptor(cursor, type, flags, length, payload);
    hdr->descriptor_count += 1;
    hdr->total_length = (uint32_t)(cursor - (UINT8 *)hdr);
    return cursor;
}

static UINT8 *emit_arch_descriptor(tbds_header_t *hdr, UINT8 *cursor)
{
    const tbds_arch_payload_t payload = {
        .arch_id = 0x0002,           /* x86_64 */
        .word_size_bits = 64,
        .feature_flags = 0x0006,     /* protected + long mode */
        .abi_version = 0x0100
    };
    return tbds_cursor_advance(hdr, cursor,
                               TBDS_TYPE_ARCH_INFO,
                               0,
                               sizeof(payload),
                               &payload);
}

static UINT8 *emit_firmware_descriptor(tbds_header_t *hdr,
                                       UINT8 *cursor,
                                       EFI_SYSTEM_TABLE *SystemTable)
{
    const CHAR16 *vendor = SystemTable->FirmwareVendor ? SystemTable->FirmwareVendor
                                                        : L"Unknown";
    const UINT16 vendor_len = (UINT16)StrLen(vendor);
    const UINT32 payload_len = sizeof(tbds_fw_payload_t) + vendor_len * sizeof(CHAR16);

    VOID *payload_buffer = NULL;
    EFI_STATUS status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool,
                                          3,
                                          EfiLoaderData,
                                          payload_len,
                                          &payload_buffer);
    if (EFI_ERROR(status) || payload_buffer == NULL) {
        Print(L"Failed to allocate firmware payload: %r\n", status);
        return cursor;
    }

    tbds_fw_payload_t *fw = (tbds_fw_payload_t *)payload_buffer;
    fw->firmware_type = 0x0002;
    fw->reserved = 0;
    fw->revision = SystemTable->Hdr.Revision;
    fw->vendor_length = vendor_len;

    CHAR16 *vendor_out = (CHAR16 *)(fw + 1);
    if (vendor_len) {
        CopyMem(vendor_out, vendor, vendor_len * sizeof(CHAR16));
    }

    cursor = tbds_cursor_advance(hdr, cursor,
                                 TBDS_TYPE_FIRMWARE_INFO,
                                 0,
                                 payload_len,
                                 payload_buffer);

    uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, payload_buffer);
    return cursor;
}

static UINT8 *emit_telemetry_descriptor(tbds_header_t *hdr, UINT8 *cursor)
{
    static const CHAR8 json[] =
        "{\n"
        "  \"steppps\": {\n"
        "    \"space\": { \"firmware\": \"UEFI\" },\n"
        "    \"time\": { \"boot_phase\": \"uefi-loader\" },\n"
        "    \"event\": { \"status\": \"initializing\" }\n"
        "  }\n"
        "}";

    return tbds_cursor_advance(hdr, cursor,
                               TBDS_TYPE_STEPPPS_TELEMETRY,
                               0,
                               sizeof(json) - 1,
                               json);
}

static UINT8 *emit_memory_map(tbds_header_t *hdr,
                              UINT8 *cursor,
                              EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    EFI_MEMORY_DESCRIPTOR *mem_map = NULL;
    UINTN map_size = 0, map_key, desc_size;
    UINT32 desc_version;

    status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5,
                               &map_size, mem_map, &map_key, &desc_size, &desc_version);
    if (status != EFI_BUFFER_TOO_SMALL) {
        return cursor;
    }

    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3,
                               EfiLoaderData, map_size, (void **)&mem_map);
    if (EFI_ERROR(status) || mem_map == NULL) {
        Print(L"Failed to allocate memory map buffer: %r\n", status);
        return cursor;
    }

    status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5,
                               &map_size, mem_map, &map_key, &desc_size, &desc_version);
    if (EFI_ERROR(status)) {
        Print(L"GetMemoryMap failed: %r\n", status);
        uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, mem_map);
        return cursor;
    }

    UINT32 entry_count = (UINT32)(map_size / desc_size);
    UINT32 payload_len = entry_count * 20;

    VOID *payload = NULL;
    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3,
                               EfiLoaderData, payload_len, &payload);
    if (EFI_ERROR(status) || payload == NULL) {
        Print(L"Failed to allocate memory map payload: %r\n", status);
        uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, mem_map);
        return cursor;
    }

    UINT8 *out = (UINT8 *)payload;
    for (UINT32 i = 0; i < entry_count; ++i) {
        EFI_MEMORY_DESCRIPTOR *d = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)mem_map + (i * desc_size));
        *((UINT64 *)(out + i * 20 + 0)) = d->PhysicalStart;
        *((UINT64 *)(out + i * 20 + 8)) = d->NumberOfPages * 4096ULL;
        *((UINT32 *)(out + i * 20 + 16)) = d->Type;
    }

    cursor = tbds_cursor_advance(hdr, cursor,
                                 TBDS_TYPE_MEMORY_MAP,
                                 0,
                                 payload_len,
                                 payload);

    uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, payload);
    uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, mem_map);
    return cursor;
}

static UINT8 *emit_console_info(tbds_header_t *hdr,
                                UINT8 *cursor,
                                EFI_SYSTEM_TABLE *SystemTable)
{
    if (!SystemTable->ConOut || !SystemTable->ConOut->Mode) {
        return cursor;
    }

    struct {
        UINT16 type;
        UINT16 columns;
        UINT16 rows;
    } __attribute__((packed)) payload;

    payload.type = 1; /* text console */
    payload.columns = (UINT16)SystemTable->ConOut->Mode->Columns;
    payload.rows = (UINT16)SystemTable->ConOut->Mode->Rows;

    return tbds_cursor_advance(hdr, cursor,
                               TBDS_TYPE_CONSOLE_INFO,
                               0,
                               sizeof(payload),
                               &payload);
}

static EFI_STATUS load_kernel_stub(void)
{
    Print(L"[TODO] Kernel loading not implemented yet.\n");
    return EFI_UNSUPPORTED;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    Print(L"TBOS UEFI Loader starting...\n");

    UINT8 *buffer = NULL;
    EFI_STATUS status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool,
                                          3,
                                          EfiLoaderData,
                                          TBDS_BUFFER_SIZE,
                                          (void **)&buffer);
    if (EFI_ERROR(status) || buffer == NULL) {
        Print(L"Failed to allocate descriptor pool: %r\n", status);
        return status;
    }

    tbds_header_t *header = (tbds_header_t *)buffer;
    tbds_init_header(header);
    UINT8 *cursor = buffer + sizeof(tbds_header_t);

    cursor = emit_arch_descriptor(header, cursor);
    cursor = emit_firmware_descriptor(header, cursor, SystemTable);
    cursor = emit_memory_map(header, cursor, SystemTable);
    cursor = emit_console_info(header, cursor, SystemTable);
    cursor = emit_telemetry_descriptor(header, cursor);

    Print(L"Descriptors prepared (%d entries, %d bytes).\n",
          header->descriptor_count,
          header->total_length);

    status = load_kernel_stub();
    if (EFI_ERROR(status)) {
        Print(L"Loader stub returned: %r\n", status);
    }

    uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, buffer);
    return status;
}
