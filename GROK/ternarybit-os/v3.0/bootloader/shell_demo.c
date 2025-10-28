/*
 * TBOS v3.0 Universal Shell Interactive Demo
 * Shows the shell commands in action across different personas
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simplified persona definitions for demo
typedef enum {
    DEMO_CALCULATOR = 0,
    DEMO_EMBEDDED = 1,
    DEMO_X86 = 2,
    DEMO_ARM64 = 4,
    DEMO_CHEMOS = 7,
    DEMO_UNIVERSAL = 8
} demo_persona_t;

typedef struct {
    demo_persona_t persona;
    const char* name;
    const char* prompt;
    const char* description;
} persona_info_t;

static persona_info_t personas[] = {
    {DEMO_CALCULATOR, "Calculator", "calc> ", "4-bit scientific calculator"},
    {DEMO_EMBEDDED, "Embedded", "embed> ", "8-bit IoT microcontroller"},
    {DEMO_X86, "x86", "x86> ", "32/64-bit PC workstation"},
    {DEMO_ARM64, "ARM64", "arm64> ", "64-bit mobile/server"},
    {DEMO_CHEMOS, "ChemOS", "chemos> ", "Quantum chemical computer"},
    {DEMO_UNIVERSAL, "Universal", "tbos> ", "Adaptive universal system"}
};

void show_help(demo_persona_t persona) {
    printf("\nTBOS v3.0 Universal Shell Commands:\n");
    printf("====================================\n");
    printf("help      - Display this help information\n");
    printf("persona   - Switch between hardware personas\n");
    printf("status    - Show current system status\n");
    printf("ls        - List files with persona-aware formatting\n");
    printf("bridge    - Manage cross-architecture bridges\n");
    printf("boot      - Boot kernel for current persona\n");
    if (persona == DEMO_CHEMOS) {
        printf("quantum   - Execute quantum operations (ChemOS only)\n");
    }
    printf("exit      - Exit the shell\n\n");
}

void show_status(demo_persona_t persona) {
    const char* name = "Unknown";
    const char* desc = "Unknown system";

    for (int i = 0; i < 6; i++) {
        if (personas[i].persona == persona) {
            name = personas[i].name;
            desc = personas[i].description;
            break;
        }
    }

    printf("\n🖥️ TBOS v3.0 System Status:\n");
    printf("===========================\n");
    printf("Current Persona: %s (%d)\n", name, persona);
    printf("Description: %s\n", desc);
    printf("Quantum Support: %s\n", (persona == DEMO_CHEMOS) ? "✅ Enabled" : "❌ Disabled");
    printf("Memory Regions: %d\n", (persona == DEMO_CHEMOS) ? 16 : (persona >= DEMO_ARM64) ? 8 : 4);
    printf("Boot Time: %dms\n", (persona == DEMO_CALCULATOR) ? 50 : (persona == DEMO_CHEMOS) ? 3000 : 1500);
    printf("Commands Executed: %d\n", rand() % 50 + 10);
    printf("Session Uptime: %d minutes\n\n", rand() % 120 + 5);
}

void show_persona_switch(demo_persona_t from, demo_persona_t to) {
    const char* from_name = "Unknown";
    const char* to_name = "Unknown";

    for (int i = 0; i < 6; i++) {
        if (personas[i].persona == from) from_name = personas[i].name;
        if (personas[i].persona == to) to_name = personas[i].name;
    }

    printf("\n🔄 Runtime Persona Switching:\n");
    printf("==============================\n");
    printf("Switching from %s → %s...\n", from_name, to_name);
    printf("💾 Saving CPU state for %s\n", from_name);
    printf("🧠 Migrating memory mappings\n");
    if (to == DEMO_CHEMOS) {
        printf("⚛️ Preserving quantum coherence for 118 elements\n");
    }
    printf("⚡ Restoring CPU state for %s\n", to_name);
    printf("✅ Persona switch completed in %d microseconds\n", rand() % 200 + 10);
    printf("🎯 Now running as %s persona\n\n", to_name);
}

void show_ls(demo_persona_t persona) {
    printf("\n📁 Persona-Aware File Listing:\n");
    printf("===============================\n");

    if (persona == DEMO_CALCULATOR) {
        printf("calc.dat      - Calculation data (4KB)\n");
        printf("functions.lib - Math functions (8KB)\n");
    } else if (persona == DEMO_EMBEDDED) {
        printf("firmware.bin  - Device firmware (64KB)\n");
        printf("config.sys    - System configuration (2KB)\n");
        printf("sensors.log   - Sensor data (16KB)\n");
    } else if (persona == DEMO_X86) {
        printf("kernel.img    - OS kernel (2MB)\n");
        printf("drivers/      - Hardware drivers\n");
        printf("boot.cfg      - Boot configuration (4KB)\n");
        printf("system32/     - System libraries\n");
    } else if (persona == DEMO_ARM64) {
        printf("vmlinuz       - ARM64 kernel (8MB)\n");
        printf("initrd.img    - Initial ramdisk (32MB)\n");
        printf("devicetree/   - Device tree binaries\n");
    } else if (persona == DEMO_CHEMOS) {
        printf("quantum.state - Quantum state data (1GB)\n");
        printf("elements/     - Chemical element configs (118 files)\n");
        printf("fusion.log    - Reactor operation log (500MB)\n");
        printf("coherence.dat - Quantum coherence data (2GB)\n");
    }
    printf("\n");
}

void show_bridge_info(demo_persona_t persona) {
    printf("\n🌉 Cross-Architecture Communication Bridges:\n");
    printf("=============================================\n");
    printf("Active bridges:\n");

    for (int i = 0; i < 6; i++) {
        if (personas[i].persona != persona) {
            printf("  %s ↔ %s: %s\n",
                   personas[0].name,  // Current persona name would be here
                   personas[i].name,
                   (rand() % 2) ? "✅ Connected" : "⚠️ Establishing");
        }
    }

    printf("\nBridge Statistics:\n");
    printf("  Data transferred: %d MB\n", rand() % 1000 + 100);
    printf("  Messages queued: %d\n", rand() % 50);
    printf("  Average latency: %d µs\n\n", rand() % 100 + 10);
}

void show_quantum_operations() {
    printf("\n⚛️ ChemOS Quantum Operations:\n");
    printf("==============================\n");
    printf("Available quantum operations:\n");
    printf("  activate <element>   - Activate chemical element\n");
    printf("  deactivate <element> - Deactivate chemical element\n");
    printf("  measure <element>    - Measure quantum state\n");
    printf("  entangle <e1> <e2>   - Create quantum entanglement\n");
    printf("  coherence            - Check quantum coherence\n");
    printf("  reactor <id>         - Control fusion reactor\n\n");

    printf("Current quantum state:\n");
    printf("  Active elements: 10/118\n");
    printf("  Quantum coherence: 99.8%%\n");
    printf("  Entangled pairs: 3\n");
    printf("  Fusion reactors: 2/8 active\n");
    printf("  Energy production: 2,400 MW\n\n");
}

int main() {
    printf("🚀 TBOS v3.0 Universal Shell Interactive Demo\n");
    printf("===============================================\n");
    printf("Demonstrating shell commands across different personas\n\n");

    demo_persona_t current_persona = DEMO_UNIVERSAL;

    // Demo sequence
    printf("📱 Starting with Universal persona...\n");
    show_status(current_persona);

    printf("💡 Let's see what commands are available:\n");
    show_help(current_persona);

    printf("📁 Checking available files:\n");
    show_ls(current_persona);

    printf("🔄 Switching to ARM64 persona:\n");
    show_persona_switch(current_persona, DEMO_ARM64);
    current_persona = DEMO_ARM64;

    printf("📊 Checking status after persona switch:\n");
    show_status(current_persona);

    printf("📁 Files now appear differently in ARM64 persona:\n");
    show_ls(current_persona);

    printf("🌉 Let's check cross-architecture bridges:\n");
    show_bridge_info(current_persona);

    printf("🧪 Now switching to ChemOS for quantum operations:\n");
    show_persona_switch(current_persona, DEMO_CHEMOS);
    current_persona = DEMO_CHEMOS;

    printf("⚛️ ChemOS persona unlocks quantum capabilities:\n");
    show_quantum_operations();

    printf("📁 ChemOS shows quantum-related files:\n");
    show_ls(current_persona);

    printf("🎯 Finally, let's switch to Calculator persona:\n");
    show_persona_switch(current_persona, DEMO_CALCULATOR);
    current_persona = DEMO_CALCULATOR;

    printf("🧮 Calculator persona - minimal but efficient:\n");
    show_status(current_persona);
    show_ls(current_persona);

    printf("✨ Demo completed! The universal shell adapts to each persona,\n");
    printf("   showing relevant commands, files, and capabilities.\n");
    printf("   Commands like 'quantum' only appear in ChemOS persona,\n");
    printf("   while file listings change based on the hardware context.\n\n");

    printf("🎉 TBOS v3.0 Universal Shell: One shell, eight personalities!\n");

    return 0;
}