# TernaryBit OS - Universal Shell Makefile
# Cross-compilation support for multiple architectures

# ═══════════════════════════════════════════════════════════════════════════
# ARCHITECTURE DETECTION & CROSS-COMPILATION
# ═══════════════════════════════════════════════════════════════════════════
# Usage:
#   make                    # Build for current host
#   make ARCH=arm64         # Cross-compile for ARM64/aarch64
#   make ARCH=arm32         # Cross-compile for ARM32/armv7
#   make ARCH=riscv64       # Cross-compile for RISC-V 64-bit
#   make ARCH=windows       # Cross-compile for Windows (MinGW)
#   make ARCH=android       # Cross-compile for Android (needs NDK)
#   make ARCH=wasm          # Compile to WebAssembly (needs emscripten)
#
# Install cross-compilers:
#   macOS:   brew install aarch64-elf-gcc riscv64-elf-gcc mingw-w64
#   Ubuntu:  apt install gcc-aarch64-linux-gnu gcc-arm-linux-gnueabihf \
#                        gcc-riscv64-linux-gnu mingw-w64
#   Arch:    pacman -S aarch64-linux-gnu-gcc arm-none-eabi-gcc \
#                      riscv64-linux-gnu-gcc mingw-w64-gcc

ARCH ?= host
BUILD_DIR ?= build

# Platform-specific settings
ifeq ($(ARCH),arm64)
    CC = aarch64-linux-gnu-gcc
    TARGET_SUFFIX = -arm64
    CROSS_CFLAGS = -DTBOS_ARCH_ARM64
    CROSS_LDFLAGS =
else ifeq ($(ARCH),arm32)
    CC = arm-linux-gnueabihf-gcc
    TARGET_SUFFIX = -arm32
    CROSS_CFLAGS = -DTBOS_ARCH_ARM32
    CROSS_LDFLAGS =
else ifeq ($(ARCH),riscv64)
    CC = riscv64-linux-gnu-gcc
    TARGET_SUFFIX = -riscv64
    CROSS_CFLAGS = -DTBOS_ARCH_RISCV64
    CROSS_LDFLAGS =
else ifeq ($(ARCH),windows)
    CC = x86_64-w64-mingw32-gcc
    TARGET_SUFFIX = .exe
    CROSS_CFLAGS = -DTBOS_ARCH_WINDOWS -D_WIN32
    CROSS_LDFLAGS = -lws2_32
    # Windows doesn't have iconv by default
    LDFLAGS_ICONV =
else ifeq ($(ARCH),android)
    # Android NDK - set ANDROID_NDK_ROOT environment variable
    ANDROID_API ?= 33
    ANDROID_ABI ?= arm64-v8a
    ifeq ($(ANDROID_ABI),arm64-v8a)
        CC = $(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/*/bin/aarch64-linux-android$(ANDROID_API)-clang
    else ifeq ($(ANDROID_ABI),armeabi-v7a)
        CC = $(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/*/bin/armv7a-linux-androideabi$(ANDROID_API)-clang
    else ifeq ($(ANDROID_ABI),x86_64)
        CC = $(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/*/bin/x86_64-linux-android$(ANDROID_API)-clang
    endif
    TARGET_SUFFIX = -android-$(ANDROID_ABI)
    CROSS_CFLAGS = -DTBOS_ARCH_ANDROID -DANDROID
    CROSS_LDFLAGS =
    LDFLAGS_ICONV =
else ifeq ($(ARCH),wasm)
    CC = emcc
    TARGET_SUFFIX = .html
    CROSS_CFLAGS = -DTBOS_ARCH_WASM -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'
    CROSS_LDFLAGS = -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1
    LDFLAGS_ICONV =
else
    # Host platform (default)
    CC ?= gcc
    TARGET_SUFFIX =
    CROSS_CFLAGS =
    CROSS_LDFLAGS =
    LDFLAGS_ICONV = -liconv
endif

CFLAGS = -Wall -Wextra -g -DTBOS_HOSTED -I../../../include -I../../core -I../common -I../../../include/fs $(CROSS_CFLAGS)
LDFLAGS = -lm $(LDFLAGS_ICONV) $(CROSS_LDFLAGS)

# Source files
SHELL_SRCS = \
    tbos_universal_shell.c \
    tbos_shell_parser.c \
    tbos_shell_karma.c \
    tbos_terminal.c \
    tbos_line_editor.c \
    tbos_completion.c \
    tbos_syntax_hl.c \
    tbos_progress.c \
    tbos_cmd_api.c \
    commands_core.c \
    commands_week1.c \
    commands_week2.c \
    commands_week3.c \
    commands_week4.c \
    commands_week5.c \
    commands_week6.c \
    commands_week7.c \
    commands_week8.c \
    commands_week9.c \
    commands_week10.c \
    commands_pxfs.c \
    commands_ucfs.c \
    commands_host.c \
    main_shell.c
SHELL_OBJS = $(SHELL_SRCS:.c=.o)

# Dependencies (stubs + network + wow + dns + process + soul + filesystems)
DEPS_SRCS = ../../core/stubs/tbos_ramdisk.c \
            ../../core/stubs/tbos_hal.c \
            ../../core/stubs/tbos_consciousness.c \
            ../../core/stubs/tbos_karma.c \
            ../../core/stubs/tbos_sangha.c \
            ../../core/stubs/tbos_boot_stub.c \
            ../../core/process/tbos_process.c \
            ../../core/soul/tbos_soul.c \
            ../../core/filesystem/ucfs_codec.c \
            ../../core/filesystem/rf2s_codec.c \
            ../../network/tbos_net.c \
            ../../network/wow/tbos_wow.c \
            ../../network/dns/tbos_dns.c \
            ../../../kernel/hal_hosted.c
DEPS_OBJS = $(DEPS_SRCS:.c=.o)

# Targets
TARGET = tbos_shell$(TARGET_SUFFIX)
API_SERVER = tbos_api_server$(TARGET_SUFFIX)

# Library sources (shared between shell and API server - excludes main_shell.c)
LIB_SRCS = $(filter-out main_shell.c,$(SHELL_SRCS))
LIB_OBJS = $(LIB_SRCS:.c=.o)

# ═══════════════════════════════════════════════════════════════════════════
# BUILD RULES
# ═══════════════════════════════════════════════════════════════════════════
all: $(TARGET) $(API_SERVER) post_build

# Cross-compilation convenience targets
arm64:
	@$(MAKE) ARCH=arm64
	@echo "ARM64 binary: ./tbos_shell-arm64"

arm32:
	@$(MAKE) ARCH=arm32
	@echo "ARM32 binary: ./tbos_shell-arm32"

riscv64:
	@$(MAKE) ARCH=riscv64
	@echo "RISC-V 64-bit binary: ./tbos_shell-riscv64"

windows:
	@$(MAKE) ARCH=windows
	@echo "Windows binary: ./tbos_shell.exe"

android:
	@if [ -z "$(ANDROID_NDK_ROOT)" ]; then \
		echo "ERROR: Set ANDROID_NDK_ROOT environment variable"; \
		echo "  export ANDROID_NDK_ROOT=/path/to/android-ndk"; \
		exit 1; \
	fi
	@$(MAKE) ARCH=android
	@echo "Android binary: ./tbos_shell-android-$(ANDROID_ABI)"

wasm:
	@if ! command -v emcc >/dev/null; then \
		echo "ERROR: emscripten not found. Install with:"; \
		echo "  brew install emscripten  # macOS"; \
		echo "  apt install emscripten   # Ubuntu"; \
		exit 1; \
	fi
	@$(MAKE) ARCH=wasm
	@echo "WebAssembly: ./tbos_shell.html (open in browser)"

# Build all supported architectures
all-archs: clean
	@echo "Building for all architectures..."
	@$(MAKE) ARCH=host
	@$(MAKE) clean-objs
	@$(MAKE) ARCH=arm64 2>/dev/null || echo "  [skip] arm64 (cross-compiler not found)"
	@$(MAKE) clean-objs
	@$(MAKE) ARCH=arm32 2>/dev/null || echo "  [skip] arm32 (cross-compiler not found)"
	@$(MAKE) clean-objs
	@$(MAKE) ARCH=riscv64 2>/dev/null || echo "  [skip] riscv64 (cross-compiler not found)"
	@$(MAKE) clean-objs
	@$(MAKE) ARCH=windows 2>/dev/null || echo "  [skip] windows (mingw not found)"
	@echo ""
	@echo "Built binaries:"
	@ls -la tbos_shell* 2>/dev/null || true

clean-objs:
	@rm -f $(SHELL_OBJS) $(DEPS_OBJS) tbos_api_server.o

$(TARGET): $(SHELL_OBJS) $(DEPS_OBJS)
	@echo "🔨 Linking Universal Shell..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ Shell built: ./$(TARGET)"

$(API_SERVER): tbos_api_server.o $(LIB_OBJS) $(DEPS_OBJS)
	@echo "🔨 Linking API Server..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ API Server built: ./$(API_SERVER)"

# Success message after all builds
.PHONY: post_build
post_build: $(TARGET) $(API_SERVER)
	@echo ""
	@echo "╔══════════════════════════════════════════════════════╗"
	@echo "║   TernaryBit OS - Build Complete!                    ║"
	@echo "╠══════════════════════════════════════════════════════╣"
	@echo "║   Terminal Shell:  ./$(TARGET)                       ║"
	@echo "║   API Server:      ./$(API_SERVER)                   ║"
	@echo "║   Web Server:      python3 web/tbos_simple_server.py ║"
	@echo "╚══════════════════════════════════════════════════════╝"
	@echo ""

%.o: %.c
	@echo "📝 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

../../core/stubs/%.o: ../../core/stubs/%.c
	@echo "📝 Compiling stub $<..."
	@mkdir -p ../../core/stubs
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo ""
	@echo "╔══════════════════════════════════════════════════════╗"
	@echo "║   Running Universal Shell                            ║"
	@echo "╚══════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Type 'help' for commands, 'exit' to quit"
	@echo ""
	./$(TARGET)

clean:
	@echo "🧹 Cleaning..."
	rm -f $(SHELL_OBJS) $(DEPS_OBJS) tbos_api_server.o
	rm -f tbos_shell tbos_shell-* tbos_shell.exe tbos_shell.html tbos_shell.js tbos_shell.wasm
	rm -f tbos_api_server tbos_api_server-* tbos_api_server.exe
	@echo "✅ Clean complete"

web: $(API_SERVER)
	@echo "🌐 Starting Web UI..."
	@echo "   API Server: http://localhost:8080"
	@echo "   Web UI:     http://localhost:8888"
	@echo ""
	./$(API_SERVER) 8080 &
	python3 web/tbos_simple_server.py

# ═══════════════════════════════════════════════════════════════════════════
# HELP
# ═══════════════════════════════════════════════════════════════════════════
help:
	@echo "TernaryBit OS Shell - Cross-Platform Build System"
	@echo ""
	@echo "Usage: make [target] [ARCH=architecture]"
	@echo ""
	@echo "Build targets:"
	@echo "  all         Build shell and API server for current/specified arch"
	@echo "  arm64       Build for ARM64/aarch64 (Raspberry Pi 4, Apple M1, etc.)"
	@echo "  arm32       Build for ARM32/armv7 (Raspberry Pi 3, older ARM)"
	@echo "  riscv64     Build for RISC-V 64-bit"
	@echo "  windows     Build for Windows (requires MinGW)"
	@echo "  android     Build for Android (requires NDK)"
	@echo "  wasm        Build for WebAssembly (requires emscripten)"
	@echo "  all-archs   Build for all available architectures"
	@echo ""
	@echo "Other targets:"
	@echo "  test        Build and run shell interactively"
	@echo "  web         Start web UI server"
	@echo "  clean       Remove all build artifacts"
	@echo "  help        Show this help"
	@echo ""
	@echo "Architecture variable:"
	@echo "  ARCH=host     Build for current platform (default)"
	@echo "  ARCH=arm64    Build for ARM64"
	@echo "  ARCH=arm32    Build for ARM32"
	@echo "  ARCH=riscv64  Build for RISC-V 64-bit"
	@echo "  ARCH=windows  Build for Windows"
	@echo "  ARCH=android  Build for Android (set ANDROID_NDK_ROOT)"
	@echo "  ARCH=wasm     Build for WebAssembly"
	@echo ""
	@echo "Examples:"
	@echo "  make                         # Build for current host"
	@echo "  make arm64                   # Cross-compile for ARM64"
	@echo "  make ARCH=arm64 clean all    # Clean and rebuild for ARM64"
	@echo "  make all-archs               # Build for all architectures"

.PHONY: all test clean web post_build help
.PHONY: arm64 arm32 riscv64 windows android wasm all-archs clean-objs
