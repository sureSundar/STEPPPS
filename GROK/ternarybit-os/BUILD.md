# TernaryBit OS Build System

## Overview
This document provides comprehensive documentation for building and testing TernaryBit OS across different platforms. The build system is designed to be modular, maintainable, and cross-platform compatible.

## Prerequisites

### Linux/macOS
- GCC or Clang
- GNU Make
- QEMU (for testing)
- Git

### Windows
- MinGW-w64 or MSYS2
- Git for Windows

## Quick Start

### Linux/macOS
```bash
# Clone the repository
git clone https://github.com/yourusername/ternarybit-os.git
cd ternarybit-os

# Set up build environment
./scripts/setup-build-env.sh

# Build everything
make all

# Run tests
make test

# Start the OS in QEMU
make run
```

### Windows
```bash
# Using Git Bash or MSYS2 terminal
# Clone the repository
git clone https://github.com/yourusername/ternarybit-os.git
cd ternarybit-os

# Run the Windows build script
.\deploy\windows\build-tbos-windows.bat

# The built binaries will be in build/windows/bin/
```

## Build System Architecture

The build system is organized into modular components:

- `Makefile` - Main build entry point
- `config/` - Build configuration
  - `targets.mk` - Build targets and rules
  - `flags.mk` - Compiler/linker flags
  - `tools.mk` - Toolchain configuration
- `scripts/` - Build and utility scripts
  - `setup-build-env.sh` - Environment setup
  - `run-tests.sh` - Test runner
- `deploy/` - Platform-specific deployment
  - `windows/` - Windows build scripts

## Build Targets

| Target | Description |
|--------|-------------|
| `all` | Build all components (default) |
| `clean` | Remove build artifacts |
| `test` | Run all tests |
| `run` | Start the OS in QEMU |
| `debug` | Start with GDB debugger |
| `doc` | Generate documentation |

## Cross-Compilation

To cross-compile for different architectures:

```bash
# For x86_64
make ARCH=x86_64

# For ARM
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
```

## Testing

The test suite includes:

- Unit tests (in `tests/unit/`)
- Integration tests (in `tests/integration/`)
- System tests (using QEMU)

Run tests with:
```bash
make test
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add/update tests
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
