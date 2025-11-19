/*
 * TBOS v3.0 Hardware Interface Bridge (JavaScript)
 * Revolutionary web-to-hardware communication layer
 * Enables HTML5 to directly control TBOS hardware layers
 */

class TBOSHardwareBridge {
    constructor() {
        this.websocket = null;
        this.sessionId = null;
        this.currentPersona = 'desktop';
        this.isConnected = false;
        this.hardwareStats = {};
        this.messageQueue = [];
        this.callbacks = {};

        this.init();
    }

    async init() {
        console.log('🌉 Initializing TBOS Hardware Bridge...');
        await this.connectToHardware();
        this.setupEventHandlers();
    }

    async connectToHardware() {
        try {
            // Connect to TBOS Web-Hardware Bridge
            this.websocket = new WebSocket('ws://localhost:9001');

            this.websocket.onopen = (event) => {
                this.isConnected = true;
                console.log('✅ Connected to TBOS Hardware Bridge');
                this.showNotification('🔗 Connected to TBOS Hardware Layer');

                // Request initial system info
                this.sendHardwareMessage({
                    type: 'system_info',
                    operation: 'get_status'
                });
            };

            this.websocket.onmessage = (event) => {
                this.handleHardwareMessage(JSON.parse(event.data));
            };

            this.websocket.onclose = (event) => {
                this.isConnected = false;
                console.log('❌ Disconnected from TBOS Hardware Bridge');
                this.showNotification('⚠️ Hardware connection lost - using simulation mode');

                // Try to reconnect after 5 seconds
                setTimeout(() => this.connectToHardware(), 5000);
            };

            this.websocket.onerror = (error) => {
                console.error('🚨 Hardware Bridge WebSocket error:', error);
                this.showNotification('🚨 Hardware communication error');
            };

        } catch (error) {
            console.error('❌ Failed to connect to hardware bridge:', error);
            this.showNotification('⚠️ Hardware bridge unavailable - using simulation mode');
            // Fall back to simulation mode
            this.initSimulationMode();
        }
    }

    sendHardwareMessage(message) {
        if (this.isConnected && this.websocket.readyState === WebSocket.OPEN) {
            message.session_id = this.sessionId;
            message.timestamp = Date.now();
            this.websocket.send(JSON.stringify(message));
            console.log('📤 Sent hardware message:', message.type);
        } else {
            console.log('⚠️ Hardware bridge not connected, queuing message');
            this.messageQueue.push(message);
        }
    }

    handleHardwareMessage(message) {
        console.log('📥 Received hardware message:', message);

        switch (message.type) {
            case 'welcome':
                this.sessionId = message.session_id;
                this.showNotification(`🎉 TBOS Hardware Session ${this.sessionId} established`);
                this.updateHardwareStatus(message);
                break;

            case 'command_result':
                this.handleCommandResult(message);
                break;

            case 'persona_switched':
                this.handlePersonaSwitchResult(message);
                break;

            case 'hardware_data':
                this.updateHardwareData(message);
                break;

            case 'hardware_stats':
                this.updateRealTimeStats(message);
                break;

            case 'hardware_error':
                this.showNotification(`🚨 Hardware Error: ${message.error}`);
                break;

            default:
                console.log('Unknown hardware message type:', message.type);
        }

        // Execute callbacks
        if (this.callbacks[message.type]) {
            this.callbacks[message.type].forEach(callback => callback(message));
        }
    }

    // Execute real TBOS commands through hardware bridge
    async executeHardwareCommand(command) {
        console.log(`💻 Executing hardware command: ${command}`);

        return new Promise((resolve, reject) => {
            const messageId = Date.now();

            // Set up callback for this command
            this.onMessage('command_result', (message) => {
                if (message.command === command) {
                    resolve(message);
                }
            });

            // Send command to hardware bridge
            this.sendHardwareMessage({
                type: 'command',
                command: command,
                message_id: messageId
            });
        });
    }

    // Switch persona through hardware bridge
    async switchPersonaHardware(persona) {
        console.log(`🔄 Switching persona via hardware: ${persona}`);

        return new Promise((resolve, reject) => {
            this.onMessage('persona_switched', (message) => {
                if (message.persona === persona) {
                    this.currentPersona = persona;
                    resolve(message);
                }
            });

            this.sendHardwareMessage({
                type: 'persona_switch',
                persona: persona
            });
        });
    }

    // Access real hardware information
    async getHardwareInfo(operation) {
        console.log(`🔧 Accessing hardware: ${operation}`);

        return new Promise((resolve, reject) => {
            this.onMessage('hardware_data', (message) => {
                if (message.operation === operation) {
                    resolve(message);
                }
            });

            this.sendHardwareMessage({
                type: 'hardware_access',
                operation: operation
            });
        });
    }

    // Real CPU information from hardware
    async getRealCPUInfo() {
        const cpuData = await this.getHardwareInfo('read_cpu_info');
        return {
            model: cpuData.cpu_model,
            speed: cpuData.cpu_speed,
            cores: cpuData.cores
        };
    }

    // Real memory information from hardware
    async getRealMemoryInfo() {
        const memData = await this.getHardwareInfo('read_memory_info');
        return {
            totalKB: memData.total_memory_kb,
            freeKB: memData.free_memory_kb,
            usedKB: memData.used_memory_kb,
            totalGB: (memData.total_memory_kb / 1024 / 1024).toFixed(1),
            usedGB: ((memData.total_memory_kb - memData.free_memory_kb) / 1024 / 1024).toFixed(1)
        };
    }

    // Handle real-time hardware stats updates
    updateRealTimeStats(message) {
        this.hardwareStats = {
            cpuUsage: message.cpu_usage,
            memoryUsage: message.memory_usage,
            totalMemoryMB: message.total_memory_mb,
            freeMemoryMB: message.free_memory_mb,
            uptime: message.uptime
        };

        // Update UI with real hardware stats
        this.updateStatsDisplay();
        console.log('📊 Hardware stats updated:', this.hardwareStats);
    }

    updateStatsDisplay() {
        // Update taskbar stats with real hardware data
        const cpuElement = document.getElementById('cpu-usage');
        const ramElement = document.getElementById('ram-usage');

        if (cpuElement && this.hardwareStats.cpuUsage !== undefined) {
            cpuElement.textContent = `${this.hardwareStats.cpuUsage.toFixed(1)}%`;
        }

        if (ramElement && this.hardwareStats.memoryUsage !== undefined) {
            const usedGB = ((this.hardwareStats.totalMemoryMB - this.hardwareStats.freeMemoryMB) / 1024).toFixed(1);
            const totalGB = (this.hardwareStats.totalMemoryMB / 1024).toFixed(1);
            ramElement.textContent = `${usedGB}GB / ${totalGB}GB`;
        }

        // Update system monitor if open
        const cpuProgress = document.getElementById('cpu-progress');
        const ramProgress = document.getElementById('ram-progress');

        if (cpuProgress && this.hardwareStats.cpuUsage !== undefined) {
            cpuProgress.style.width = `${this.hardwareStats.cpuUsage}%`;
        }

        if (ramProgress && this.hardwareStats.memoryUsage !== undefined) {
            ramProgress.style.width = `${this.hardwareStats.memoryUsage}%`;
        }
    }

    handleCommandResult(message) {
        console.log(`✅ Hardware command completed: ${message.command}`);
        console.log(`📊 Exit code: ${message.exit_code}`);
        console.log(`⚡ Execution time: ${message.execution_time}μs`);
        console.log(`🎯 Routing strategy: ${message.routing_strategy}`);

        // Display command result in terminal if open
        const terminalContent = document.getElementById('terminal-content');
        if (terminalContent && message.output) {
            const outputLine = document.createElement('div');
            outputLine.innerHTML = message.output.replace(/\n/g, '<br>');
            terminalContent.appendChild(outputLine);

            const emptyLine = document.createElement('div');
            terminalContent.appendChild(emptyLine);

            terminalContent.scrollTop = terminalContent.scrollHeight;
        }

        this.showNotification(`✅ Command executed: ${message.command}`);
    }

    handlePersonaSwitchResult(message) {
        console.log(`🔄 Persona switched to: ${message.persona}`);
        console.log(`🔧 Hardware adapted: ${message.hardware_adapted}`);
        console.log(`💾 Memory optimized: ${message.memory_optimized}`);
        console.log(`⚡ Switch time: ${message.switch_time_ms}ms`);

        this.currentPersona = message.persona;
        this.showNotification(`🔄 Hardware persona switched to ${message.persona} in ${message.switch_time_ms}ms`);
    }

    // Event handler registration
    onMessage(type, callback) {
        if (!this.callbacks[type]) {
            this.callbacks[type] = [];
        }
        this.callbacks[type].push(callback);
    }

    // Simulation mode fallback
    initSimulationMode() {
        console.log('🎭 Initializing simulation mode...');
        this.isConnected = false;

        // Simulate hardware stats updates
        setInterval(() => {
            this.hardwareStats = {
                cpuUsage: 10 + Math.random() * 20,
                memoryUsage: 40 + Math.random() * 20,
                totalMemoryMB: 16384,
                freeMemoryMB: 8192 + Math.random() * 4096
            };
            this.updateStatsDisplay();
        }, 3000);
    }

    // Utility functions
    setupEventHandlers() {
        // Handle page unload
        window.addEventListener('beforeunload', () => {
            if (this.websocket) {
                this.websocket.close();
            }
        });
    }

    showNotification(message) {
        console.log('🔔', message);

        // Show notification in UI if available
        if (typeof showNotification === 'function') {
            showNotification(message);
        }
    }

    // Check if hardware bridge is available
    isHardwareConnected() {
        return this.isConnected;
    }

    // Get current session info
    getSessionInfo() {
        return {
            sessionId: this.sessionId,
            persona: this.currentPersona,
            connected: this.isConnected,
            stats: this.hardwareStats
        };
    }
}

// Global TBOS Hardware Bridge instance
const tbosHardware = new TBOSHardwareBridge();

// Enhanced terminal command processing with hardware integration
function processHardwareCommand(command) {
    const cmd = command.toLowerCase().trim();

    // Route certain commands through hardware bridge
    if (cmd === 'sysinfo' || cmd.startsWith('persona ') || cmd.startsWith('quantum ')) {
        tbosHardware.executeHardwareCommand(command).then(result => {
            console.log('Hardware command result:', result);
        }).catch(error => {
            console.error('Hardware command failed:', error);
        });
        return 'Command sent to hardware layer...';
    }

    // For other commands, use the existing local processing
    return processCommand(command);
}

// Enhanced persona switching with hardware integration
function switchPersonaHardware(persona) {
    if (tbosHardware.isHardwareConnected()) {
        tbosHardware.switchPersonaHardware(persona).then(result => {
            console.log('Hardware persona switch result:', result);

            // Update UI based on hardware response
            updatePersonaUI(persona, result);
        }).catch(error => {
            console.error('Hardware persona switch failed:', error);
            // Fall back to UI-only switch
            switchPersona(persona);
        });
    } else {
        // Fall back to UI-only switch
        switchPersona(persona);
    }
}

function updatePersonaUI(persona, hardwareResult) {
    const body = document.body;
    const indicator = document.getElementById('persona-indicator');
    const quantumIcon = document.getElementById('quantum-icon');

    // Update radio buttons
    const radios = document.querySelectorAll('input[name="persona"]');
    radios.forEach(radio => {
        radio.checked = radio.value === persona;
    });

    // Remove all persona classes
    body.className = '';

    switch(persona) {
        case 'chemos':
            body.className = 'quantum-mode';
            indicator.textContent = '⚛️ ChemOS Quantum (Hardware)';
            quantumIcon.style.display = 'flex';
            break;
        case 'mobile':
            indicator.textContent = '📱 Mobile Mode (Hardware)';
            quantumIcon.style.display = 'none';
            break;
        default:
            indicator.textContent = '💻 Desktop Mode (Hardware)';
            quantumIcon.style.display = 'none';
    }

    // Show hardware-specific notification
    if (hardwareResult && hardwareResult.hardware_adapted) {
        showNotification(`🔧 Hardware adapted for ${persona} - Memory optimized: ${hardwareResult.memory_optimized}`);
    }
}

// Enhanced system monitor with real hardware data
async function updateSystemMonitorHardware() {
    if (tbosHardware.isHardwareConnected()) {
        try {
            // Get real CPU info
            const cpuInfo = await tbosHardware.getRealCPUInfo();
            const cpuDetail = document.getElementById('cpu-detail');
            if (cpuDetail) {
                cpuDetail.textContent = `${cpuInfo.model} (${cpuInfo.cores} cores)`;
            }

            // Get real memory info
            const memInfo = await tbosHardware.getRealMemoryInfo();
            const ramDetail = document.getElementById('ram-detail');
            if (ramDetail) {
                ramDetail.textContent = `${memInfo.usedGB}GB / ${memInfo.totalGB}GB used`;
            }

        } catch (error) {
            console.error('Failed to get hardware info:', error);
        }
    }
}

// Initialize hardware monitoring when system monitor opens
function openAppHardware(appName) {
    openApp(appName);

    if (appName === 'monitor') {
        setTimeout(updateSystemMonitorHardware, 500);
    }
}

// Export for global use
window.tbosHardware = tbosHardware;
window.processHardwareCommand = processHardwareCommand;
window.switchPersonaHardware = switchPersonaHardware;
window.openAppHardware = openAppHardware;

console.log('🚀 TBOS Hardware Interface loaded - Revolutionary web-to-hardware bridge active!');
console.log('🌟 HTML5 can now directly control TBOS hardware layers!');