#!/usr/bin/env python3
"""
TernaryBit OS - Web Shell Server
Provides a web-based interface to the TBOS shell via WebSocket.

Usage:
    cd src/shell/universal
    python3 web/tbos_web_server.py

Then open: http://localhost:8888
"""

import asyncio
import json
import os
import pty
import select
import subprocess
import sys
import signal
from pathlib import Path

# Try to import websockets, provide helpful message if not available
try:
    import websockets
    from websockets.server import serve
except ImportError:
    print("Error: websockets library not found.")
    print("Install with: pip3 install websockets")
    sys.exit(1)

# Try to import aiohttp for HTTP serving
try:
    from aiohttp import web
except ImportError:
    print("Error: aiohttp library not found.")
    print("Install with: pip3 install aiohttp")
    sys.exit(1)


class TBOSWebShell:
    """Manages a PTY connection to the TBOS shell."""

    def __init__(self, shell_path):
        self.shell_path = shell_path
        self.master_fd = None
        self.slave_fd = None
        self.process = None
        self.running = False

    def start(self):
        """Start the shell in a pseudo-terminal."""
        # Create pseudo-terminal
        self.master_fd, self.slave_fd = pty.openpty()

        # Start shell process
        self.process = subprocess.Popen(
            [self.shell_path],
            stdin=self.slave_fd,
            stdout=self.slave_fd,
            stderr=self.slave_fd,
            preexec_fn=os.setsid
        )

        # Close slave in parent
        os.close(self.slave_fd)
        self.slave_fd = None

        self.running = True
        return True

    def stop(self):
        """Stop the shell."""
        self.running = False
        if self.process:
            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
            except:
                pass
            self.process = None
        if self.master_fd:
            os.close(self.master_fd)
            self.master_fd = None

    def write(self, data):
        """Write data to the shell."""
        if self.master_fd and self.running:
            os.write(self.master_fd, data.encode('utf-8'))

    def read(self, timeout=0.1):
        """Read available data from the shell."""
        if not self.master_fd or not self.running:
            return None

        r, _, _ = select.select([self.master_fd], [], [], timeout)
        if r:
            try:
                data = os.read(self.master_fd, 4096)
                return data.decode('utf-8', errors='replace')
            except OSError:
                return None
        return ""


# Store active shell sessions
sessions = {}


async def websocket_handler(websocket, path):
    """Handle WebSocket connections."""
    session_id = id(websocket)

    # Find the shell binary
    script_dir = Path(__file__).parent.parent
    shell_path = script_dir / "tbos_shell"

    if not shell_path.exists():
        await websocket.send(json.dumps({
            "type": "error",
            "data": f"Shell not found at {shell_path}. Run 'make' first."
        }))
        return

    # Create shell session
    shell = TBOSWebShell(str(shell_path))
    sessions[session_id] = shell

    try:
        shell.start()
        await websocket.send(json.dumps({
            "type": "connected",
            "data": "Connected to TBOS shell"
        }))

        # Task to read from shell and send to client
        async def read_shell():
            while shell.running:
                data = shell.read(0.05)
                if data:
                    await websocket.send(json.dumps({
                        "type": "output",
                        "data": data
                    }))
                await asyncio.sleep(0.01)

        # Start reading task
        read_task = asyncio.create_task(read_shell())

        # Handle incoming messages
        async for message in websocket:
            try:
                msg = json.loads(message)
                if msg.get("type") == "input":
                    shell.write(msg.get("data", ""))
                elif msg.get("type") == "resize":
                    # Handle terminal resize if needed
                    pass
            except json.JSONDecodeError:
                pass

    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        shell.stop()
        del sessions[session_id]


# HTML for the web interface
HTML_CONTENT = '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TernaryBit OS - Web Shell</title>
    <style>
        :root {
            --bg-primary: #0d1117;
            --bg-secondary: #161b22;
            --bg-terminal: #0a0e14;
            --text-primary: #c9d1d9;
            --text-secondary: #8b949e;
            --accent-green: #3fb950;
            --accent-red: #f85149;
            --accent-yellow: #d29922;
            --accent-blue: #58a6ff;
            --accent-purple: #bc8cff;
            --accent-cyan: #39c5cf;
            --border-color: #30363d;
        }

        * { box-sizing: border-box; margin: 0; padding: 0; }

        body {
            font-family: 'SF Mono', 'Fira Code', 'Consolas', monospace;
            background: var(--bg-primary);
            color: var(--text-primary);
            height: 100vh;
            display: flex;
            flex-direction: column;
        }

        header {
            background: var(--bg-secondary);
            border-bottom: 1px solid var(--border-color);
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

        .logo-icon {
            font-size: 24px;
        }

        .logo h1 {
            font-size: 18px;
            font-weight: 600;
            color: var(--accent-cyan);
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
            background: var(--text-secondary);
        }

        .status-dot.connected { background: var(--accent-green); }
        .status-dot.disconnected { background: var(--accent-red); }
        .status-dot.connecting {
            background: var(--accent-yellow);
            animation: pulse 1s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.4; }
        }

        .terminal-container {
            flex: 1;
            display: flex;
            flex-direction: column;
            padding: 10px;
            overflow: hidden;
        }

        #terminal {
            flex: 1;
            background: var(--bg-terminal);
            border: 1px solid var(--border-color);
            border-radius: 8px;
            padding: 15px;
            overflow-y: auto;
            font-size: 14px;
            line-height: 1.5;
            white-space: pre-wrap;
            word-wrap: break-word;
        }

        #terminal .line { margin: 0; }

        /* ANSI color classes */
        .ansi-black { color: #484f58; }
        .ansi-red { color: var(--accent-red); }
        .ansi-green { color: var(--accent-green); }
        .ansi-yellow { color: var(--accent-yellow); }
        .ansi-blue { color: var(--accent-blue); }
        .ansi-magenta { color: var(--accent-purple); }
        .ansi-cyan { color: var(--accent-cyan); }
        .ansi-white { color: var(--text-primary); }
        .ansi-bright-black { color: #6e7681; }
        .ansi-bright-red { color: #ff7b72; }
        .ansi-bright-green { color: #7ee787; }
        .ansi-bright-yellow { color: #ffd33d; }
        .ansi-bright-blue { color: #79c0ff; }
        .ansi-bright-magenta { color: #d2a8ff; }
        .ansi-bright-cyan { color: #56d4dd; }
        .ansi-bright-white { color: #ffffff; }
        .ansi-bold { font-weight: bold; }

        .toolbar {
            display: flex;
            gap: 10px;
            padding: 10px 0;
        }

        .toolbar button {
            background: var(--bg-secondary);
            border: 1px solid var(--border-color);
            color: var(--text-primary);
            padding: 8px 16px;
            border-radius: 6px;
            cursor: pointer;
            font-size: 13px;
            transition: all 0.2s;
        }

        .toolbar button:hover {
            background: var(--border-color);
            border-color: var(--text-secondary);
        }

        footer {
            background: var(--bg-secondary);
            border-top: 1px solid var(--border-color);
            padding: 8px 20px;
            font-size: 12px;
            color: var(--text-secondary);
            display: flex;
            justify-content: space-between;
        }

        .keyboard-hint {
            display: flex;
            gap: 15px;
        }

        .keyboard-hint kbd {
            background: var(--bg-primary);
            border: 1px solid var(--border-color);
            border-radius: 4px;
            padding: 2px 6px;
            font-size: 11px;
        }
    </style>
</head>
<body>
    <header>
        <div class="logo">
            <span class="logo-icon">🕉️</span>
            <h1>TernaryBit OS</h1>
        </div>
        <div class="status">
            <span class="status-dot" id="status-dot"></span>
            <span id="status-text">Connecting...</span>
        </div>
    </header>

    <div class="terminal-container">
        <div class="toolbar">
            <button onclick="clearTerminal()">Clear</button>
            <button onclick="reconnect()">Reconnect</button>
            <button onclick="sendCtrlC()">Send Ctrl+C</button>
        </div>
        <div id="terminal" tabindex="0"></div>
    </div>

    <footer>
        <span>TBOS Web Shell v1.0</span>
        <div class="keyboard-hint">
            <span><kbd>↑</kbd><kbd>↓</kbd> History</span>
            <span><kbd>Tab</kbd> Complete</span>
            <span><kbd>Ctrl+L</kbd> Clear</span>
        </div>
    </footer>

    <script>
        const terminal = document.getElementById('terminal');
        const statusDot = document.getElementById('status-dot');
        const statusText = document.getElementById('status-text');

        let ws = null;
        let inputBuffer = '';

        // ANSI escape code parser
        function parseAnsi(text) {
            const container = document.createDocumentFragment();
            const ansiRegex = /\\x1b\\[([0-9;]*)m/g;

            // Simple ANSI parser
            let result = text;
            const colorMap = {
                '0': 'ansi-reset',
                '1': 'ansi-bold',
                '30': 'ansi-black', '31': 'ansi-red', '32': 'ansi-green',
                '33': 'ansi-yellow', '34': 'ansi-blue', '35': 'ansi-magenta',
                '36': 'ansi-cyan', '37': 'ansi-white',
                '90': 'ansi-bright-black', '91': 'ansi-bright-red',
                '92': 'ansi-bright-green', '93': 'ansi-bright-yellow',
                '94': 'ansi-bright-blue', '95': 'ansi-bright-magenta',
                '96': 'ansi-bright-cyan', '97': 'ansi-bright-white'
            };

            // Replace ANSI codes with spans
            let currentClasses = [];
            result = result.replace(/\\x1b\\[([0-9;]*)m/g, (match, codes) => {
                if (codes === '0' || codes === '') {
                    currentClasses = [];
                    return '</span>';
                }
                const classList = codes.split(';').map(c => colorMap[c] || '').filter(Boolean);
                currentClasses = classList;
                return `<span class="${classList.join(' ')}">`;
            });

            return result;
        }

        function appendOutput(text) {
            // Handle ANSI escape sequences
            const parsed = text
                .replace(/\\x1b\\[([0-9;]*)m/g, (m, codes) => {
                    if (codes === '0' || codes === '') return '</span>';
                    const classes = [];
                    codes.split(';').forEach(c => {
                        if (c === '1') classes.push('ansi-bold');
                        else if (c === '31') classes.push('ansi-red');
                        else if (c === '32') classes.push('ansi-green');
                        else if (c === '33') classes.push('ansi-yellow');
                        else if (c === '34') classes.push('ansi-blue');
                        else if (c === '35') classes.push('ansi-magenta');
                        else if (c === '36') classes.push('ansi-cyan');
                        else if (c === '91') classes.push('ansi-bright-red');
                        else if (c === '92') classes.push('ansi-bright-green');
                        else if (c === '93') classes.push('ansi-bright-yellow');
                    });
                    return classes.length ? `<span class="${classes.join(' ')}">` : '';
                })
                .replace(/\\x1b\\[K/g, '')  // Clear to EOL
                .replace(/\\x1b\\[2J/g, '') // Clear screen (handle separately)
                .replace(/\\x1b\\[H/g, '')  // Home
                .replace(/\\r/g, '');       // Remove CR

            terminal.innerHTML += parsed;
            terminal.scrollTop = terminal.scrollHeight;
        }

        function clearTerminal() {
            terminal.innerHTML = '';
        }

        function sendCtrlC() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ type: 'input', data: '\\x03' }));
            }
        }

        function setStatus(state, text) {
            statusDot.className = 'status-dot ' + state;
            statusText.textContent = text;
        }

        function connect() {
            setStatus('connecting', 'Connecting...');

            const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
            ws = new WebSocket(`${protocol}//${location.hostname}:8889`);

            ws.onopen = () => {
                setStatus('connected', 'Connected');
                terminal.focus();
            };

            ws.onmessage = (event) => {
                try {
                    const msg = JSON.parse(event.data);
                    if (msg.type === 'output') {
                        appendOutput(msg.data);
                    } else if (msg.type === 'error') {
                        appendOutput(`\\n[ERROR] ${msg.data}\\n`);
                    }
                } catch (e) {
                    console.error('Parse error:', e);
                }
            };

            ws.onclose = () => {
                setStatus('disconnected', 'Disconnected');
            };

            ws.onerror = (error) => {
                setStatus('disconnected', 'Connection error');
                console.error('WebSocket error:', error);
            };
        }

        function reconnect() {
            if (ws) {
                ws.close();
            }
            clearTerminal();
            connect();
        }

        // Keyboard input handling
        terminal.addEventListener('keydown', (e) => {
            if (!ws || ws.readyState !== WebSocket.OPEN) return;

            let data = '';

            if (e.key === 'Enter') {
                data = '\\n';
            } else if (e.key === 'Backspace') {
                data = '\\x7f';
            } else if (e.key === 'Tab') {
                e.preventDefault();
                data = '\\t';
            } else if (e.key === 'ArrowUp') {
                e.preventDefault();
                data = '\\x1b[A';
            } else if (e.key === 'ArrowDown') {
                e.preventDefault();
                data = '\\x1b[B';
            } else if (e.key === 'ArrowLeft') {
                e.preventDefault();
                data = '\\x1b[D';
            } else if (e.key === 'ArrowRight') {
                e.preventDefault();
                data = '\\x1b[C';
            } else if (e.key === 'Home') {
                e.preventDefault();
                data = '\\x1b[H';
            } else if (e.key === 'End') {
                e.preventDefault();
                data = '\\x1b[F';
            } else if (e.key === 'Delete') {
                e.preventDefault();
                data = '\\x1b[3~';
            } else if (e.ctrlKey) {
                e.preventDefault();
                const code = e.key.toLowerCase().charCodeAt(0) - 96;
                if (code > 0 && code < 27) {
                    data = String.fromCharCode(code);
                }
            } else if (e.key.length === 1 && !e.ctrlKey && !e.altKey && !e.metaKey) {
                data = e.key;
            }

            if (data) {
                ws.send(JSON.stringify({ type: 'input', data: data }));
            }
        });

        // Focus terminal on click
        terminal.addEventListener('click', () => terminal.focus());

        // Connect on load
        connect();
    </script>
</body>
</html>
'''


async def http_handler(request):
    """Serve the HTML page."""
    return web.Response(text=HTML_CONTENT, content_type='text/html')


async def main():
    """Start both HTTP and WebSocket servers."""
    # Get paths
    script_dir = Path(__file__).parent
    shell_dir = script_dir.parent

    print("=" * 60)
    print("  TernaryBit OS - Web Shell Server")
    print("=" * 60)
    print()

    # Check if shell exists
    shell_path = shell_dir / "tbos_shell"
    if not shell_path.exists():
        print(f"[ERROR] Shell not found: {shell_path}")
        print(f"        Run 'make' in {shell_dir} first")
        return

    print(f"[OK] Shell found: {shell_path}")

    # Start HTTP server
    app = web.Application()
    app.router.add_get('/', http_handler)

    runner = web.AppRunner(app)
    await runner.setup()
    http_site = web.TCPSite(runner, 'localhost', 8888)
    await http_site.start()
    print(f"[OK] HTTP server: http://localhost:8888")

    # Start WebSocket server
    ws_server = await serve(websocket_handler, 'localhost', 8889)
    print(f"[OK] WebSocket server: ws://localhost:8889")

    print()
    print("Open http://localhost:8888 in your browser")
    print("Press Ctrl+C to stop")
    print()

    # Keep running
    try:
        await asyncio.Future()
    except KeyboardInterrupt:
        print("\nShutting down...")
        ws_server.close()
        await ws_server.wait_closed()
        await runner.cleanup()


if __name__ == '__main__':
    asyncio.run(main())
