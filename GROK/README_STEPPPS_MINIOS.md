# SundarOS - Universal Operating System

## Overview

SundarOS is a revolutionary universal operating system powered by the STEPPPS framework (Space, Time, Event, Psychology, Pixel, Prompt, Script). It's designed to run on any digital device, from powerful servers to minimal devices like calculators, while maintaining a single-file constraint per component for maximum compatibility.

## Architecture

### Core Components

1. **Kernel Abstraction Layer** (`steppps_minios_kernel.py`)
   - Hardware detection and initialization
   - STEPPPS dimension subsystems
   - Dual-mode operation (hosted/bare-metal)
   - System service orchestration

2. **Process Management** (`steppps_minios_process.py`)
   - STEPPPS-native process control blocks
   - AI-aware adaptive scheduling
   - Inter-process communication
   - Autonomous process behavior

3. **Memory Management** (`steppps_minios_memory.py`)
   - Virtual memory with paging
   - Garbage collection
   - AI-driven optimization
   - STEPPPS metadata integration

4. **Device Drivers** (`steppps_minios_drivers.py`)
   - Universal device interface
   - Generic, storage, and network drivers
   - STEPPPS-enhanced device management
   - Hot-plug support

5. **File System** (`steppps_minios_filesystem.py`)
   - Virtual file system abstraction
   - STEPPPS metadata support
   - Universal file operations
   - Directory management

6. **Network Stack** (`steppps_minios_network.py`)
   - TCP/UDP/STEPPPS protocols
   - Service discovery
   - Inter-STEPPPS communication
   - AI-enhanced networking

7. **Unified Runtime** (`steppps_minios.py`)
   - Main OS orchestrator
   - System integration
   - Command interface
   - STEPPPS state management

## STEPPPS Framework Integration

Each subsystem integrates all seven STEPPPS dimensions:

- **Space**: Physical resources, hardware abstraction, deployment topology
- **Time**: Scheduling, timing, temporal coordination, system uptime
- **Event**: Interrupts, system events, error handling, monitoring
- **Psychology**: Adaptive behavior, AI optimization, system personality
- **Pixel**: Visualization, display management, user interfaces
- **Prompt**: AI assistance, natural language interfaces, help systems
- **Script**: Automation, orchestration, autonomous behavior, plugins

## Features

- **Universal Deployment**: Runs on all digital devices (servers, laptops, embedded systems, calculators)
- **Dual-Mode Operation**: Hosted mode (on existing OS) and bare-metal mode (direct hardware)
- **STEPPPS Integration**: Full integration of all seven STEPPPS dimensions
- **Single-File Components**: Each subsystem is a single Python file for maximum compatibility
- **AI-Enhanced Operations**: Intelligent scheduling, memory management, and networking
- **Autonomous Behavior**: Self-optimizing and adaptive system behavior
- **Inter-STEPPPS Communication**: Native protocol for STEPPPS node communication
- **Graceful Degradation**: Adapts to device capabilities and available resources
- Autonomous system evolution
- AI-driven optimization
- Natural language interfaces

## Quick Start

### Basic Usage

```bash
# Run SundarOS in hosted mode
python3 steppps_minios.py

# Run in bare-metal mode (requires appropriate hardware setup)
python3 steppps_minios.py --bare-metal

# Run comprehensive tests
python3 steppps_minios_test.py --full

# Run quick tests
python3 steppps_minios_test.py --quick
```

### Interactive Demo

Once SundarOS boots, you'll see an interactive command prompt:

```
SundarOS> ps          # List processes
SundarOS> free        # Show memory usage
SundarOS> df          # Show filesystem info
SundarOS> netstat     # Show network status
SundarOS> uptime      # Show system uptime
SundarOS> steppps     # Show STEPPPS dimensions
SundarOS> stats       # Show system statistics
SundarOS> quit        # Shutdown system
```

## Testing

### Quick Test
```bash
python3 steppps_minios_test.py --quick
```

### Comprehensive Test Suite
```bash
python3 steppps_minios_test.py --full
```

### Individual Component Tests
```bash
# Test kernel
python3 steppps_minios_kernel.py

# Test memory management
python3 steppps_minios_memory.py

# Test process management
python3 steppps_minios_process.py

# Test file system
python3 steppps_minios_filesystem.py

# Test network stack
python3 steppps_minios_network.py

# Test device drivers
python3 steppps_minios_drivers.py
```

## Configuration Options

### SystemConfiguration Parameters

- `operation_mode`: "hosted", "bare_metal", "virtual", "container"
- `max_processes`: Maximum number of concurrent processes
- `memory_limit_mb`: Memory limit in megabytes
- `max_open_files`: Maximum open file handles
- `max_network_connections`: Maximum network connections
- `steppps_enabled`: Enable STEPPPS framework features
- `ai_optimization`: Enable AI-driven optimizations
- `autonomous_mode`: Enable autonomous system behavior
- `enable_gui`: Enable graphical interface
- `enable_cli`: Enable command line interface
- `enable_web_interface`: Enable web-based interface
- `enable_sandboxing`: Enable process sandboxing
- `log_level`: Logging level ("DEBUG", "INFO", "WARNING", "ERROR")
- `debug_mode`: Enable debug output

## System Requirements

### Minimum Requirements
- Python 3.7+
- 16MB RAM (for minimal devices)
- Basic I/O capabilities

### Recommended Requirements
- Python 3.8+
- 64MB+ RAM
- Network connectivity
- Storage device access

### Optional Dependencies
- `psutil`: Enhanced hardware detection
- `tkinter`: GUI interface support
- Network libraries for advanced networking features (usually built-in)

## Architecture Diagrams

### System Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                      SundarOS                               │
├─────────────────────────────────────────────────────────────┤
│  Unified Runtime (steppps_minios.py)                       │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐          │
│  │   Kernel    │ │  Processes  │ │   Memory    │          │
│  │ Abstraction │ │ Management  │ │ Management  │          │
│  └─────────────┘ └─────────────┘ └─────────────┘          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐          │
│  │   Device    │ │ File System │ │   Network   │          │
│  │   Drivers   │ │ Abstraction │ │    Stack    │          │
│  └─────────────┘ └─────────────┘ └─────────────┘          │
├─────────────────────────────────────────────────────────────┤
│                STEPPPS Framework                            │
│  Space │ Time │ Event │ Psychology │ Pixel │ Prompt │ Script│
├─────────────────────────────────────────────────────────────┤
│              Hardware Abstraction Layer                     │
│  CPU │ Memory │ Storage │ Network │ Display │ Input │ Audio │
└─────────────────────────────────────────────────────────────┘
```

### STEPPPS Dimensions Integration
```
┌─────────────────────────────────────────────────────────────┐
│                    STEPPPS Dimensions                       │
├─────────────────────────────────────────────────────────────┤
│ SPACE    │ Hardware resources, deployment topology         │
│ TIME     │ Scheduling, timing, temporal coordination       │
│ EVENT    │ Interrupts, system events, monitoring          │
│ PSYCHOLOGY│ Adaptive behavior, AI optimization            │
│ PIXEL    │ Visualization, display management              │
│ PROMPT   │ AI assistance, natural language interfaces     │
│ SCRIPT   │ Automation, orchestration, autonomous behavior │
└─────────────────────────────────────────────────────────────┘
```

## Development

### Adding New Components

1. Follow STEPPPS integration pattern
2. Implement required interfaces
3. Add STEPPPS dimensions initialization
4. Include comprehensive error handling
5. Add unit tests
6. Update documentation

### Contributing

1. Fork the repository
2. Create feature branch
3. Implement changes with tests
4. Ensure all tests pass
5. Submit pull request

## Troubleshooting

### Common Issues

1. **Import Errors**: Ensure all files are in the same directory
2. **Memory Issues**: Reduce `memory_limit_mb` for low-memory systems
3. **Process Limits**: Adjust `max_processes` based on system capabilities
4. **Network Issues**: Check firewall settings for network features

### Debug Mode

Enable debug mode for detailed logging:
```python
config = steppps_minios.SystemConfiguration(debug_mode=True)
```

### Log Analysis

Check system logs for detailed error information and performance metrics.

## License

STEPPPS Mini OS is part of the STEPPPS framework project. See project license for details.

## Version History

- **v1.0**: Initial release with full STEPPPS integration
  - Kernel abstraction layer
  - Process and memory management
  - Device drivers and file system
  - Network stack integration
  - Unified runtime system
  - Comprehensive test suite

## Contact

For questions, issues, or contributions, please refer to the main STEPPPS project documentation.
