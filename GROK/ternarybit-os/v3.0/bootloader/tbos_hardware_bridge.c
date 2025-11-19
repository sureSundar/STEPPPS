/*
 * TBOS v3.0 Hardware Bridge
 * Revolutionary interface between HTML5 Web GUI and TBOS Hardware Shell Layers
 * Enables real hardware control through browser interface
 */

#include "tbos_shell_architecture.h"
#include "tbos_shell_manager.h"
#include "tbos_command_router.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <json-c/json.h>

//=============================================================================
// HARDWARE BRIDGE ARCHITECTURE
//=============================================================================

typedef struct {
    int web_socket_fd;
    int client_socket_fd;
    bool is_connected;
    uint32_t session_id;

    // Hardware access
    tbos_shell_manager_t* shell_manager;
    command_router_t* command_router;
    plugin_manager_t* plugin_manager;

    // Real-time data streams
    pthread_t cpu_monitor_thread;
    pthread_t memory_monitor_thread;
    pthread_t hardware_events_thread;

    // Hardware state
    float cpu_usage;
    float memory_usage;
    float temperature;
    uint32_t network_rx_bytes;
    uint32_t network_tx_bytes;

} tbos_hardware_bridge_t;

static tbos_hardware_bridge_t g_bridge = {0};

//=============================================================================
// REAL HARDWARE MONITORING FUNCTIONS
//=============================================================================

float tbos_get_real_cpu_usage() {
    FILE* stat_file = fopen("/proc/stat", "r");
    if (!stat_file) return 0.0f;

    static long long prev_idle = 0, prev_total = 0;
    long long user, nice, system, idle, iowait, irq, softirq, steal;

    if (fscanf(stat_file, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) == 8) {

        long long total = user + nice + system + idle + iowait + irq + softirq + steal;
        long long diff_idle = idle - prev_idle;
        long long diff_total = total - prev_total;

        float usage = 100.0f * (1.0f - ((float)diff_idle / diff_total));

        prev_idle = idle;
        prev_total = total;

        fclose(stat_file);
        return usage;
    }

    fclose(stat_file);
    return 0.0f;
}

float tbos_get_real_memory_usage() {
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo) return 0.0f;

    long long total_mem = 0, free_mem = 0, available_mem = 0;
    char line[256];

    while (fgets(line, sizeof(line), meminfo)) {
        if (sscanf(line, "MemTotal: %lld kB", &total_mem) == 1) continue;
        if (sscanf(line, "MemFree: %lld kB", &free_mem) == 1) continue;
        if (sscanf(line, "MemAvailable: %lld kB", &available_mem) == 1) continue;
    }

    fclose(meminfo);

    if (total_mem > 0) {
        long long used_mem = total_mem - available_mem;
        return (float)used_mem / total_mem * 100.0f;
    }

    return 0.0f;
}

float tbos_get_cpu_temperature() {
    FILE* temp_file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!temp_file) return 0.0f;

    int temp_millicelsius;
    if (fscanf(temp_file, "%d", &temp_millicelsius) == 1) {
        fclose(temp_file);
        return temp_millicelsius / 1000.0f;
    }

    fclose(temp_file);
    return 0.0f;
}

void tbos_get_network_stats(uint32_t* rx_bytes, uint32_t* tx_bytes) {
    FILE* net_file = fopen("/proc/net/dev", "r");
    if (!net_file) {
        *rx_bytes = *tx_bytes = 0;
        return;
    }

    char line[512];
    *rx_bytes = *tx_bytes = 0;

    // Skip header lines
    fgets(line, sizeof(line), net_file);
    fgets(line, sizeof(line), net_file);

    while (fgets(line, sizeof(line), net_file)) {
        char interface[32];
        uint32_t rx, tx;
        uint32_t dummy;

        if (sscanf(line, "%31[^:]: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
                   interface, &rx, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy,
                   &tx, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) >= 10) {

            // Skip loopback interface
            if (strcmp(interface, "lo") != 0) {
                *rx_bytes += rx;
                *tx_bytes += tx;
            }
        }
    }

    fclose(net_file);
}

//=============================================================================
// HARDWARE MONITORING THREADS
//=============================================================================

void* tbos_cpu_monitor_thread(void* arg) {
    tbos_hardware_bridge_t* bridge = (tbos_hardware_bridge_t*)arg;

    while (bridge->is_connected) {
        bridge->cpu_usage = tbos_get_real_cpu_usage();
        bridge->temperature = tbos_get_cpu_temperature();

        // Send real-time update to web interface
        json_object* update = json_object_new_object();
        json_object* type = json_object_new_string("hardware_update");
        json_object* cpu = json_object_new_double(bridge->cpu_usage);
        json_object* temp = json_object_new_double(bridge->temperature);

        json_object_object_add(update, "type", type);
        json_object_object_add(update, "cpu_usage", cpu);
        json_object_object_add(update, "temperature", temp);

        const char* json_string = json_object_to_json_string(update);
        if (bridge->client_socket_fd > 0) {
            send(bridge->client_socket_fd, json_string, strlen(json_string), 0);
        }

        json_object_put(update);
        sleep(1);
    }

    return NULL;
}

void* tbos_memory_monitor_thread(void* arg) {
    tbos_hardware_bridge_t* bridge = (tbos_hardware_bridge_t*)arg;

    while (bridge->is_connected) {
        bridge->memory_usage = tbos_get_real_memory_usage();
        tbos_get_network_stats(&bridge->network_rx_bytes, &bridge->network_tx_bytes);

        // Send memory and network updates
        json_object* update = json_object_new_object();
        json_object* type = json_object_new_string("memory_update");
        json_object* memory = json_object_new_double(bridge->memory_usage);
        json_object* rx = json_object_new_int(bridge->network_rx_bytes);
        json_object* tx = json_object_new_int(bridge->network_tx_bytes);

        json_object_object_add(update, "type", type);
        json_object_object_add(update, "memory_usage", memory);
        json_object_object_add(update, "network_rx", rx);
        json_object_object_add(update, "network_tx", tx);

        const char* json_string = json_object_to_json_string(update);
        if (bridge->client_socket_fd > 0) {
            send(bridge->client_socket_fd, json_string, strlen(json_string), 0);
        }

        json_object_put(update);
        sleep(2);
    }

    return NULL;
}

//=============================================================================
// HARDWARE SHELL INTEGRATION
//=============================================================================

int tbos_execute_hardware_command(const char* command, char* response, size_t response_size) {
    // Route command through TBOS shell layers
    command_router_t* router = tbos_get_global_command_router();
    if (!router) {
        snprintf(response, response_size, "Error: Command router not available");
        return -1;
    }

    command_result_t result;
    int status = tbos_router_process_command(router, command, &result);

    if (status == TBOS_ROUTER_SUCCESS) {
        snprintf(response, response_size, "%s", result.output);
        return result.exit_code;
    } else {
        snprintf(response, response_size, "Error: %s", result.error);
        return status;
    }
}

int tbos_switch_hardware_persona(uint8_t persona) {
    // Actually switch TBOS hardware persona
    tbos_shell_manager_t* manager = g_bridge.shell_manager;
    if (!manager) return -1;

    printf("🔄 Hardware Persona Switch: %s → %s\n",
           tbos_get_persona_name(manager->current_persona),
           tbos_get_persona_name(persona));

    // Execute real persona switch
    int result = tbos_shell_switch_persona(manager, persona);

    // Update plugin manager
    plugin_manager_t* plugin_mgr = g_bridge.plugin_manager;
    if (plugin_mgr) {
        tbos_plugin_switch_persona(plugin_mgr, persona);
    }

    // Send confirmation to web interface
    json_object* update = json_object_new_object();
    json_object* type = json_object_new_string("persona_switched");
    json_object* new_persona = json_object_new_int(persona);
    json_object* persona_name = json_object_new_string(tbos_get_persona_name(persona));

    json_object_object_add(update, "type", type);
    json_object_object_add(update, "persona_id", new_persona);
    json_object_object_add(update, "persona_name", persona_name);

    const char* json_string = json_object_to_json_string(update);
    if (g_bridge.client_socket_fd > 0) {
        send(g_bridge.client_socket_fd, json_string, strlen(json_string), 0);
    }

    json_object_put(update);
    return result;
}

//=============================================================================
// WEB INTERFACE MESSAGE HANDLING
//=============================================================================

void tbos_handle_web_message(const char* message) {
    json_object* root = json_tokener_parse(message);
    if (!root) return;

    json_object* type_obj;
    if (!json_object_object_get_ex(root, "type", &type_obj)) {
        json_object_put(root);
        return;
    }

    const char* type = json_object_get_string(type_obj);

    if (strcmp(type, "execute_command") == 0) {
        json_object* command_obj;
        if (json_object_object_get_ex(root, "command", &command_obj)) {
            const char* command = json_object_get_string(command_obj);
            char response[4096];

            int result = tbos_execute_hardware_command(command, response, sizeof(response));

            // Send response back to web interface
            json_object* resp = json_object_new_object();
            json_object* resp_type = json_object_new_string("command_response");
            json_object* resp_result = json_object_new_int(result);
            json_object* resp_output = json_object_new_string(response);

            json_object_object_add(resp, "type", resp_type);
            json_object_object_add(resp, "result", resp_result);
            json_object_object_add(resp, "output", resp_output);

            const char* json_string = json_object_to_json_string(resp);
            send(g_bridge.client_socket_fd, json_string, strlen(json_string), 0);

            json_object_put(resp);
        }
    }
    else if (strcmp(type, "switch_persona") == 0) {
        json_object* persona_obj;
        if (json_object_object_get_ex(root, "persona", &persona_obj)) {
            int persona = json_object_get_int(persona_obj);
            tbos_switch_hardware_persona(persona);
        }
    }
    else if (strcmp(type, "get_hardware_info") == 0) {
        // Send comprehensive hardware information
        json_object* info = json_object_new_object();
        json_object* info_type = json_object_new_string("hardware_info");

        // CPU information
        json_object* cpu_info = json_object_new_object();
        json_object_object_add(cpu_info, "usage", json_object_new_double(g_bridge.cpu_usage));
        json_object_object_add(cpu_info, "temperature", json_object_new_double(g_bridge.temperature));
        json_object_object_add(cpu_info, "cores", json_object_new_int(sysconf(_SC_NPROCESSORS_ONLN)));

        // Memory information
        json_object* mem_info = json_object_new_object();
        json_object_object_add(mem_info, "usage_percent", json_object_new_double(g_bridge.memory_usage));

        // Network information
        json_object* net_info = json_object_new_object();
        json_object_object_add(net_info, "rx_bytes", json_object_new_int(g_bridge.network_rx_bytes));
        json_object_object_add(net_info, "tx_bytes", json_object_new_int(g_bridge.network_tx_bytes));

        json_object_object_add(info, "type", info_type);
        json_object_object_add(info, "cpu", cpu_info);
        json_object_object_add(info, "memory", mem_info);
        json_object_object_add(info, "network", net_info);

        const char* json_string = json_object_to_json_string(info);
        send(g_bridge.client_socket_fd, json_string, strlen(json_string), 0);

        json_object_put(info);
    }

    json_object_put(root);
}

//=============================================================================
// WEBSOCKET SERVER
//=============================================================================

void* tbos_websocket_server_thread(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return NULL;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        return NULL;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9001);  // WebSocket port

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return NULL;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return NULL;
    }

    printf("🌐 TBOS Hardware Bridge WebSocket Server listening on port 9001\n");

    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("🔗 Web interface connected to hardware bridge\n");

        g_bridge.client_socket_fd = client_fd;
        g_bridge.is_connected = true;

        // Start monitoring threads
        pthread_create(&g_bridge.cpu_monitor_thread, NULL, tbos_cpu_monitor_thread, &g_bridge);
        pthread_create(&g_bridge.memory_monitor_thread, NULL, tbos_memory_monitor_thread, &g_bridge);

        // Handle incoming messages
        char buffer[4096];
        int bytes_read;

        while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            tbos_handle_web_message(buffer);
        }

        g_bridge.is_connected = false;
        close(client_fd);

        printf("🔌 Web interface disconnected from hardware bridge\n");
    }

    return NULL;
}

//=============================================================================
// HARDWARE BRIDGE INITIALIZATION
//=============================================================================

int tbos_hardware_bridge_init() {
    printf("🔧 Initializing TBOS Hardware Bridge...\n");

    // Initialize shell manager
    g_bridge.shell_manager = malloc(sizeof(tbos_shell_manager_t));
    if (tbos_shell_manager_init(g_bridge.shell_manager) != 0) {
        printf("❌ Failed to initialize shell manager\n");
        return -1;
    }

    // Initialize command router
    g_bridge.command_router = tbos_get_global_command_router();
    if (!g_bridge.command_router) {
        if (tbos_initialize_global_command_router() != 0) {
            printf("❌ Failed to initialize command router\n");
            return -1;
        }
        g_bridge.command_router = tbos_get_global_command_router();
    }

    // Initialize plugin manager
    g_bridge.plugin_manager = tbos_get_global_plugin_manager();
    if (!g_bridge.plugin_manager) {
        if (tbos_initialize_global_plugin_system() != 0) {
            printf("❌ Failed to initialize plugin system\n");
            return -1;
        }
        g_bridge.plugin_manager = tbos_get_global_plugin_manager();
    }

    // Start WebSocket server
    pthread_t websocket_thread;
    if (pthread_create(&websocket_thread, NULL, tbos_websocket_server_thread, NULL) != 0) {
        printf("❌ Failed to start WebSocket server\n");
        return -1;
    }

    printf("✅ TBOS Hardware Bridge initialized successfully\n");
    printf("🌐 Web interface can now control real hardware through port 9001\n");

    return 0;
}

//=============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
//=============================================================================

#ifdef HARDWARE_BRIDGE_STANDALONE
int main() {
    printf("🚀 TBOS v3.0 Hardware Bridge - Standalone Mode\n");
    printf("===============================================\n");

    if (tbos_hardware_bridge_init() != 0) {
        printf("❌ Failed to initialize hardware bridge\n");
        return 1;
    }

    printf("🎯 Hardware bridge running - Web interface connected to real TBOS layers\n");
    printf("🌐 Open http://localhost:9000 and experience real hardware control!\n");

    // Keep running
    while (1) {
        sleep(1);
    }

    return 0;
}
#endif