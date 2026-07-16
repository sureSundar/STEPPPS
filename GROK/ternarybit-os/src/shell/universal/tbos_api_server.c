/**
 * @file tbos_api_server.c
 * @brief TBOS HTTP API Server with embedded Web UI
 *
 * Single-port server providing both web UI and JSON API.
 * Usage: ./tbos_api_server [port]    (default: 8888)
 * Open http://localhost:8888 in browser
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <ctype.h>

#include "tbos_cmd_api.h"
#include "tbos_universal_shell.h"

#define DEFAULT_PORT 8888
#define BUFFER_SIZE 65536

static volatile int running = 1;
static int server_fd = -1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
    if (server_fd >= 0) close(server_fd);
}

typedef struct {
    char method[16];
    char path[256];
    char* body;
    size_t body_len;
} http_request_t;

static void send_response(int fd, int status, const char* status_text,
                         const char* content_type, const char* body) {
    char header[512];
    size_t body_len = body ? strlen(body) : 0;
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n\r\n",
        status, status_text, content_type, body_len);
    write(fd, header, strlen(header));
    if (body && body_len > 0) write(fd, body, body_len);
}

static void send_json(int fd, int status, const char* json) {
    send_response(fd, status, status == 200 ? "OK" : "Error", "application/json", json);
}

static int parse_request(const char* buffer, size_t len, http_request_t* req) {
    memset(req, 0, sizeof(*req));
    sscanf(buffer, "%15s %255s", req->method, req->path);
    const char* body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        size_t header_len = body_start - buffer;
        if (len > header_len) {
            req->body_len = len - header_len;
            req->body = (char*)body_start;
        }
    }
    return 0;
}

static char* json_get_string(const char* json, const char* key) {
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char* pos = strstr(json, search);
    if (!pos) return NULL;
    pos = strchr(pos + strlen(search), ':');
    if (!pos) return NULL;
    while (*pos && (isspace((unsigned char)*pos) || *pos == ':')) pos++;
    if (*pos != '"') return NULL;
    pos++;
    const char* end = pos;
    while (*end && *end != '"') {
        if (*end == '\\' && *(end + 1)) end++;
        end++;
    }
    size_t slen = end - pos;
    char* result = (char*)malloc(slen + 1);
    memcpy(result, pos, slen);
    result[slen] = '\0';
    return result;
}

static void handle_cmd(int fd, http_request_t* req) {
    if (!req->body || req->body_len == 0) {
        send_json(fd, 400, "{\"status\":\"error\",\"error\":\"Missing body\"}");
        return;
    }
    char* cmd = json_get_string(req->body, "cmd");
    if (!cmd) {
        send_json(fd, 400, "{\"status\":\"error\",\"error\":\"Missing cmd\"}");
        return;
    }
    tbos_api_context_t* ctx = tbos_api_context_create(TBOS_FMT_JSON);
    tbos_api_result_t* result = tbos_cmd_execute(cmd, ctx);
    free(cmd);
    char* json = tbos_result_render(result, TBOS_FMT_JSON);
    send_json(fd, 200, json);
    free(json);
    tbos_result_free(result);
    tbos_context_free(ctx);
}

static void handle_commands(int fd) {
    const universal_command_t* commands[256];
    int count = universal_get_commands(commands, 256);
    char* json = (char*)malloc(32768);
    size_t len = sprintf(json, "{\"status\":\"ok\",\"commands\":[");
    for (int i = 0; i < count; i++) {
        if (i > 0) len += sprintf(json + len, ",");
        len += sprintf(json + len, "{\"name\":\"%s\"}", commands[i]->name);
    }
    sprintf(json + len, "],\"count\":%d}", count);
    send_json(fd, 200, json);
    free(json);
}

/* Landing Page */
static const char* INDEX_HTML =
"<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
"<title>TernaryBit OS</title><style>"
":root{--bg:#0d1117;--bg2:#161b22;--text:#c9d1d9;--cyan:#39c5cf;--green:#3fb950;--border:#30363d}"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:'SF Pro','Segoe UI',sans-serif;background:var(--bg);color:var(--text);min-height:100vh}"
".hero{text-align:center;padding:60px 20px;background:linear-gradient(135deg,#161b22 0%,#0d1117 100%)}"
".hero h1{font-size:48px;margin-bottom:10px;background:linear-gradient(90deg,var(--cyan),var(--green));-webkit-background-clip:text;-webkit-text-fill-color:transparent}"
".hero p{font-size:18px;color:#8b949e;margin-bottom:30px}"
".icon{font-size:80px;margin-bottom:20px}"
".cards{display:flex;flex-wrap:wrap;gap:20px;justify-content:center;padding:40px 20px;max-width:1000px;margin:0 auto}"
".card{background:var(--bg2);border:1px solid var(--border);border-radius:12px;padding:24px;width:280px;text-decoration:none;color:var(--text);transition:all .2s}"
".card:hover{border-color:var(--cyan);transform:translateY(-4px);box-shadow:0 8px 30px rgba(0,0,0,.3)}"
".card h3{color:var(--cyan);margin-bottom:10px;font-size:20px}"
".card p{color:#8b949e;font-size:14px;line-height:1.5}"
".card .emoji{font-size:36px;margin-bottom:12px}"
".stats{background:var(--bg2);border-top:1px solid var(--border);padding:30px 20px}"
".stats-inner{max-width:800px;margin:0 auto;display:flex;justify-content:space-around;text-align:center}"
".stat h4{font-size:32px;color:var(--cyan)}"
".stat p{color:#8b949e;font-size:14px}"
"footer{text-align:center;padding:20px;color:#8b949e;font-size:12px}"
"</style></head><body>"
"<div class=\"hero\">"
"<div class=\"icon\">&#x1F549;</div>"
"<h1>TernaryBit OS</h1>"
"<p>A consciousness-aware operating system for mindful computing</p>"
"</div>"
"<div class=\"cards\">"
"<a class=\"card\" href=\"/shell\"><div class=\"emoji\">&#x1F4BB;</div><h3>Web Shell</h3><p>Interactive command-line interface in your browser. Execute commands, manage files, and explore the system.</p></a>"
"<a class=\"card\" href=\"/api/commands\"><div class=\"emoji\">&#x1F4DC;</div><h3>API Reference</h3><p>View all available commands and their descriptions. JSON format for easy integration.</p></a>"
"<a class=\"card\" href=\"/health\"><div class=\"emoji\">&#x2764;</div><h3>Health Check</h3><p>Monitor server status and connectivity. Useful for automation and monitoring.</p></a>"
"</div>"
"<div class=\"stats\"><div class=\"stats-inner\">"
"<div class=\"stat\"><h4>107</h4><p>Commands</p></div>"
"<div class=\"stat\"><h4>10</h4><p>Weeks Complete</p></div>"
"<div class=\"stat\"><h4>3</h4><p>Filesystems</p></div>"
"<div class=\"stat\"><h4>&#x221E;</h4><p>Karma Potential</p></div>"
"</div></div>"
"<footer>TernaryBit OS | Truth is One, Paths are Many</footer>"
"</body></html>";

/* Shell Web UI */
static const char* SHELL_HTML =
"<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
"<title>TBOS Web Shell</title><style>"
":root{--bg:#0d1117;--bg2:#161b22;--term:#0a0e14;--text:#c9d1d9;--green:#3fb950;"
"--red:#f85149;--cyan:#39c5cf;--border:#30363d}"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:Consolas,monospace;background:var(--bg);color:var(--text);height:100vh;display:flex;flex-direction:column}"
"header{background:var(--bg2);border-bottom:1px solid var(--border);padding:12px 20px;display:flex;justify-content:space-between}"
".logo{display:flex;align-items:center;gap:12px}"
".logo h1{font-size:18px;color:var(--cyan)}"
"main{flex:1;display:flex;flex-direction:column;padding:16px;overflow:hidden}"
".bar{display:flex;gap:8px;padding-bottom:12px;flex-wrap:wrap}"
".bar button{background:var(--bg2);border:1px solid var(--border);color:var(--text);padding:8px 14px;border-radius:6px;cursor:pointer}"
".bar button:hover{border-color:var(--cyan)}"
"#term{flex:1;background:var(--term);border:1px solid var(--border);border-radius:8px;padding:16px;overflow-y:auto;white-space:pre-wrap}"
".inp{display:flex;gap:10px;padding-top:12px}"
"#cmd{flex:1;background:var(--term);border:1px solid var(--border);color:var(--text);padding:12px;border-radius:6px;font-family:inherit}"
"#cmd:focus{border-color:var(--cyan);outline:none}"
"#run{background:var(--cyan);border:none;color:var(--bg);padding:12px 24px;border-radius:6px;cursor:pointer;font-weight:bold}"
".p{color:var(--cyan)}.c{color:var(--green)}.e{color:var(--red)}"
"footer{background:var(--bg2);border-top:1px solid var(--border);padding:8px 20px;font-size:12px;color:#8b949e}"
"</style></head><body>"
"<header><div class=\"logo\"><span style=\"font-size:24px\">&#x1F549;</span><h1>TernaryBit OS Web Shell</h1></div>"
"<div id=\"st\" style=\"color:var(--green)\">Ready</div></header>"
"<main><div class=\"bar\">"
"<button onclick=\"r('help')\">Help</button>"
"<button onclick=\"r('ls')\">ls</button>"
"<button onclick=\"r('pwd')\">pwd</button>"
"<button onclick=\"r('ps')\">ps</button>"
"<button onclick=\"r('uname -a')\">uname</button>"
"<button onclick=\"term.innerHTML=''\">Clear</button>"
"</div><div id=\"term\"></div>"
"<div class=\"inp\"><input id=\"cmd\" placeholder=\"Enter command...\" autofocus>"
"<button id=\"run\" onclick=\"s()\">Run</button></div></main>"
"<footer>TBOS Shell | Enter to run | Up/Down for history</footer>"
"<script>"
"const term=document.getElementById('term'),cmd=document.getElementById('cmd'),st=document.getElementById('st');"
"const h=[];let hi=-1;"
"function e(t){return t?t.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;'):''}"
"function o(x){term.innerHTML+=x;term.scrollTop=term.scrollHeight}"
"function r(c){cmd.value=c;s()}"
"async function s(){"
"const c=cmd.value.trim();if(!c)return;"
"if(h[h.length-1]!==c)h.push(c);hi=h.length;"
"o('<span class=\"p\">tbos:/&gt; </span><span class=\"c\">'+e(c)+'</span>\\n');"
"cmd.value='';st.textContent='Running...';st.style.color='var(--cyan)';"
"try{"
"const x=await fetch('/api/cmd',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({cmd:c})});"
"const d=await x.json();"
"if(d.status==='ok'&&d.output)o(e(d.output));"
"else if(d.error)o('<span class=\"e\">Error: '+e(d.error)+'</span>\\n');"
"st.textContent='Ready';st.style.color='var(--green)';"
"}catch(err){o('<span class=\"e\">Error: '+e(err.message)+'</span>\\n');st.textContent='Error';st.style.color='var(--red)';}"
"cmd.focus()}"
"cmd.onkeydown=function(ev){"
"if(ev.key==='Enter')s();"
"else if(ev.key==='ArrowUp'){ev.preventDefault();if(hi>0){hi--;cmd.value=h[hi]}}"
"else if(ev.key==='ArrowDown'){ev.preventDefault();if(hi<h.length-1){hi++;cmd.value=h[hi]}else{hi=h.length;cmd.value=''}}"
"else if(ev.key==='l'&&ev.ctrlKey){ev.preventDefault();term.innerHTML=''}"
"};"
"o('<span style=\"color:var(--green)\">Connected to TBOS</span>\\n<span style=\"color:var(--cyan)\">Type help for commands</span>\\n\\n');"
"</script></body></html>";

static void handle_index(int fd) {
    send_response(fd, 200, "OK", "text/html; charset=utf-8", INDEX_HTML);
}

static void handle_shell(int fd) {
    send_response(fd, 200, "OK", "text/html; charset=utf-8", SHELL_HTML);
}

static void handle_request(int fd, http_request_t* req) {
    if (strcmp(req->method, "OPTIONS") == 0) {
        send_response(fd, 200, "OK", "text/plain", "");
    } else if (strcmp(req->path, "/") == 0 || strcmp(req->path, "/index.html") == 0) {
        handle_index(fd);
    } else if (strcmp(req->path, "/shell") == 0) {
        handle_shell(fd);
    } else if (strcmp(req->path, "/api/cmd") == 0 && strcmp(req->method, "POST") == 0) {
        handle_cmd(fd, req);
    } else if (strcmp(req->path, "/api/commands") == 0) {
        handle_commands(fd);
    } else if (strcmp(req->path, "/health") == 0) {
        send_json(fd, 200, "{\"status\":\"ok\"}");
    } else {
        send_json(fd, 404, "{\"status\":\"error\",\"error\":\"Not found\"}");
    }
}

int main(int argc, char* argv[]) {
    int port = argc > 1 ? atoi(argv[1]) : DEFAULT_PORT;
    if (port <= 0 || port > 65535) port = DEFAULT_PORT;

    universal_shell_init();
    tbos_cmd_init();

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    printf("\n");
    printf("  TBOS Web Shell Server\n");
    printf("  Open: http://localhost:%d\n", port);
    printf("  Press Ctrl+C to stop\n\n");

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) continue;

        char buffer[BUFFER_SIZE];
        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            http_request_t req;
            if (parse_request(buffer, n, &req) == 0) {
                handle_request(client_fd, &req);
            }
        }
        close(client_fd);
    }

    close(server_fd);
    tbos_cmd_shutdown();
    universal_shell_shutdown();
    return 0;
}
