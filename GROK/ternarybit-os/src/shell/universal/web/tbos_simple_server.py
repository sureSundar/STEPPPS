#!/usr/bin/env python3
"""
TernaryBit OS - Simple Web Shell Server (No Dependencies)
Uses only Python standard library.

Usage:
    cd src/shell/universal
    python3 web/tbos_simple_server.py

Then open: http://localhost:8888
"""

import http.server
import json
import os
import pty
import select
import socketserver
import subprocess
import sys
import signal
import threading
import time
from pathlib import Path
from urllib.parse import parse_qs

# HTML content embedded
HTML = '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TernaryBit OS - Web Shell</title>
    <style>
        :root {
            --bg: #0d1117;
            --bg2: #161b22;
            --term-bg: #0a0e14;
            --text: #c9d1d9;
            --green: #3fb950;
            --red: #f85149;
            --yellow: #d29922;
            --blue: #58a6ff;
            --purple: #bc8cff;
            --cyan: #39c5cf;
            --border: #30363d;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Consolas', 'Monaco', monospace;
            background: var(--bg);
            color: var(--text);
            height: 100vh;
            display: flex;
            flex-direction: column;
        }
        header {
            background: var(--bg2);
            border-bottom: 1px solid var(--border);
            padding: 12px 20px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        .logo {
            display: flex;
            align-items: center;
            gap: 12px;
        }
        .logo h1 {
            font-size: 18px;
            color: var(--cyan);
        }
        .status {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 13px;
        }
        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
        }
        .status-dot.on { background: var(--green); }
        .status-dot.off { background: var(--red); }
        .terminal-wrap {
            flex: 1;
            display: flex;
            flex-direction: column;
            padding: 10px;
            overflow: hidden;
        }
        .toolbar {
            display: flex;
            gap: 10px;
            padding: 10px 0;
        }
        .toolbar button {
            background: var(--bg2);
            border: 1px solid var(--border);
            color: var(--text);
            padding: 8px 16px;
            border-radius: 6px;
            cursor: pointer;
        }
        .toolbar button:hover {
            background: var(--border);
        }
        #terminal {
            flex: 1;
            background: var(--term-bg);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 15px;
            overflow-y: auto;
            font-size: 14px;
            line-height: 1.5;
            white-space: pre-wrap;
        }
        #input-line {
            display: flex;
            gap: 10px;
            padding-top: 10px;
        }
        #input-line input {
            flex: 1;
            background: var(--term-bg);
            border: 1px solid var(--border);
            color: var(--text);
            padding: 10px 15px;
            border-radius: 6px;
            font-family: inherit;
            font-size: 14px;
        }
        #input-line input:focus {
            outline: none;
            border-color: var(--cyan);
        }
        #input-line button {
            background: var(--cyan);
            border: none;
            color: var(--bg);
            padding: 10px 20px;
            border-radius: 6px;
            cursor: pointer;
            font-weight: bold;
        }
        footer {
            background: var(--bg2);
            border-top: 1px solid var(--border);
            padding: 8px 20px;
            font-size: 12px;
            color: #8b949e;
        }
        .c-red { color: var(--red); }
        .c-green { color: var(--green); }
        .c-yellow { color: var(--yellow); }
        .c-blue { color: var(--blue); }
        .c-purple { color: var(--purple); }
        .c-cyan { color: var(--cyan); }
        .c-white { color: #ffffff; }
        .c-gray { color: #8b949e; }
        .c-bold { font-weight: bold; }
    </style>
</head>
<body>
    <header>
        <div class="logo">
            <span style="font-size:24px">🕉️</span>
            <h1>TernaryBit OS Web Shell</h1>
        </div>
        <div class="status">
            <span class="status-dot on" id="status-dot"></span>
            <span id="status-text">Ready</span>
        </div>
    </header>

    <div class="terminal-wrap">
        <div class="toolbar">
            <button onclick="clearTerm()">Clear</button>
            <button onclick="sendCmd('help')">Help</button>
            <button onclick="sendCmd('karma')">Karma</button>
            <button onclick="sendCmd('pwd')">PWD</button>
        </div>
        <div id="terminal"></div>
        <div id="input-line">
            <input type="text" id="cmd" placeholder="Enter command..." autocomplete="off">
            <button onclick="sendInput()">Send</button>
        </div>
    </div>

    <footer>
        TBOS Web Shell | Press Enter to send command | Use toolbar for quick access
    </footer>

    <script>
        const terminal = document.getElementById('terminal');
        const cmdInput = document.getElementById('cmd');
        const history = [];
        let historyIdx = 0;

        function escapeHtml(text) {
            return text.replace(/&/g, '&amp;')
                       .replace(/</g, '&lt;')
                       .replace(/>/g, '&gt;');
        }

        function parseAnsi(text) {
            // Handle actual escape characters (0x1B) in various formats
            return text
                // Remove cursor positioning sequences like ESC[K, ESC[20C, ESC[row;colH
                .replace(/\x1b\[\d*[ABCDJKH]/g, '')
                .replace(/\x1b\[\d+;\d+H/g, '')
                .replace(/\x1b\[\d*[Cm]/g, '')
                .replace(/\x1b\[K/g, '')
                // Convert \r\n and \r to \n for display
                .replace(/\r\n/g, '\n')
                .replace(/\r/g, '')
                // Bold colors
                .replace(/\x1b\[1;31m/g, '<span class="c-red c-bold">')
                .replace(/\x1b\[1;32m/g, '<span class="c-green c-bold">')
                .replace(/\x1b\[1;33m/g, '<span class="c-yellow c-bold">')
                .replace(/\x1b\[1m/g, '<span class="c-bold">')
                // Standard colors
                .replace(/\x1b\[31m/g, '<span class="c-red">')
                .replace(/\x1b\[32m/g, '<span class="c-green">')
                .replace(/\x1b\[33m/g, '<span class="c-yellow">')
                .replace(/\x1b\[34m/g, '<span class="c-blue">')
                .replace(/\x1b\[35m/g, '<span class="c-purple">')
                .replace(/\x1b\[36m/g, '<span class="c-cyan">')
                .replace(/\x1b\[37m/g, '<span class="c-white">')
                // Bright colors
                .replace(/\x1b\[90m/g, '<span class="c-gray">')
                .replace(/\x1b\[91m/g, '<span class="c-red">')
                .replace(/\x1b\[92m/g, '<span class="c-green">')
                .replace(/\x1b\[93m/g, '<span class="c-yellow">')
                .replace(/\x1b\[94m/g, '<span class="c-blue">')
                .replace(/\x1b\[95m/g, '<span class="c-purple">')
                .replace(/\x1b\[96m/g, '<span class="c-cyan">')
                // Reset
                .replace(/\x1b\[0m/g, '</span>')
                // Clean up any remaining escape sequences
                .replace(/\x1b\[[0-9;]*[a-zA-Z]/g, '');
        }

        function appendOutput(text) {
            const escaped = escapeHtml(text);
            const colored = parseAnsi(escaped);
            terminal.innerHTML += colored;
            terminal.scrollTop = terminal.scrollHeight;
        }

        function clearTerm() {
            terminal.innerHTML = '';
        }

        function sendCmd(cmd) {
            cmdInput.value = cmd;
            sendInput();
        }

        async function sendInput() {
            const cmd = cmdInput.value.trim();
            if (!cmd) return;

            history.push(cmd);
            historyIdx = history.length;

            appendOutput('> ' + cmd + '\n');
            cmdInput.value = '';

            try {
                const response = await fetch('/cmd', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ cmd: cmd })
                });
                const data = await response.json();
                if (data.output) {
                    appendOutput(data.output);
                }
                if (data.error) {
                    appendOutput('[Error] ' + data.error + '\n');
                }
            } catch (err) {
                appendOutput('[Error] ' + err.message + '\n');
            }
        }

        cmdInput.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                sendInput();
            } else if (e.key === 'ArrowUp') {
                if (historyIdx > 0) {
                    historyIdx--;
                    cmdInput.value = history[historyIdx];
                }
                e.preventDefault();
            } else if (e.key === 'ArrowDown') {
                if (historyIdx < history.length - 1) {
                    historyIdx++;
                    cmdInput.value = history[historyIdx];
                } else {
                    historyIdx = history.length;
                    cmdInput.value = '';
                }
                e.preventDefault();
            }
        });

        // Initial load
        fetch('/cmd', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ cmd: '' })
        }).then(r => r.json()).then(data => {
            if (data.output) appendOutput(data.output);
        });

        cmdInput.focus();
    </script>
</body>
</html>
'''


class TBOSShell:
    """Simple shell wrapper using pexpect-like PTY."""

    def __init__(self, shell_path):
        self.shell_path = shell_path
        self.master_fd = None
        self.process = None
        self.lock = threading.Lock()

    def start(self):
        """Start the shell."""
        master, slave = pty.openpty()
        self.master_fd = master

        # Set terminal size
        import fcntl
        import struct
        import termios
        size = struct.pack('HHHH', 40, 120, 0, 0)
        fcntl.ioctl(slave, termios.TIOCSWINSZ, size)

        self.process = subprocess.Popen(
            [self.shell_path],
            stdin=slave,
            stdout=slave,
            stderr=slave,
            preexec_fn=os.setsid,
            env={**os.environ, 'TERM': 'xterm-256color'}
        )
        os.close(slave)

        # Wait for initial output
        time.sleep(0.5)
        return self.read()

    def write(self, data):
        """Send command to shell."""
        with self.lock:
            if self.master_fd:
                os.write(self.master_fd, (data + '\n').encode())

    def read(self, timeout=0.5):
        """Read output from shell."""
        output = []
        end_time = time.time() + timeout

        while time.time() < end_time:
            r, _, _ = select.select([self.master_fd], [], [], 0.1)
            if r:
                try:
                    data = os.read(self.master_fd, 4096)
                    if data:
                        output.append(data.decode('utf-8', errors='replace'))
                except:
                    break
            elif output:
                break

        return ''.join(output)

    def execute(self, cmd):
        """Execute command and return output."""
        if not cmd.strip():
            return self.read(0.2)

        self.write(cmd)
        time.sleep(0.1)
        return self.read(1.0)

    def stop(self):
        """Stop the shell."""
        if self.process:
            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
            except:
                pass
        if self.master_fd:
            try:
                os.close(self.master_fd)
            except:
                pass


# Global shell instance
shell = None


class RequestHandler(http.server.SimpleHTTPRequestHandler):
    """Handle HTTP requests."""

    def do_GET(self):
        if self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(HTML.encode())
        else:
            self.send_error(404)

    def do_POST(self):
        global shell

        if self.path == '/cmd':
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode('utf-8')

            try:
                data = json.loads(body)
                cmd = data.get('cmd', '')

                output = shell.execute(cmd)

                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({
                    'output': output,
                    'status': 'ok'
                }).encode())

            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({
                    'error': str(e)
                }).encode())
        else:
            self.send_error(404)

    def log_message(self, format, *args):
        pass  # Suppress logging


def main():
    global shell

    # Find shell
    script_dir = Path(__file__).parent.parent
    shell_path = script_dir / 'tbos_shell'

    print("=" * 60)
    print("  TernaryBit OS - Web Shell Server")
    print("=" * 60)
    print()

    if not shell_path.exists():
        print(f"[ERROR] Shell not found: {shell_path}")
        print(f"        Run 'make' in {script_dir} first")
        sys.exit(1)

    print(f"[OK] Shell: {shell_path}")

    # Start shell
    shell = TBOSShell(str(shell_path))
    print("[OK] Starting shell...")
    initial_output = shell.start()
    print("[OK] Shell started")

    # Start server
    PORT = 8888
    with socketserver.TCPServer(("", PORT), RequestHandler) as httpd:
        print(f"[OK] Server: http://localhost:{PORT}")
        print()
        print("Open http://localhost:8888 in your browser")
        print("Press Ctrl+C to stop")
        print()

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down...")
            shell.stop()


if __name__ == '__main__':
    main()
