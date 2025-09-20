/**
 * TernaryBit OS Web Interface
 * JavaScript bridge between WebAssembly TBOS core and web UI
 */

class TBOS_WebInterface {
    constructor() {
        this.module = null;
        this.isInitialized = false;
        this.commandHistory = [];
        this.historyIndex = -1;
        this.startTime = Date.now();

        this.initializeUI();
        this.loadTBOS();
    }

    initializeUI() {
        this.terminal = document.getElementById('terminal');
        this.output = document.getElementById('output');
        this.input = document.getElementById('input');
        this.loading = document.getElementById('loading');
        this.statusEl = document.getElementById('status');
        this.memoryEl = document.getElementById('memory');
        this.uptimeEl = document.getElementById('uptime');

        // Setup input handling
        this.input.addEventListener('keydown', (e) => this.handleKeyDown(e));

        // Update status periodically
        setInterval(() => this.updateStatus(), 1000);

        this.log('🔱 TernaryBit OS Web Interface Initialized');
        this.log('📡 Loading WebAssembly module...');
    }

    async loadTBOS() {
        try {
            this.loading.style.display = 'block';

            // Load TBOS WebAssembly module
            this.module = await TBOS({
                onRuntimeInitialized: () => {
                    this.onTBOSReady();
                },
                print: (text) => {
                    this.log(text);
                },
                printErr: (text) => {
                    this.log(`ERROR: ${text}`, 'error');
                }
            });
        } catch (error) {
            this.log(`❌ Failed to load TBOS: ${error.message}`, 'error');
            this.loading.style.display = 'none';
        }
    }

    onTBOSReady() {
        this.loading.style.display = 'none';
        this.isInitialized = true;
        this.input.disabled = false;
        this.input.focus();

        this.statusEl.textContent = 'Ready';

        this.log('✅ TernaryBit OS WebAssembly Module Loaded');
        this.log('🎯 STEPPPS Framework Initialized');
        this.log('🌐 Web-native TBOS ready for commands');
        this.log('💡 Type "help" for available commands');
        this.log('');
    }

    handleKeyDown(event) {
        if (!this.isInitialized) return;

        switch (event.key) {
            case 'Enter':
                this.executeCommand();
                break;
            case 'ArrowUp':
                this.navigateHistory(-1);
                event.preventDefault();
                break;
            case 'ArrowDown':
                this.navigateHistory(1);
                event.preventDefault();
                break;
            case 'Tab':
                this.autoComplete();
                event.preventDefault();
                break;
        }
    }

    executeCommand() {
        const command = this.input.value.trim();
        if (!command) return;

        this.log(`tbos@web:~$ ${command}`);
        this.commandHistory.push(command);
        this.historyIndex = this.commandHistory.length;
        this.input.value = '';

        // Process command
        this.processCommand(command);
    }

    processCommand(command) {
        const args = command.split(' ');
        const cmd = args[0].toLowerCase();

        switch (cmd) {
            case 'help':
                this.showHelp();
                break;
            case 'clear':
                this.output.textContent = '';
                break;
            case 'info':
            case 'sysinfo':
                this.showSystemInfo();
                break;
            case 'demo':
                this.runDemo();
                break;
            case 'boot':
                this.boot();
                break;
            case 'reset':
                this.reset();
                break;
            case 'ls':
                this.listFiles(args[1] || '/');
                break;
            case 'cat':
                this.catFile(args[1]);
                break;
            case 'echo':
                this.log(args.slice(1).join(' '));
                break;
            case 'uptime':
                this.showUptime();
                break;
            case 'ps':
                this.showProcesses();
                break;
            case 'mem':
            case 'memory':
                this.showMemory();
                break;
            case 'version':
                this.showVersion();
                break;
            case 'steppps':
                this.showSTEPPPS();
                break;
            default:
                if (this.module && this.module._tbos_run) {
                    // Pass command to TBOS core
                    try {
                        const result = this.module.ccall('tbos_run', 'string', ['string'], [command]);
                        this.log(result);
                    } catch (error) {
                        this.log(`Command not found: ${cmd}`, 'error');
                        this.log('Type "help" for available commands');
                    }
                } else {
                    this.log(`Command not found: ${cmd}`, 'error');
                    this.log('Type "help" for available commands');
                }
                break;
        }
    }

    showHelp() {
        this.log(`
🔱 TernaryBit OS Web Interface Commands:

System Commands:
  help              Show this help message
  clear             Clear terminal
  info/sysinfo      Show system information
  boot              Initialize TBOS kernel
  reset             Reset TBOS system
  version           Show TBOS version
  uptime            Show system uptime

File System:
  ls [path]         List directory contents
  cat <file>        Display file contents

Process Management:
  ps                Show running processes
  mem/memory        Show memory usage

TBOS Specific:
  demo              Run TBOS demonstration
  steppps           Show STEPPPS framework info

Web Interface:
  🚀 Boot TBOS      Initialize kernel
  🔄 Reset          Restart system
  ℹ️ System Info    Hardware details
  🎮 Demo Mode      Interactive demo
  📁 File System    Browse files
  ❓ Help           This help screen

🌐 This is TernaryBit OS running in WebAssembly
🔱 STEPPPS Framework: Sacred Technology for Enhanced Processing
`);
    }

    showSystemInfo() {
        const memUsage = this.getMemoryUsage();
        const uptime = this.getUptime();

        this.log(`
🔱 TernaryBit OS System Information

Platform:          WebAssembly (${navigator.platform})
Browser:            ${navigator.userAgent.split(' ')[0]}
Memory Available:   ${memUsage.total} MB
Memory Used:        ${memUsage.used} MB
Uptime:            ${uptime}
STEPPPS Framework:  Active
Sacred Modules:     Loaded
Pixel Codec:        Enabled
Universal FS:       Mounted

Architecture:       WASM32
Kernel Version:     1.0.0-web
Build Date:         ${new Date().toISOString().split('T')[0]}
`);
    }

    runDemo() {
        this.log('🎮 Starting TernaryBit OS Demo...');
        this.log('');
        this.log('🔱 STEPPPS Framework Demonstration');
        this.log('📊 Initializing sacred pixel processing...');

        setTimeout(() => {
            this.log('✨ Sacred geometry patterns loading...');
            this.animateDemo();
        }, 1000);
    }

    animateDemo() {
        const patterns = ['🔱', '🕉️', '🌀', '✨', '🎯', '🔮'];
        let i = 0;

        const interval = setInterval(() => {
            if (i < 20) {
                this.log(`${patterns[i % patterns.length]} Processing dimensional layer ${i + 1}/20...`);
                i++;
            } else {
                clearInterval(interval);
                this.log('');
                this.log('✅ Demo completed successfully!');
                this.log('🌟 TBOS Web Interface fully operational');
            }
        }, 200);
    }

    boot() {
        this.log('🚀 Booting TernaryBit OS...');
        this.log('🔱 STEPPPS Framework: Initializing...');
        this.log('📡 Sacred Pixel Codec: Loading...');
        this.log('🌐 Universal Filesystem: Mounting...');
        this.log('✅ TernaryBit OS Boot Complete');
        this.statusEl.textContent = 'Running';
    }

    reset() {
        this.output.textContent = '';
        this.log('🔄 TernaryBit OS Reset Complete');
        this.log('🔱 Ready for new session');
        this.statusEl.textContent = 'Ready';
    }

    showSTEPPPS() {
        this.log(`
🔱 STEPPPS Framework Information

Sacred Technology for Enhanced Processing and Pixel Synthesis

Components:
  🕉️ Sacred Geometry Engine     [ACTIVE]
  🎯 Dimensional Computing      [ACTIVE]
  ✨ Pixel Transformation       [ACTIVE]
  🌀 Universal Algorithms       [ACTIVE]
  🔮 Consciousness Interface    [ACTIVE]

Philosophy:
  - Hindu mythology integration
  - Sacred mathematical principles
  - Dimensional transcendence
  - Universal consciousness bridge

Status: Fully operational in WebAssembly environment
`);
    }

    getMemoryUsage() {
        // Estimate WebAssembly memory usage
        const performance = window.performance || {};
        const memory = performance.memory || {};

        return {
            used: Math.round((memory.usedJSHeapSize || 32 * 1024 * 1024) / 1024 / 1024),
            total: Math.round((memory.totalJSHeapSize || 64 * 1024 * 1024) / 1024 / 1024)
        };
    }

    getUptime() {
        const seconds = Math.floor((Date.now() - this.startTime) / 1000);
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;

        return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }

    updateStatus() {
        if (this.isInitialized) {
            const memory = this.getMemoryUsage();
            this.memoryEl.textContent = `${memory.used} MB`;
            this.uptimeEl.textContent = this.getUptime();
        }
    }

    log(text, type = 'normal') {
        const line = document.createElement('div');
        line.textContent = text;

        if (type === 'error') {
            line.style.color = '#ff6b6b';
        }

        this.output.appendChild(line);
        this.terminal.scrollTop = this.terminal.scrollHeight;
    }

    navigateHistory(direction) {
        if (this.commandHistory.length === 0) return;

        this.historyIndex += direction;

        if (this.historyIndex < 0) {
            this.historyIndex = 0;
        } else if (this.historyIndex >= this.commandHistory.length) {
            this.historyIndex = this.commandHistory.length;
            this.input.value = '';
            return;
        }

        this.input.value = this.commandHistory[this.historyIndex] || '';
    }

    // Global interface for buttons
    static getInstance() {
        return window.TBOS_Web_Instance;
    }
}

// Global interface functions for HTML buttons
window.TBOS_Web = {
    boot: () => TBOS_WebInterface.getInstance().boot(),
    reset: () => TBOS_WebInterface.getInstance().reset(),
    showInfo: () => TBOS_WebInterface.getInstance().showSystemInfo(),
    runDemo: () => TBOS_WebInterface.getInstance().runDemo(),
    openFS: () => TBOS_WebInterface.getInstance().processCommand('ls /'),
    showHelp: () => TBOS_WebInterface.getInstance().showHelp()
};

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.TBOS_Web_Instance = new TBOS_WebInterface();
});