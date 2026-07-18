/*
 * TernaryBit OS Kernel - STEPPPS Framework Implementation
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 * Sprint 5-11: Core Kernel Implementation
 */

#include "kernel.h"
#include "kernel_headers.h"

// Forward declarations
void memory_init(void);
void interrupt_init(void);
void timer_init(uint32_t freq);

uint32_t g_tbds_pointer = 0;
uint32_t g_tbds_length = 0;
boot_descriptor_context_t g_boot_descriptor;
boot_memory_map_entry_t g_boot_memory_map[BOOT_MEMORY_MAP_MAX_ENTRIES];
int g_boot_memory_map_entries = 0;

static void boot_descriptor_reset(void)
{
    g_boot_descriptor.arch_id = 0;
    g_boot_descriptor.arch_word_bits = 0;
    g_boot_descriptor.arch_features = 0;
    g_boot_descriptor.firmware_type = 0;
    g_boot_descriptor.firmware_revision = 0;
    g_boot_descriptor.boot_stage_id = 0;
    g_boot_descriptor.boot_drive = 0;
    g_boot_descriptor.boot_lba_start = 0;
    g_boot_descriptor.boot_sector_count = 0;
    g_boot_descriptor.memory_map_entries = 0;
    g_boot_descriptor.total_memory_kb = 0;
    g_boot_descriptor.console_type = 0;
    g_boot_descriptor.console_columns = 0;
    g_boot_descriptor.console_rows = 0;
    g_boot_descriptor.descriptors_seen = 0;
    g_boot_descriptor.telemetry_descriptors = 0;
    g_boot_descriptor.valid = 0;
    g_boot_memory_map_entries = 0;
    for (int i = 0; i < BOOT_MEMORY_MAP_MAX_ENTRIES; ++i) {
        g_boot_memory_map[i].base = 0;
        g_boot_memory_map[i].length = 0;
        g_boot_memory_map[i].type = 0;
        g_boot_memory_map[i].attributes = 0;
    }
}

static void print_label_value(const char* label, const char* value)
{
    kernel_printf(label);
    kernel_printf(value);
    kernel_printf("\n");
}

static void print_label_int(const char* label, int value)
{
    char buf[16];
    int_to_string(value, buf);
    print_label_value(label, buf);
}

static void print_label_hex(const char* label, u32 value)
{
    char buf[16];
    hex32_to_string(value, buf);
    print_label_value(label, buf);
}

void parse_boot_descriptors(void)
{
    boot_descriptor_reset();

    if (g_tbds_pointer == 0) {
        kernel_printf("No TBDS stream provided by bootloader.\n\n");
        return;
    }

    tbds_header_t* header = (tbds_header_t*)(uintptr_t)g_tbds_pointer;
    if (!header || header->signature != TBDS_SIGNATURE) {
        kernel_printf("Invalid TBDS signature.\n\n");
        return;
    }

    uint32_t total_length = header->total_length;
    if (g_tbds_length != 0 && g_tbds_length < total_length) {
        total_length = g_tbds_length;
    }
    if (total_length < sizeof(tbds_header_t)) {
        kernel_printf("TBDS header truncated.\n\n");
        return;
    }

    uint8_t* base = (uint8_t*)header;
    uint8_t* cursor = base + sizeof(tbds_header_t);
    uint8_t* end = base + total_length;

    uint16_t processed = 0;
    while (cursor + sizeof(tbds_tlv_t) <= end && processed < header->descriptor_count) {
        tbds_tlv_t* tlv = (tbds_tlv_t*)cursor;
        cursor += sizeof(tbds_tlv_t);

        if (cursor + tlv->length > end) {
            break;
        }

        g_boot_descriptor.descriptors_seen++;

        switch (tlv->type) {
        case TBDS_TYPE_ARCH_INFO:
            if (tlv->length >= 8) {
                const uint16_t* fields = (const uint16_t*)cursor;
                g_boot_descriptor.arch_id = fields[0];
                g_boot_descriptor.arch_word_bits = fields[1];
                g_boot_descriptor.arch_features = fields[2];
            }
            break;
        case TBDS_TYPE_FIRMWARE_INFO:
            if (tlv->length >= 8) {
                const uint16_t* fw = (const uint16_t*)cursor;
                g_boot_descriptor.firmware_type = fw[0];
                g_boot_descriptor.firmware_revision = *((const uint32_t*)(cursor + 4));
            }
            break;
        case TBDS_TYPE_MEMORY_MAP:
        {
            if (tlv->length >= 20) {
                const uint16_t entry_size = 20;
                uint16_t available_entries = (uint16_t)(tlv->length / entry_size);
                if (available_entries > BOOT_MEMORY_MAP_MAX_ENTRIES) {
                    available_entries = BOOT_MEMORY_MAP_MAX_ENTRIES;
                }
                uint64_t total_ram = 0;
                for (uint16_t i = 0; i < available_entries; ++i) {
                    const uint8_t* src = cursor + (i * entry_size);
                    boot_memory_map_entry_t* dst = &g_boot_memory_map[i];
                    dst->base = *((const uint64_t*)(src + 0));
                    dst->length = *((const uint64_t*)(src + 8));
                    dst->type = *((const uint32_t*)(src + 16));
                    dst->attributes = 0;
                    if (dst->type == 1) {
                        total_ram += dst->length;
                    }
                }
                g_boot_memory_map_entries = available_entries;
                g_boot_descriptor.memory_map_entries = available_entries;
                if (total_ram > 0) {
                    uint64_t kb = total_ram >> 10;
                    if (kb > 0xFFFFFFFFULL) {
                        kb = 0xFFFFFFFFULL;
                    }
                    g_boot_descriptor.total_memory_kb = (uint32_t)kb;
                }
            }
        }
            break;
        case TBDS_TYPE_BOOT_DEVICE:
            if (tlv->length >= 12) {
                g_boot_descriptor.boot_drive = cursor[0];
                g_boot_descriptor.boot_stage_id = *((const uint16_t*)(cursor + 2));
                g_boot_descriptor.boot_lba_start = *((const uint32_t*)(cursor + 4));
                g_boot_descriptor.boot_sector_count = *((const uint32_t*)(cursor + 8));
            }
            break;
        case TBDS_TYPE_CONSOLE_INFO:
            if (tlv->length >= 6) {
                const uint16_t* console = (const uint16_t*)cursor;
                g_boot_descriptor.console_type = console[0];
                g_boot_descriptor.console_columns = console[1];
                g_boot_descriptor.console_rows = console[2];
            }
            break;
        case TBDS_TYPE_STEPPPS_TELEMETRY:
            g_boot_descriptor.telemetry_descriptors += 1;
            break;
        default:
            /* ignore unknown descriptors */
            break;
        }

        cursor += tlv->length;
        processed++;
    }

    g_boot_descriptor.valid = 1;
}

void show_boot_descriptor_summary(void)
{
    kernel_printf("Boot Descriptor Summary:\n");

    if (!g_boot_descriptor.valid) {
        kernel_printf("  (no descriptor data)\n\n");
        return;
    }

    print_label_int("  descriptors seen: ", g_boot_descriptor.descriptors_seen);
    print_label_int("  telemetry entries: ", g_boot_descriptor.telemetry_descriptors);

    if (g_boot_descriptor.arch_id != 0) {
        print_label_int("  arch id: ", g_boot_descriptor.arch_id);
        print_label_int("  arch bits: ", g_boot_descriptor.arch_word_bits);
    }

    if (g_boot_descriptor.firmware_type != 0) {
        print_label_int("  firmware type: ", g_boot_descriptor.firmware_type);
        print_label_hex("  firmware rev: ", g_boot_descriptor.firmware_revision);
    }

    if (g_boot_descriptor.boot_drive != 0 || g_boot_descriptor.boot_lba_start != 0) {
        print_label_int("  boot drive: ", g_boot_descriptor.boot_drive);
        print_label_int("  boot stage id: ", g_boot_descriptor.boot_stage_id);
        print_label_hex("  boot LBA start: ", g_boot_descriptor.boot_lba_start);
        print_label_int("  boot sector count: ", g_boot_descriptor.boot_sector_count);
    }

    if (g_boot_descriptor.memory_map_entries > 0) {
        print_label_int("  memory map entries: ", g_boot_descriptor.memory_map_entries);
        print_label_int("  RAM (KB): ", (int)g_boot_descriptor.total_memory_kb);
        kernel_printf("  memory regions:\n");
        int entries = g_boot_memory_map_entries;
        if (entries > g_boot_descriptor.memory_map_entries) {
            entries = g_boot_descriptor.memory_map_entries;
        }
        if (entries > BOOT_MEMORY_MAP_MAX_ENTRIES) {
            entries = BOOT_MEMORY_MAP_MAX_ENTRIES;
        }
        if (entries > 5) {
            entries = 5;
        }
        for (int i = 0; i < entries; ++i) {
            char index_buf[8];
            char base_buf[24];
            char length_buf[24];
            char type_buf[12];
            int_to_string(i, index_buf);
            hex64_to_string(g_boot_memory_map[i].base, base_buf);
            hex64_to_string(g_boot_memory_map[i].length, length_buf);
            int_to_string(g_boot_memory_map[i].type, type_buf);
            kernel_printf("    [");
            kernel_printf(index_buf);
            kernel_printf("] base=");
            kernel_printf(base_buf);
            kernel_printf(" len=");
            kernel_printf(length_buf);
            kernel_printf(" type=");
            kernel_printf(type_buf);
            kernel_printf("\n");
        }
    }

    if (g_boot_descriptor.console_columns != 0) {
        print_label_int("  console columns: ", g_boot_descriptor.console_columns);
        print_label_int("  console rows: ", g_boot_descriptor.console_rows);
    }

    kernel_printf("\n");
}

// STEPPPS Dimension State
static steppps_state_t steppps_state;

// Kernel entry point (called from bootloader)
// Forward declare shell
void shell_init(void);
void shell_main(void);

void kernel_main(void) {
    // Clear screen
    volatile uint16_t* video = (uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        video[i] = 0x0F20;  // White on black space
    }

    // Display boot message
    const char* msg = "TBOS READY";
    for (size_t i = 0; msg[i] && i < 10; ++i) {
        video[i] = 0x1F00 | msg[i];
    }

    kernel_printf("\nTernaryBit OS v1.0 Booting...\n");
    kernel_printf("Swamiye Saranam Aiyappa\n\n");

    // Initialize core systems
    kernel_printf("[INIT] Initializing interrupt system...\n");
    interrupt_init();

    kernel_printf("[INIT] Initializing memory system...\n");
    memory_init();

    kernel_printf("[INIT] Initializing timer system...\n");
    timer_init(1000);  // 1000 Hz timer

    // Initialize STEPPPS framework
    kernel_printf("[INIT] Initializing STEPPPS framework...\n");
    init_steppps_framework();

    // Initialize keyboard
    kernel_printf("[INIT] Initializing keyboard driver...\n");
    keyboard_init();

    // Initialize shell
    kernel_printf("[INIT] Initializing shell...\n");
    shell_init();

    kernel_printf("[INIT] Boot complete! Starting shell...\n");

    // Start interactive shell
    shell_main();

    // Should never reach here
    for (;;) {
        asm volatile ("hlt");
    }
}

void init_steppps_framework(void) {
    kernel_printf("Initializing STEPPPS Framework:\n");

    // Initialize each dimension
    init_space_dimension();
    init_time_dimension();
    init_event_dimension();
    init_psychology_dimension();
    init_pixel_dimension();
    init_prompt_dimension();
    init_script_dimension();

    kernel_printf("STEPPPS Framework: All dimensions active\n\n");
}

// SPACE Dimension - Hardware and Resource Management
void init_space_dimension(void) {
    kernel_printf("[SPACE] Hardware resource management...\n");

    // Hardware detection using descriptors where possible
    steppps_state.space.cpu_count = detect_cpu_count();
    steppps_state.space.memory_size = detect_memory_size();
    if (g_boot_descriptor.valid && g_boot_descriptor.total_memory_kb > 0) {
        steppps_state.space.memory_size = g_boot_descriptor.total_memory_kb;
    }
    steppps_state.space.devices_found = detect_hardware_devices();

    // Resource allocation setup
    steppps_state.space.resource_pools_initialized = 1;
    steppps_state.space.active = 1;

    kernel_printf("  CPU cores: %d\n", steppps_state.space.cpu_count);
    kernel_printf("  Memory: %d MB\n", steppps_state.space.memory_size / 1024);
    kernel_printf("  Devices: %d found\n", steppps_state.space.devices_found);
}

// TIME Dimension - Scheduling and Temporal Coordination
void init_time_dimension(void) {
    kernel_printf("[TIME] Temporal coordination system...\n");

    // Initialize system timer
    steppps_state.time.system_uptime = 0;
    steppps_state.time.timer_frequency = 1000;  // 1000 Hz
    steppps_state.time.scheduler_quantum = 10;  // 10ms

    // AI-enhanced scheduler
    steppps_state.time.ai_scheduler_active = 1;
    steppps_state.time.active = 1;

    kernel_printf("  Timer frequency: %d Hz\n", steppps_state.time.timer_frequency);
    kernel_printf("  Scheduler quantum: %d ms\n", steppps_state.time.scheduler_quantum);
}

// EVENT Dimension - System Events and Monitoring
void init_event_dimension(void) {
    kernel_printf("[EVENT] Event monitoring system...\n");

    steppps_state.event.events_processed = 0;
    steppps_state.event.interrupt_count = 0;
    steppps_state.event.error_count = 0;

    // Event queue initialization
    steppps_state.event.event_queue_size = 1024;
    steppps_state.event.active = 1;

    kernel_printf("  Event queue: %d entries\n", steppps_state.event.event_queue_size);
}

// PSYCHOLOGY Dimension - AI Learning and Adaptation
void init_psychology_dimension(void) {
    kernel_printf("[PSYCHOLOGY] AI learning system...\n");

    steppps_state.psychology.learning_cycles = 0;
    steppps_state.psychology.adaptation_rate = 0.1f;  // 10% adaptation rate
    steppps_state.psychology.confidence_level = 1.0f; // 100% initial confidence

    // Neural network initialization (simplified)
    steppps_state.psychology.neural_net_active = 1;
    steppps_state.psychology.active = 1;

    kernel_printf("  Neural network: Active\n");
    kernel_printf("  Learning rate: %.1f%%\n", steppps_state.psychology.adaptation_rate * 100);
}

// PIXEL Dimension - Graphics and Visualization
void init_pixel_dimension(void) {
    kernel_printf("[PIXEL] Graphics subsystem...\n");

    // Initialize graphics mode
    steppps_state.pixel.graphics_mode_active = 0;  // Text mode for now
    if (g_boot_descriptor.console_columns != 0) {
        steppps_state.pixel.resolution_x = g_boot_descriptor.console_columns;
        steppps_state.pixel.resolution_y = g_boot_descriptor.console_rows;
    } else {
        steppps_state.pixel.resolution_x = 80;
        steppps_state.pixel.resolution_y = 25;
    }
    steppps_state.pixel.color_depth = 4;  // 16 colors

    steppps_state.pixel.active = 1;

    kernel_printf("  Display: %dx%d text mode\n",
                  steppps_state.pixel.resolution_x,
                  steppps_state.pixel.resolution_y);
}

// PROMPT Dimension - AI Interface and Natural Language
void init_prompt_dimension(void) {
    kernel_printf("[PROMPT] AI interface system...\n");

    steppps_state.prompt.commands_processed = 0;
    steppps_state.prompt.ai_responses_generated = 0;
    steppps_state.prompt.language_model_active = 1;

    // Command recognition setup
    steppps_state.prompt.active = 1;

    kernel_printf("  Language model: Active\n");
    kernel_printf("  Command recognition: Ready\n");
}

// SCRIPT Dimension - Automation and Orchestration
void init_script_dimension(void) {
    kernel_printf("[SCRIPT] Automation engine...\n");

    steppps_state.script.scripts_executed = 0;
    steppps_state.script.automation_level = 80;  // 80% automation
    steppps_state.script.orchestrator_active = 1;

    steppps_state.script.active = 1;

    kernel_printf("  Automation level: %d%%\n", steppps_state.script.automation_level);
    kernel_printf("  Orchestrator: Active\n");
}

void start_steppps_orchestrator(void) {
    kernel_printf("Starting STEPPPS Orchestrator...\n");

    // Main orchestration loop will run in background
    steppps_state.orchestrator_active = 1;
    steppps_state.coordination_cycles = 0;

    kernel_printf("STEPPPS Orchestrator: Running\n\n");
}

void kernel_main_loop(void) {
    kernel_printf("TernaryBit kernel ready.\n");
    kernel_printf("Type 'help' for available commands.\n\n");

    char command_buffer[256];

    while (1) {
        kernel_printf("ternarybit> ");

        // Read command (simplified keyboard input)
        read_command(command_buffer);

        // Process command through STEPPPS
        process_steppps_command(command_buffer);

        // Update STEPPPS dimensions
        update_steppps_dimensions();

        // Yield CPU (in real implementation)
        kernel_yield();
    }
}

void process_steppps_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        show_help();
    } else if (strcmp(command, "steppps") == 0) {
        show_steppps_status();
    } else if (strcmp(command, "bootinfo") == 0) {
        show_boot_descriptor_summary();
    } else if (strcmp(command, "stats") == 0) {
        show_system_stats();
    } else if (strcmp(command, "ai") == 0) {
        show_ai_status();
    } else if (strcmp(command, "reboot") == 0) {
        kernel_reboot();
    } else if (strlen(command) > 0) {
        // Use AI to understand unknown commands
        process_ai_command(command);
    }

    // Update command statistics
    steppps_state.prompt.commands_processed++;
}

void show_help(void) {
    kernel_printf("TernaryBit OS Commands:\n");
    kernel_printf("  help     - Show this help\n");
    kernel_printf("  steppps  - Show STEPPPS dimension status\n");
    kernel_printf("  bootinfo - Show boot descriptor telemetry\n");
    kernel_printf("  stats    - Show system statistics\n");
    kernel_printf("  ai       - Show AI subsystem status\n");
    kernel_printf("  reboot   - Restart system\n");
    kernel_printf("\n");
}

void show_steppps_status(void) {
    kernel_printf("\nSTEPPPS Dimension Status:\n");
    kernel_printf("========================\n");

    kernel_printf("SPACE:      %s (CPU:%d, RAM:%dMB)\n",
                  steppps_state.space.active ? "Active" : "Inactive",
                  steppps_state.space.cpu_count,
                  steppps_state.space.memory_size / 1024);

    kernel_printf("TIME:       %s (Uptime: %ds)\n",
                  steppps_state.time.active ? "Active" : "Inactive",
                  steppps_state.time.system_uptime);

    kernel_printf("EVENT:      %s (Events: %d)\n",
                  steppps_state.event.active ? "Active" : "Inactive",
                  steppps_state.event.events_processed);

    kernel_printf("PSYCHOLOGY: %s (Cycles: %d)\n",
                  steppps_state.psychology.active ? "Active" : "Inactive",
                  steppps_state.psychology.learning_cycles);

    kernel_printf("PIXEL:      %s (%dx%d)\n",
                  steppps_state.pixel.active ? "Active" : "Inactive",
                  steppps_state.pixel.resolution_x,
                  steppps_state.pixel.resolution_y);

    kernel_printf("PROMPT:     %s (Commands: %d)\n",
                  steppps_state.prompt.active ? "Active" : "Inactive",
                  steppps_state.prompt.commands_processed);

    kernel_printf("SCRIPT:     %s (Auto: %d%%)\n",
                  steppps_state.script.active ? "Active" : "Inactive",
                  steppps_state.script.automation_level);

    kernel_printf("\n");
}

void show_system_stats(void) {
    kernel_printf("\nSystem Statistics:\n");
    kernel_printf("==================\n");
    kernel_printf("Uptime: %d seconds\n", steppps_state.time.system_uptime);
    kernel_printf("Coordination cycles: %d\n", steppps_state.coordination_cycles);
    kernel_printf("Commands processed: %d\n", steppps_state.prompt.commands_processed);
    kernel_printf("Events handled: %d\n", steppps_state.event.events_processed);
    kernel_printf("Learning cycles: %d\n", steppps_state.psychology.learning_cycles);
    kernel_printf("\n");
}

void show_ai_status(void) {
    kernel_printf("\nAI Subsystem Status:\n");
    kernel_printf("===================\n");
    kernel_printf("Neural Network: %s\n",
                  steppps_state.psychology.neural_net_active ? "Active" : "Inactive");
    kernel_printf("Confidence Level: %.1f%%\n",
                  steppps_state.psychology.confidence_level * 100);
    kernel_printf("Language Model: %s\n",
                  steppps_state.prompt.language_model_active ? "Active" : "Inactive");
    kernel_printf("AI Scheduler: %s\n",
                  steppps_state.time.ai_scheduler_active ? "Active" : "Inactive");
    kernel_printf("\n");
}

void process_ai_command(const char* command) {
    kernel_printf("AI analyzing command: '%s'\n", command);

    // Simple AI response (in real implementation, this would use ML)
    if (strstr(command, "hello") || strstr(command, "hi")) {
        kernel_printf("AI: Hello! I'm the TernaryBit AI assistant. How can I help?\n");
    } else if (strstr(command, "status")) {
        kernel_printf("AI: System is running optimally. All STEPPPS dimensions active.\n");
    } else if (strstr(command, "optimize")) {
        kernel_printf("AI: Running system optimization...\n");
        // Trigger optimization routines
        steppps_state.psychology.learning_cycles++;
    } else {
        kernel_printf("AI: I understand you said '%s'. Learning this command pattern.\n", command);
        steppps_state.psychology.learning_cycles++;
    }

    steppps_state.prompt.ai_responses_generated++;
}

void update_steppps_dimensions(void) {
    // Update system time
    steppps_state.time.system_uptime++;

    // Increment coordination cycles
    steppps_state.coordination_cycles++;

    // AI learning (simplified)
    if (steppps_state.coordination_cycles % 100 == 0) {
        steppps_state.psychology.learning_cycles++;

        // Adapt confidence based on successful operations
        if (steppps_state.event.error_count == 0) {
            steppps_state.psychology.confidence_level =
                (steppps_state.psychology.confidence_level * 0.99f) + 0.01f;
        }
    }
}

void kernel_yield(void) {
    // In real implementation, this would yield to scheduler
    // For now, just a simple delay
    volatile int i;
    for (i = 0; i < 100000; i++) {
        // Busy wait
    }
}

void kernel_reboot(void) {
    kernel_printf("TernaryBit OS shutting down...\n");
    kernel_printf("STEPPPS dimensions deactivating...\n");

    // Deactivate STEPPPS dimensions
    steppps_state.orchestrator_active = 0;

    // Trigger system reboot
    asm volatile("int $0x19");  // BIOS reboot interrupt
}
