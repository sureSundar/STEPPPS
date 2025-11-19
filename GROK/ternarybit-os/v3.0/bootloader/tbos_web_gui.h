/*
 * TBOS v3.0 Web-Based GUI Framework
 * HTML5/CSS3/JavaScript GUI with browser rendering
 * Intuitive, innovative, and expandable for humans and machines
 */

#ifndef TBOS_WEB_GUI_H
#define TBOS_WEB_GUI_H

#include "tbos_shell_architecture.h"
#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// WEB GUI ARCHITECTURE
//=============================================================================

typedef enum {
    WEB_GUI_PROTOCOL_HTTP = 0,      // Standard HTTP
    WEB_GUI_PROTOCOL_HTTPS = 1,     // Secure HTTPS
    WEB_GUI_PROTOCOL_WS = 2,        // WebSocket
    WEB_GUI_PROTOCOL_WSS = 3,       // Secure WebSocket
    WEB_GUI_PROTOCOL_WEBRTC = 4,    // WebRTC for P2P
    WEB_GUI_PROTOCOL_REST = 5,      // RESTful API
    WEB_GUI_PROTOCOL_GRAPHQL = 6,   // GraphQL API
    WEB_GUI_PROTOCOL_COUNT = 7
} web_gui_protocol_t;

typedef enum {
    WEB_GUI_RENDERER_CHROME = 0,    // Chromium-based browsers
    WEB_GUI_RENDERER_FIREFOX = 1,   // Gecko-based browsers
    WEB_GUI_RENDERER_SAFARI = 2,    // WebKit-based browsers
    WEB_GUI_RENDERER_EDGE = 3,      // Edge browser
    WEB_GUI_RENDERER_EMBEDDED = 4,  // Embedded web view
    WEB_GUI_RENDERER_VR = 5,        // WebXR for VR/AR
    WEB_GUI_RENDERER_MOBILE = 6,    // Mobile optimized
    WEB_GUI_RENDERER_AI = 7,        // AI/Machine interface
    WEB_GUI_RENDERER_COUNT = 8
} web_gui_renderer_t;

typedef enum {
    WEB_GUI_FRAMEWORK_VANILLA = 0,  // Pure HTML/CSS/JS
    WEB_GUI_FRAMEWORK_REACT = 1,    // React.js
    WEB_GUI_FRAMEWORK_VUE = 2,      // Vue.js
    WEB_GUI_FRAMEWORK_ANGULAR = 3,  // Angular
    WEB_GUI_FRAMEWORK_SVELTE = 4,   // Svelte
    WEB_GUI_FRAMEWORK_THREE = 5,    // Three.js for 3D
    WEB_GUI_FRAMEWORK_D3 = 6,       // D3.js for data viz
    WEB_GUI_FRAMEWORK_WEBGL = 7,    // WebGL for graphics
    WEB_GUI_FRAMEWORK_COUNT = 8
} web_gui_framework_t;

//=============================================================================
// WEB GUI COMPONENT SYSTEM
//=============================================================================

typedef struct {
    char tag[32];               // HTML tag (div, button, canvas, etc.)
    char id[64];                // Unique element ID
    char class_name[128];       // CSS classes
    char inner_html[1024];      // HTML content
    char styles[512];           // Inline CSS styles
    char attributes[256];       // HTML attributes

    // Event handlers (JavaScript function names)
    char on_click[64];
    char on_change[64];
    char on_hover[64];
    char on_focus[64];

    // Persona-specific properties
    uint8_t persona_mask;       // Which personas support this component
    bool quantum_enabled;       // Quantum visualization support
    bool vr_compatible;         // WebXR compatibility
    bool ai_accessible;         // Machine-readable interface

    // Dynamic properties
    bool visible;
    bool interactive;
    float opacity;

    // Layout properties
    int32_t x, y, width, height;
    char position[16];          // CSS position (relative, absolute, fixed)
    char display[16];           // CSS display (block, inline, flex, grid)

} web_gui_component_t;

typedef struct {
    char name[64];              // Page name/title
    char url_path[128];         // URL path (/desktop, /quantum, etc.)
    char html_template[256];    // HTML template file
    char css_file[256];         // CSS stylesheet
    char js_file[256];          // JavaScript file

    // Page metadata
    char description[256];
    char keywords[128];
    uint8_t target_persona;

    // Page components
    web_gui_component_t components[64];
    uint32_t component_count;

    // Page configuration
    bool requires_auth;
    bool real_time_updates;
    bool offline_capable;
    web_gui_protocol_t protocol;

} web_gui_page_t;

//=============================================================================
// WEB GUI SERVER ARCHITECTURE
//=============================================================================

#define MAX_WEB_GUI_SESSIONS 256
#define MAX_WEB_GUI_PAGES 128
#define MAX_WEB_GUI_COMPONENTS 2048
#define MAX_WEB_GUI_THEMES 32

typedef struct {
    uint32_t session_id;
    char user_agent[256];       // Browser/client information
    char ip_address[64];        // Client IP
    web_gui_renderer_t renderer;

    // Session state
    bool authenticated;
    char username[64];
    uint8_t current_persona;
    uint32_t privilege_level;

    // Browser capabilities
    bool webgl_support;
    bool webxr_support;
    bool webrtc_support;
    bool service_worker_support;
    bool local_storage_support;

    // Session metrics
    uint64_t session_start_time;
    uint32_t pages_visited;
    uint32_t api_calls_made;

} web_gui_session_t;

typedef struct {
    // Server configuration
    bool server_running;
    uint16_t http_port;         // Default: 8080
    uint16_t https_port;        // Default: 8443
    uint16_t websocket_port;    // Default: 8081

    // TLS/SSL configuration
    bool ssl_enabled;
    char cert_file[256];
    char key_file[256];

    // Web server state
    uint8_t current_persona;
    web_gui_framework_t framework;

    // Session management
    web_gui_session_t sessions[MAX_WEB_GUI_SESSIONS];
    uint32_t session_count;

    // Page registry
    web_gui_page_t pages[MAX_WEB_GUI_PAGES];
    uint32_t page_count;

    // Static resources
    char web_root[256];         // Web root directory
    char static_path[256];      // Static files path
    char templates_path[256];   // HTML templates path

    // API configuration
    bool rest_api_enabled;
    bool graphql_enabled;
    bool websocket_enabled;
    char api_base_path[64];     // /api/v1

    // Security configuration
    bool cors_enabled;
    char allowed_origins[512];
    bool csrf_protection;
    char security_headers[512];

    // Performance metrics
    uint64_t requests_served;
    uint64_t bytes_transferred;
    uint32_t concurrent_connections;
    uint64_t average_response_time_ms;

} web_gui_server_t;

//=============================================================================
// PERSONA-SPECIFIC WEB THEMES
//=============================================================================

typedef struct {
    char theme_name[64];
    uint8_t target_persona;

    // CSS properties
    char primary_color[16];     // #rrggbb
    char secondary_color[16];
    char accent_color[16];
    char background_color[16];
    char text_color[16];

    // Typography
    char font_family[64];
    char font_size[16];
    char line_height[16];

    // Layout
    char layout_mode[16];       // grid, flexbox, absolute
    char breakpoints[128];      // Responsive breakpoints

    // Animations
    char transition_duration[16];
    char animation_easing[32];
    bool reduced_motion;

    // Special effects
    bool glassmorphism;         // Glass-like effects
    bool neumorphism;           // Soft, extruded shapes
    bool quantum_effects;       // Particle effects for ChemOS
    bool vr_optimizations;      // VR-friendly design

    // CSS file paths
    char css_file[256];
    char css_variables[512];    // CSS custom properties

} web_gui_theme_t;

//=============================================================================
// WEB GUI API INTERFACE
//=============================================================================

// Server management
int tbos_web_gui_init(web_gui_server_t* server);
int tbos_web_gui_start_server(web_gui_server_t* server);
int tbos_web_gui_stop_server(web_gui_server_t* server);
int tbos_web_gui_restart_server(web_gui_server_t* server);

// Page management
int tbos_web_gui_register_page(web_gui_server_t* server, web_gui_page_t* page);
int tbos_web_gui_generate_page_html(web_gui_page_t* page, char* html_output, size_t output_size);
int tbos_web_gui_update_page_component(web_gui_page_t* page, const char* component_id, web_gui_component_t* component);
web_gui_page_t* tbos_web_gui_get_page(web_gui_server_t* server, const char* url_path);

// Component management
int tbos_web_gui_create_component(web_gui_component_t* component, const char* tag, const char* id);
int tbos_web_gui_set_component_content(web_gui_component_t* component, const char* content);
int tbos_web_gui_set_component_style(web_gui_component_t* component, const char* property, const char* value);
int tbos_web_gui_add_component_event(web_gui_component_t* component, const char* event, const char* handler);

// Session management
uint32_t tbos_web_gui_create_session(web_gui_server_t* server, const char* user_agent, const char* ip);
int tbos_web_gui_destroy_session(web_gui_server_t* server, uint32_t session_id);
web_gui_session_t* tbos_web_gui_get_session(web_gui_server_t* server, uint32_t session_id);
int tbos_web_gui_authenticate_session(web_gui_server_t* server, uint32_t session_id, const char* credentials);

// Persona integration
int tbos_web_gui_switch_persona(web_gui_server_t* server, uint8_t new_persona);
int tbos_web_gui_apply_persona_theme(web_gui_server_t* server, uint8_t persona);
int tbos_web_gui_update_persona_pages(web_gui_server_t* server, uint8_t persona);

// Real-time communication
int tbos_web_gui_broadcast_message(web_gui_server_t* server, const char* message);
int tbos_web_gui_send_to_session(web_gui_server_t* server, uint32_t session_id, const char* message);
int tbos_web_gui_subscribe_events(web_gui_server_t* server, uint32_t session_id, const char* event_types);

// API endpoints
int tbos_web_gui_register_api_endpoint(web_gui_server_t* server, const char* path,
                                      int (*handler)(const char* request, char* response, size_t response_size));
int tbos_web_gui_handle_rest_request(web_gui_server_t* server, const char* method, const char* path,
                                    const char* body, char* response, size_t response_size);
int tbos_web_gui_handle_graphql_query(web_gui_server_t* server, const char* query,
                                     char* response, size_t response_size);

// Theme management
int tbos_web_gui_load_theme(web_gui_server_t* server, web_gui_theme_t* theme);
int tbos_web_gui_generate_css(web_gui_theme_t* theme, char* css_output, size_t output_size);
int tbos_web_gui_apply_theme(web_gui_server_t* server, const char* theme_name);

// Security and authentication
int tbos_web_gui_enable_ssl(web_gui_server_t* server, const char* cert_file, const char* key_file);
int tbos_web_gui_set_cors_policy(web_gui_server_t* server, const char* allowed_origins);
int tbos_web_gui_enable_csrf_protection(web_gui_server_t* server);
int tbos_web_gui_validate_session_token(web_gui_server_t* server, const char* token);

// WebXR/VR support
int tbos_web_gui_enable_webxr(web_gui_server_t* server);
int tbos_web_gui_create_vr_scene(web_gui_server_t* server, const char* scene_name);
int tbos_web_gui_add_vr_object(web_gui_server_t* server, const char* scene_name, const char* object_data);

// AI/Machine interface
int tbos_web_gui_enable_machine_api(web_gui_server_t* server);
int tbos_web_gui_generate_machine_schema(web_gui_server_t* server, char* schema_output, size_t output_size);
int tbos_web_gui_process_machine_request(web_gui_server_t* server, const char* machine_request,
                                        char* machine_response, size_t response_size);

// Utility functions
const char* tbos_web_gui_protocol_name(web_gui_protocol_t protocol);
const char* tbos_web_gui_renderer_name(web_gui_renderer_t renderer);
const char* tbos_web_gui_framework_name(web_gui_framework_t framework);
void tbos_web_gui_print_server_status(web_gui_server_t* server);
void tbos_web_gui_print_session_info(web_gui_session_t* session);

// Error codes
#define TBOS_WEB_GUI_SUCCESS                0
#define TBOS_WEB_GUI_ERROR_INVALID_PARAM   -1
#define TBOS_WEB_GUI_ERROR_SERVER_FAILED   -2
#define TBOS_WEB_GUI_ERROR_SESSION_INVALID -3
#define TBOS_WEB_GUI_ERROR_PAGE_NOT_FOUND  -4
#define TBOS_WEB_GUI_ERROR_AUTH_FAILED     -5
#define TBOS_WEB_GUI_ERROR_SSL_FAILED      -6
#define TBOS_WEB_GUI_ERROR_WEBSOCKET_FAILED -7
#define TBOS_WEB_GUI_ERROR_VR_NOT_SUPPORTED -8

//=============================================================================
// HTML TEMPLATES AND GENERATORS
//=============================================================================

// Generate complete HTML pages for each persona
int tbos_web_gui_generate_calculator_page(char* html_output, size_t output_size);
int tbos_web_gui_generate_embedded_dashboard(char* html_output, size_t output_size);
int tbos_web_gui_generate_desktop_environment(char* html_output, size_t output_size);
int tbos_web_gui_generate_mobile_interface(char* html_output, size_t output_size);
int tbos_web_gui_generate_quantum_workbench(char* html_output, size_t output_size);
int tbos_web_gui_generate_vr_environment(char* html_output, size_t output_size);

// Generate JavaScript for persona switching
int tbos_web_gui_generate_persona_switcher_js(char* js_output, size_t output_size);

// Generate CSS themes
int tbos_web_gui_generate_quantum_css(char* css_output, size_t output_size);
int tbos_web_gui_generate_mobile_css(char* css_output, size_t output_size);
int tbos_web_gui_generate_desktop_css(char* css_output, size_t output_size);

#endif // TBOS_WEB_GUI_H