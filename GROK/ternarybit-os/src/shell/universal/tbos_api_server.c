/**
 * @file tbos_api_server.c
 * @brief TBOS HTTP API Server with embedded Web UI
 *
 * Single-port server providing both web UI and JSON API.
 * Usage: ./tbos_api_server [port]    (default: 8888)
 * Open the URL printed at startup (includes a required auth token) in
 * your browser.
 *
 * Security model: this process executes arbitrary shell commands on
 * behalf of any request that presents the startup token, so the token
 * is the entire access boundary. It binds to 127.0.0.1 by default -
 * set TBOS_API_BIND_ALL=1 to opt in to listening on all interfaces,
 * and only do that on a trusted network.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#include "tbos_cmd_api.h"
#include "tbos_universal_shell.h"

#define DEFAULT_PORT 8888
#define BUFFER_SIZE 65536
#define TOKEN_LEN 32 /* hex chars */

static volatile int running = 1;
static int server_fd = -1;
static char g_auth_token[TOKEN_LEN + 1];

static void generate_auth_token(char* out, size_t out_len) {
    static const char hex[] = "0123456789abcdef";
    unsigned char raw[TOKEN_LEN / 2];
    int fd = open("/dev/urandom", O_RDONLY);
    size_t got = 0;
    if (fd >= 0) {
        ssize_t n = read(fd, raw, sizeof(raw));
        if (n > 0) got = (size_t)n;
        close(fd);
    }
    if (got < sizeof(raw)) {
        /* Fallback if /dev/urandom is unavailable: not cryptographically
         * strong, but still unguessable enough to not be printed in any
         * log/history, unlike a fixed default token would be. */
        srand((unsigned)(getpid() ^ time(NULL)));
        for (size_t i = got; i < sizeof(raw); i++) raw[i] = (unsigned char)rand();
    }
    for (size_t i = 0; i < sizeof(raw) && i * 2 + 1 < out_len; i++) {
        out[i * 2] = hex[raw[i] >> 4];
        out[i * 2 + 1] = hex[raw[i] & 0x0F];
    }
    out[TOKEN_LEN] = '\0';
}

/* Extract ?token=... from a request path/query string, if present. */
static void extract_query_token(const char* path, char* out, size_t out_len) {
    out[0] = '\0';
    const char* q = strchr(path, '?');
    if (!q) return;
    const char* t = strstr(q, "token=");
    if (!t) return;
    t += 6;
    size_t i = 0;
    while (t[i] && t[i] != '&' && i + 1 < out_len) {
        out[i] = t[i];
        i++;
    }
    out[i] = '\0';
}

/* Extract a bearer/custom token header from raw request headers. */
static void extract_header_token(const char* raw, char* out, size_t out_len) {
    out[0] = '\0';
    const char* h = strstr(raw, "X-TBOS-Token:");
    size_t skip = 13;
    if (!h) {
        h = strstr(raw, "Authorization: Bearer ");
        skip = 22;
    }
    if (!h) return;
    const char* t = h + skip;
    while (*t == ' ') t++;
    size_t i = 0;
    while (t[i] && t[i] != '\r' && t[i] != '\n' && i + 1 < out_len) {
        out[i] = t[i];
        i++;
    }
    out[i] = '\0';
}

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
    if (server_fd >= 0) close(server_fd);
}

typedef struct {
    char method[16];
    char path[256];       /* routing path only, query string stripped */
    char query_token[TOKEN_LEN + 1];
    char header_token[TOKEN_LEN + 1];
    char* body;
    size_t body_len;
} http_request_t;

/* Same-origin only: the bundled web UI is served from this same process,
 * so it never needs cross-origin access. No CORS headers are emitted -
 * a browser will refuse cross-origin fetches by default without them,
 * which is what we want given /api/cmd executes arbitrary commands. */
static void send_response(int fd, int status, const char* status_text,
                         const char* content_type, const char* body) {
    char header[512];
    size_t body_len = body ? strlen(body) : 0;
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
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
    char raw_target[256] = {0};
    sscanf(buffer, "%15s %255s", req->method, raw_target);

    extract_query_token(raw_target, req->query_token, sizeof(req->query_token));
    extract_header_token(buffer, req->header_token, sizeof(req->header_token));

    /* Strip the query string so routing comparisons (strcmp on req->path)
     * work regardless of a ?token=... suffix. */
    char* qmark = strchr(raw_target, '?');
    if (qmark) *qmark = '\0';
    snprintf(req->path, sizeof(req->path), "%s", raw_target);

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

static int request_is_authorized(const http_request_t* req) {
    if (req->query_token[0] && strcmp(req->query_token, g_auth_token) == 0) return 1;
    if (req->header_token[0] && strcmp(req->header_token, g_auth_token) == 0) return 1;
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
"const tk=new URLSearchParams(location.search).get('token')||'';"
"const x=await fetch('/api/cmd',{method:'POST',headers:{'Content-Type':'application/json','X-TBOS-Token':tk},body:JSON.stringify({cmd:c})});"
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

/* Endpoints that read the filesystem, run commands, or reveal the
 * command surface all require the startup token. The marketing index
 * page and the plain health check don't. */
static int path_requires_auth(const char* path) {
    return strcmp(path, "/shell") == 0 ||
           strcmp(path, "/api/cmd") == 0 ||
           strcmp(path, "/api/commands") == 0;
}

static void handle_request(int fd, http_request_t* req) {
    if (strcmp(req->method, "OPTIONS") == 0) {
        send_response(fd, 200, "OK", "text/plain", "");
        return;
    }
    if (path_requires_auth(req->path) && !request_is_authorized(req)) {
        send_json(fd, 401, "{\"status\":\"error\",\"error\":\"Missing or invalid token\"}");
        return;
    }
    if (strcmp(req->path, "/") == 0 || strcmp(req->path, "/index.html") == 0) {
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

    generate_auth_token(g_auth_token, sizeof(g_auth_token));

    universal_shell_init();
    tbos_cmd_init();

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Bind to localhost only unless the operator explicitly opts in to
     * listening on all interfaces. This process executes arbitrary
     * shell commands for anyone who presents the token, so the default
     * must not be reachable from the network. */
    int bind_all = 0;
    const char* bind_all_env = getenv("TBOS_API_BIND_ALL");
    if (bind_all_env && strcmp(bind_all_env, "1") == 0) bind_all = 1;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = bind_all ? INADDR_ANY : inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    printf("\n");
    printf("  TBOS Web Shell Server\n");
    printf("  Listening on: %s:%d%s\n", bind_all ? "0.0.0.0" : "127.0.0.1", port,
           bind_all ? "  (TBOS_API_BIND_ALL=1 - reachable from the network!)" : "");
    printf("  Open: http://localhost:%d/shell?token=%s\n", port, g_auth_token);
    printf("  (This token is required for /shell, /api/cmd and /api/commands.\n");
    printf("   It is not logged anywhere else - copy it from here.)\n");
    printf("  Press Ctrl+C to stop\n\n");
    fflush(stdout); /* stdout is fully buffered when not a TTY (e.g.
                      * redirected to a log file); without this the token
                      * above would sit unflushed while accept() blocks. */

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
