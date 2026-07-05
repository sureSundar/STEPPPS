.PHONY: paper
paper:
	@echo "[make] Building research paper via scripts/build-paper.sh"
	@bash scripts/build-paper.sh
	@echo "[make] Done. See out/paper/RESEARCH_PAPER.{pdf,html}"
# TernaryBit Universal OS - Modular Build System
# Traceability: PRD-044 (Build System Enhancement)
# Supports cross-compilation for x86_64, ARM, and RISC-V architectures

# Default architecture (can be overridden: make ARCH=arm64)
ARCH ?= x86_64
BUILD_DIR = build/$(ARCH)

# Include configuration
include config/flags.mk
include config/targets.mk
include config/tools.mk

# Persona & package-driven build orchestration
PERSONA ?= desktop_512m
PERSONAS ?=
PACKAGES ?=
TBOS_PACKAGE_DIR := config/packages
TBOS_PACKAGE_DEFINES :=
TBOS_PACKAGE_NOTES :=

ifneq ($(strip $(PACKAGES)),)
PKG_FILES := $(foreach pkg,$(PACKAGES),$(TBOS_PACKAGE_DIR)/$(pkg).mk)
$(foreach file,$(PKG_FILES),$(if $(wildcard $(file)),$(eval include $(file)),$(error Unknown package '$(basename $(notdir $(file)))' (missing $(file)))))
endif

export PERSONA
export PACKAGES
export TBOS_PACKAGE_DEFINES
export TBOS_PACKAGE_NOTES

# Source directories
SRC_DIR = src
TEST_DIR = tests
ARCH_DIR = arch/$(ARCH)

# WINDSURF: Additional source roots (scaffolded)
DRV_DIR = drivers
FS_DIR  = fs
NET_DIR = net
SHELL2_DIR = shell

# Source files
CORE_SRCS = $(wildcard $(SRC_DIR)/core/*.c) \
            $(wildcard $(SRC_DIR)/core/compression/*.c) \
            $(wildcard $(SRC_DIR)/core/filesystem/*.c) \
            $(wildcard $(ARCH_DIR)/*.c)
BOOT_SRCS = $(wildcard $(SRC_DIR)/boot/*.c)
SHELL_SRCS = $(wildcard $(SRC_DIR)/shell/*.c)

# WINDSURF: New scaffolding sources
DRV_SRCS  = $(shell find $(DRV_DIR) -type f -name '*.c')
FS_SRCS   = $(shell find $(FS_DIR)  -type f -name '*.c')
NET_SRCS  = $(shell find $(NET_DIR) -type f -name '*.c')
SHELL2_SRCS = $(shell find $(SHELL2_DIR) -maxdepth 1 -type f -name '*.c')

# Object files
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CORE_SRCS))
BOOT_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(BOOT_SRCS))
SHELL_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SHELL_SRCS))

# WINDSURF: Objects for new scaffolding
DRV_OBJS    = $(patsubst $(DRV_DIR)/%.c,$(BUILD_DIR)/$(DRV_DIR)/%.o,$(DRV_SRCS))
FS_OBJS     = $(patsubst $(FS_DIR)/%.c,$(BUILD_DIR)/$(FS_DIR)/%.o,$(FS_SRCS))
NET_OBJS    = $(patsubst $(NET_DIR)/%.c,$(BUILD_DIR)/$(NET_DIR)/%.o,$(NET_SRCS))
SHELL2_OBJS = $(patsubst $(SHELL2_DIR)/%.c,$(BUILD_DIR)/$(SHELL2_DIR)/%.o,$(SHELL2_SRCS))

# WINDSURF: Host build (no cross toolchain) for quick validation
HOST_BUILD_DIR = build/host
HOST_CC = gcc
HOST_CFLAGS = -Wall -Wextra -O0 -g
HOST_DRV_OBJS    = $(patsubst $(DRV_DIR)/%.c,$(HOST_BUILD_DIR)/$(DRV_DIR)/%.o,$(DRV_SRCS))
HOST_FS_OBJS     = $(patsubst $(FS_DIR)/%.c,$(HOST_BUILD_DIR)/$(FS_DIR)/%.o,$(FS_SRCS))
HOST_NET_OBJS    = $(patsubst $(NET_DIR)/%.c,$(HOST_BUILD_DIR)/$(NET_DIR)/%.o,$(NET_SRCS))
HOST_SHELL2_OBJS = $(patsubst $(SHELL2_DIR)/%.c,$(HOST_BUILD_DIR)/$(SHELL2_DIR)/%.o,$(SHELL2_SRCS))

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/unit/test_*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/unit/%.c,$(BUILD_DIR)/%,$(TEST_SRCS))

# Default target
all: $(BUILD_TARGETS) $(SHELL_TARGETS)
	@echo "🏗️  TernaryBit OS for $(ARCH) built successfully"
	@echo "  Kernel: $(KERNEL_BIN)"
	@echo "  Bootloader: $(BOOTLOADER_BIN)"
	@echo "  Shell: $(SHELL_TARGETS)"
	@echo "\nRun 'make test' to execute tests or 'make run' to start in QEMU"

# WINDSURF: Build scaffolding-only (drivers, fs, net, shell stubs)
.PHONY: drivers
drivers: | $(BUILD_DIR)
	@echo "🔧 Building scaffolding (drivers/fs/net/shell) for $(ARCH)..."
	@$(MAKE) --no-print-directory $(DRV_OBJS) $(FS_OBJS) $(NET_OBJS) $(SHELL2_OBJS)
	@echo "✅ Scaffolding objects built under $(BUILD_DIR)/"

.PHONY: drivers-host
drivers-host:
	@echo "🔧 Building scaffolding (drivers/fs/net/shell) with host GCC..."
	@$(MAKE) --no-print-directory $(HOST_DRV_OBJS) $(HOST_FS_OBJS) $(HOST_NET_OBJS) $(HOST_SHELL2_OBJS)
	@echo "✅ Host scaffolding objects built under $(HOST_BUILD_DIR)/"

.PHONY: hosted hosted-test
hosted:
	@echo "🌐 Building hosted universal shell..."
	@./hosted/build_hosted.sh

hosted-test: hosted
	@echo "🧪 Running hosted universal shell smoke test..."
	@./hosted/test_hosted.sh

.PHONY: test-fat32-host
test-fat32-host:
	@echo "🧪 Building FAT32 host test..."
	@gcc -Wall -Wextra -g -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_fat32.c \
		src/core/filesystem/blockdev.c \
		src/core/filesystem/fat32.c \
		kernel/fs/vfs.c \
		-o build/host_fat32_test
	@echo "🏃 Running FAT32 host test..."
	@./build/host_fat32_test

.PHONY: test-ucfs-host
test-ucfs-host:
	@echo "🧪 Building UCFS codec host test..."
	@gcc -Wall -Wextra -g -DHOST_BUILD -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_ucfs_codec.c \
		src/core/filesystem/ucfs_codec.c \
		-o build/host_ucfs_codec_test
	@echo "🏃 Running UCFS codec host test..."
	@./build/host_ucfs_codec_test

.PHONY: test-ucfs-overlay-host
test-ucfs-overlay-host:
	@echo "🧪 Building UCFS overlay host test..."
	@gcc -Wall -Wextra -g -DHOST_BUILD -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_ucfs_overlay.c \
		src/core/filesystem/ucfs_codec.c \
		src/core/filesystem/ucfs_overlay.c \
		kernel/fs/vfs.c \
		kernel/fs/ramfs.c \
		-o build/host_ucfs_overlay_test
	@echo "🏃 Running UCFS overlay host test..."
	@./build/host_ucfs_overlay_test

# Kernel target
$(KERNEL_BIN): $(CORE_OBJS) $(SHELL_OBJS) | $(BUILD_DIR)
	@echo "🔗 Linking $@ for $(ARCH)..."
	@$(CC) $(LDFLAGS) -T $(ARCH_DIR)/linker.ld -o $@ $^ $(LIBS)

# Bootloader target
$(BOOTLOADER_BIN): $(BOOT_OBJS) | $(BUILD_DIR)
	@echo "🔗 Linking $@ for $(ARCH)..."
	@$(CC) $(LDFLAGS) -T $(ARCH_DIR)/bootloader.ld -o $@ $^ $(LIBS)

# Shell target
$(SHELL_TARGETS): %: $(BUILD_DIR)/shell/%
	@ln -sf $< $@

# Test targets
$(BUILD_DIR)/%: $(TEST_DIR)/unit/%.c $(filter-out %main.o,$(CORE_OBJS)) | $(BUILD_DIR)
	@mkdir -p $(@D)
	@echo "🧪 Building test $@ for $(ARCH)..."
	@$(CC) $(TEST_CFLAGS) -o $@ $^ $(TEST_LDFLAGS)

# Compile .c to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# WINDSURF: Compile rules for new directories
$(BUILD_DIR)/$(DRV_DIR)/%.o: $(DRV_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [DRV] Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(FS_DIR)/%.o: $(FS_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [FS] Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(NET_DIR)/%.o: $(NET_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [NET] Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(SHELL2_DIR)/%.o: $(SHELL2_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [SHELL] Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# WINDSURF: Host compile rules
$(HOST_BUILD_DIR)/$(DRV_DIR)/%.o: $(DRV_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [HOST-DRV] Compiling $<..."
	@$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

$(HOST_BUILD_DIR)/$(FS_DIR)/%.o: $(FS_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [HOST-FS] Compiling $<..."
	@$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

$(HOST_BUILD_DIR)/$(NET_DIR)/%.o: $(NET_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [HOST-NET] Compiling $<..."
	@$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

$(HOST_BUILD_DIR)/$(SHELL2_DIR)/%.o: $(SHELL2_DIR)/%.c
	@mkdir -p $(@D)
	@echo "🔨 [HOST-SHELL] Compiling $<..."
	@$(HOST_CC) $(HOST_CFLAGS) -c $< -o $@

# Include dependency files
-include $(CORE_OBJS:.o=.d) $(BOOT_OBJS:.o=.d) $(SHELL_OBJS:.o=.d)

# Hardware Detector Test
test_hardware_detector: $(BUILD_DIR)/test_hardware_detector
$(BUILD_DIR)/test_hardware_detector: $(TEST_DIR)/test_hardware_detector.c $(HW_DETECTOR_SRCS) | directories
	@echo "🔨 Building Hardware Detector Test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Universal Boot Test
test_universal_boot: $(BUILD_DIR)/test_universal_boot
$(BUILD_DIR)/test_universal_boot: $(TEST_DIR)/test_universal_boot.c $(BOOT_SRCS) $(HW_DETECTOR_SRCS) | directories
	@echo "🔨 Building Universal Boot Test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# STEPPPS Framework Test
test_steppps_framework: $(BUILD_DIR)/test_steppps_framework
$(BUILD_DIR)/test_steppps_framework: $(TEST_DIR)/test_steppps_framework.c $(STEPPPS_SRCS) $(HW_DETECTOR_SRCS) | directories
	@echo "🔨 Building STEPPPS Framework Test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# STEPPPS Entity Framework Demo (uses host compiler)
STEPPPS_ENTITY_SRCS := src/core/steppps/steppps_entity.c
steppps-entity-demo: build/steppps_entity_demo
build/steppps_entity_demo: demos/steppps_entity_demo.c $(STEPPPS_ENTITY_SRCS) | directories
	@echo "🔨 Building STEPPPS Entity Framework Demo..."
	gcc -DHOST_BUILD -Wall -Wextra -O2 -g -Iinclude -o $@ $^
	@echo "✅ Run with: ./build/steppps_entity_demo"

# Memory Manager Test
test_memory_manager: $(BUILD_DIR)/test_memory_manager
$(BUILD_DIR)/test_memory_manager: $(TEST_DIR)/test_memory_manager.c $(MEMORY_SRCS) $(BOOT_SRCS) $(HW_DETECTOR_SRCS) | directories
	@echo "🔨 Building Memory Manager Test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# IoC Container Test
test_ioc_container: $(BUILD_DIR)/test_ioc_container
$(BUILD_DIR)/test_ioc_container: $(TEST_DIR)/test_ioc_container.c $(IOC_SRCS) | directories
	@echo "🔨 Building IoC Container Test..."
	$(CC) $(CFLAGS) $(INCLUDES) -lpthread -o $@ $^

# DI Bootstrap Test
test_di_bootstrap: $(BUILD_DIR)/test_di_bootstrap
$(BUILD_DIR)/test_di_bootstrap: $(TEST_DIR)/test_di_bootstrap.c $(ALL_DI_SRCS) | directories
	@echo "🔨 Building DI Bootstrap Test..."
	$(CC) $(CFLAGS) $(INCLUDES) -lpthread -o $@ $^

# Run all tests
test: $(TESTS)
	@echo ""
	@echo "🧪 RUNNING TERNARYBIT OS COMPONENT TESTS"
	@echo "========================================"
	@echo ""
	@echo "Testing Hardware Detection System..."
	@./$(BUILD_DIR)/test_hardware_detector
	@echo ""
	@echo "Testing Universal Boot System..."
	@./$(BUILD_DIR)/test_universal_boot
	@echo ""
	@echo "Testing STEPPPS Framework..."
	@./$(BUILD_DIR)/test_steppps_framework
	@echo ""
	@echo "Testing Memory Management System..."
	@./$(BUILD_DIR)/test_memory_manager
	@echo ""
	@echo "Testing IoC Container..."
	@./$(BUILD_DIR)/test_ioc_container
	@echo ""
	@echo "Testing DI Bootstrap..."
	@./$(BUILD_DIR)/test_di_bootstrap
	@echo ""
	@echo "🎯 All component tests completed!"

# Run individual tests
test-hardware: $(BUILD_DIR)/test_hardware_detector
	@echo "🧪 Testing Hardware Detection System..."
	@./$(BUILD_DIR)/test_hardware_detector

test-boot: $(BUILD_DIR)/test_universal_boot
	@echo "🧪 Testing Universal Boot System..."
	@./$(BUILD_DIR)/test_universal_boot

# Test targets
.PHONY: test test-unit test-integration test-coverage

test: ## Run all tests (unit and integration)
	@echo "🚀 Running all tests..."
	@./scripts/run-tests.sh --type all

test-unit: ## Run unit tests only
	@echo "🔍 Running unit tests..."
	@./scripts/run-tests.sh --type unit

test-integration: ## Run integration tests only
	@echo "🔗 Running integration tests..."
	@./scripts/run-tests.sh --type integration

test-coverage: ## Run tests with coverage report
	@echo "📊 Running tests with coverage..."
	@./scripts/run-tests.sh --coverage

# Legacy test targets for backward compatibility
test-steppps: test-unit
	@# Alias for running STEPPPS framework tests

# Helper target to list all available test targets
test-help:
	@echo "Available test targets:"
	@echo "  test           - Run all tests (unit and integration)"
	@echo "  test-unit      - Run unit tests only"
	@echo "  test-integration - Run integration tests only"
	@echo "  test-coverage  - Run tests with coverage report"
	@echo "  test-steppps   - Run STEPPPS framework tests"

# Include test files in the build
$(BUILD_DIR)/tests/test_%: $(TEST_DIR)/unit/test_%.c
	@mkdir -p $(@D)
	@echo "🔨 Building test: $@"
	@$(CC) $(CFLAGS) -Iinclude -Itests/unit/mocks -o $@ $< $(LDFLAGS)

# Uninstallation targets
.PHONY: uninstall uninstall-windows uninstall-linux uninstall-help

uninstall: uninstall-help
	@echo "Please specify a target: make uninstall-windows or make uninstall-linux"

uninstall-windows:
	@echo "🚀 Uninstalling TernaryBit OS from Windows..."
	@if [ -f "deploy/windows/uninstall-tbos-windows.ps1" ]; then \
		powershell -ExecutionPolicy Bypass -File "deploy/windows/uninstall-tbos-windows.ps1"; \
	else \
		echo "Error: Windows uninstall script not found."; \
		exit 1; \
	fi

uninstall-linux:
	@echo "🚀 Uninstalling TernaryBit OS from Linux/Unix..."
	@if [ -f "scripts/uninstall-tbos.sh" ]; then \
		sh "scripts/uninstall-tbos.sh"; \
	else \
		echo "Error: Linux uninstall script not found."; \
		exit 1; \
	fi

uninstall-help:
	@echo "Uninstall targets:"
	@echo "  uninstall-windows  - Remove TernaryBit OS from Windows"
	@echo "  uninstall-linux    - Remove TernaryBit OS from Linux/Unix"
	@echo "  uninstall-help     - Show this help message"

test-memory: $(BUILD_DIR)/test_memory_manager
	@echo "🧪 Testing Memory Management System..."
	@./$(BUILD_DIR)/test_memory_manager

test-ioc: $(BUILD_DIR)/test_ioc_container
	@echo "🧪 Testing IoC Container..."
	@./$(BUILD_DIR)/test_ioc_container

test-di: $(BUILD_DIR)/test_di_bootstrap
	@echo "🧪 Testing DI Bootstrap..."
	@./$(BUILD_DIR)/test_di_bootstrap

# Run all DI-related tests
test-di-all: test-ioc test-di

# Interactive Shell
$(SHELL_MAIN): $(BUILD_DIR)/$(SHELL_MAIN)
$(BUILD_DIR)/$(SHELL_MAIN): $(SRC_DIR)/shell/shell_main.c $(ALL_CORE_SRCS) | directories
	@echo "🔨 Building TernaryBit OS Interactive Shell..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Run interactive shell
shell: $(BUILD_DIR)/$(SHELL_MAIN)
	@echo "🚀 Starting TernaryBit OS Interactive Shell..."
	@./$(BUILD_DIR)/$(SHELL_MAIN)

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

# Development targets
dev: all test

.PHONY: build-ci
build-ci:
	@echo "🏭 Running CI universal builds (universal/minimal/calc profiles)..."
	@./scripts/ci_build_universal.sh

# Install (placeholder)
install:
	@echo "📦 Install target not yet implemented"

# Help
help:
	@echo "TernaryBit Universal OS - Development Targets"
	@echo "============================================"
	@echo "all          - Build all components"
	@echo "drivers      - Build scaffolding objects (drivers/fs/net/shell)"
	@echo "drivers-host - Build scaffolding with host GCC (no cross toolchain)"
	@echo "test         - Run all tests"
	@echo "test-hardware - Test hardware detection only"
	@echo "test-boot    - Test boot system only"
	@echo "test-steppps - Test STEPPPS framework only"
	@echo "test-memory  - Test memory management only"
	@echo "shell        - Start interactive shell"
	@echo "clean        - Clean build artifacts"
	@echo "dev          - Build and test (development cycle)"
	@echo "help         - Show this help"

.PHONY: all test test-hardware test-boot test-steppps test-memory shell clean dev install help directories
.PHONY: test-network-host
test-network-host:
	@echo "🧪 Building Network module host test..."
	@mkdir -p build
	@gcc -Wall -Wextra -g -DHOST_BUILD -Iinclude -Isrc -Isrc/core -Isrc/network/core \
		tests/unit/test_network_module.c \
		src/network/core/tbos_network.c \
		-o build/host_network_test
	@echo "🏃 Running Network module host test..."
	@./build/host_network_test

.PHONY: test-pxfs-host
test-pxfs-host:
	@echo "🧪 Building PXFS codec host test..."
	@gcc -Wall -Wextra -g -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_pxfs_codec.c \
		src/core/filesystem/pxfs_codec.c \
		-o build/host_pxfs_codec_test
	@echo "🏃 Running PXFS codec host test..."
	@./build/host_pxfs_codec_test

.PHONY: test-pxfs-overlay-host
test-pxfs-overlay-host:
	@echo "🧪 Building PXFS overlay host test..."
	@gcc -Wall -Wextra -g -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_pxfs_overlay.c \
		src/core/filesystem/pxfs_codec.c \
		src/core/filesystem/pxfs_overlay.c \
		kernel/fs/vfs.c \
		kernel/fs/ramfs.c \
		-o build/host_pxfs_overlay_test
	@echo "🏃 Running PXFS overlay host test..."
	@./build/host_pxfs_overlay_test

.PHONY: test-rf2s-host
test-rf2s-host:
	@echo "🧪 Building RF2S codec host test..."
	@gcc -Wall -Wextra -g -DHOST_BUILD -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_rf2s_codec.c \
		src/core/filesystem/rf2s_codec.c \
		-o build/host_rf2s_codec_test
	@echo "🏃 Running RF2S codec host test..."
	@./build/host_rf2s_codec_test

.PHONY: test-rf2s-driver-host
test-rf2s-driver-host:
	@echo "🧪 Building RF2S driver host test..."
	@gcc -Wall -Wextra -g -DHOST_BUILD -Iinclude -Isrc -Isrc/core -Isrc/boot -Isrc/shell \
		tests/unit/test_rf2s_driver.c \
		src/core/filesystem/rf2s_codec.c \
		kernel/fs/rf2s_driver.c \
		kernel/fs/vfs.c \
		kernel/fs/ramfs.c \
		-o build/host_rf2s_driver_test
	@echo "🏃 Running RF2S driver host test..."
	@./build/host_rf2s_driver_test

# V4.0 Universal Boot Targets
.PHONY: test-bcb-host build-hosted-linux test-hosted-linux

test-bcb-host:
	@echo "🧪 Building BCB host test..."
	@mkdir -p build/v4
	@gcc -Wall -Wextra -g -DHOST_BUILD -Iinclude \
		tests/v4/test_bcb.c \
		-o build/v4/test_bcb
	@echo "🏃 Running BCB host test..."
	@./build/v4/test_bcb

build-hosted-linux:
	@echo "🔧 Building hosted Linux target..."
	@chmod +x hosted/build_hosted_linux.sh
	@./hosted/build_hosted_linux.sh

test-hosted-linux: build-hosted-linux
	@echo "🧪 Testing hosted Linux target..."
	@echo "bcb\nmemory\nexit" | ./artifacts/hosted/tbos-linux

.PHONY: persona-help list-personas list-packages persona-build persona-test persona-run persona-ci

persona-help:
	@echo "Persona-aware workflow"
	@echo "  make list-personas                 # show available persona configs"
	@echo "  make persona-build PERSONA=name    # build persona binary"
	@echo "  make persona-test PERSONA=name     # run persona smoke tests"
	@echo "  make persona-run PERSONA=name      # launch persona in QEMU"
	@echo "  make persona-ci [PERSONAS=\"...\"]  # run persona CI matrix (defaults to all)"
	@echo ""
	@echo "Optional: PACKAGES=\"pkg1 pkg2\" to include config/packages overlays"

list-personas:
	@./build_tbos_system.sh --list-personas

list-packages:
	@ls $(TBOS_PACKAGE_DIR)/*.mk 2>/dev/null | sed 's#$(TBOS_PACKAGE_DIR)/##;s#.mk##'

persona-build:
	@./build_tbos_system.sh --persona $(PERSONA)

persona-test:
	@./test_tbos.sh --persona $(PERSONA)

persona-run:
	@./scripts/run_persona_qemu.sh --persona $(PERSONA)

persona-ci:
	@PACKAGES="$(PACKAGES)" ./scripts/tbos_persona_ci.sh $(PERSONAS)

# ═══════════════════════════════════════════════════════════════════════════
# MULTI-ARCHITECTURE BUILD TARGETS
# ═══════════════════════════════════════════════════════════════════════════

.PHONY: arch-info arch-all arch-host arch-x86_64 arch-arm64 arch-riscv64 arch-test arch-clean

# Detect host architecture
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
    TBOS_HOST_OS := macos
else ifeq ($(UNAME_S),Linux)
    TBOS_HOST_OS := linux
else
    TBOS_HOST_OS := unknown
endif

ifeq ($(UNAME_M),x86_64)
    TBOS_HOST_ARCH := x86_64
else ifeq ($(UNAME_M),arm64)
    TBOS_HOST_ARCH := arm64
else ifeq ($(UNAME_M),aarch64)
    TBOS_HOST_ARCH := arm64
else
    TBOS_HOST_ARCH := $(UNAME_M)
endif

# Cross-compiler detection
X86_64_ELF_CC := $(shell which x86_64-elf-gcc 2>/dev/null)
ARM64_ELF_CC := $(shell which aarch64-elf-gcc 2>/dev/null || which aarch64-linux-gnu-gcc 2>/dev/null)
RISCV64_ELF_CC := $(shell which riscv64-elf-gcc 2>/dev/null || which riscv64-linux-gnu-gcc 2>/dev/null)

# Common flags
TBOS_COMMON_FLAGS := -Wall -Wextra -Iinclude -DTBOS_HOSTED -DHOST_BUILD
TBOS_DEBUG_FLAGS := -g -O0 -DDEBUG
TBOS_RELEASE_FLAGS := -O2 -DNDEBUG

# Shell sources for multi-arch build
TBOS_SHELL_SRCS := demos/tbos_mini_shell.c
TBOS_HYBRID_SRCS := src/shell/universal/tbos_hybrid_shell.c

# Network sources
TBOS_NET_SRCS := \
    kernel/hal_hosted.c \
    src/network/tbos_net.c \
    src/network/dns/tbos_dns.c \
    src/network/wow/tbos_wow.c \
    src/network/transport/udp/tbos_udp.c

arch-info:
	@echo ""
	@echo "================================================================"
	@echo "  TBOS Multi-Architecture Build Info"
	@echo "================================================================"
	@echo ""
	@echo "Host System:"
	@echo "  OS:   $(TBOS_HOST_OS)"
	@echo "  Arch: $(TBOS_HOST_ARCH)"
	@echo ""
	@echo "Available Cross-Compilers:"
	@echo "  Host GCC:       $(shell which gcc)"
ifeq ($(X86_64_ELF_CC),)
	@echo "  x86_64-elf-gcc: NOT FOUND"
else
	@echo "  x86_64-elf-gcc: $(X86_64_ELF_CC)"
endif
ifeq ($(ARM64_ELF_CC),)
	@echo "  aarch64-gcc:    NOT FOUND"
else
	@echo "  aarch64-gcc:    $(ARM64_ELF_CC)"
endif
ifeq ($(RISCV64_ELF_CC),)
	@echo "  riscv64-gcc:    NOT FOUND"
else
	@echo "  riscv64-gcc:    $(RISCV64_ELF_CC)"
endif
	@echo ""
	@echo "To install cross-compilers:"
	@echo "  macOS:  brew install x86_64-elf-gcc aarch64-elf-gcc"
	@echo "  Ubuntu: apt install gcc-aarch64-linux-gnu gcc-riscv64-linux-gnu"
	@echo "================================================================"
	@echo ""

# Build for all available architectures
arch-all: arch-info arch-host
	@echo ""
	@echo "Building for all available architectures..."
ifneq ($(X86_64_ELF_CC),)
	@$(MAKE) arch-x86_64
endif
ifneq ($(ARM64_ELF_CC),)
	@$(MAKE) arch-arm64
endif
ifneq ($(RISCV64_ELF_CC),)
	@$(MAKE) arch-riscv64
endif
	@echo ""
	@echo "================================================================"
	@echo "  Multi-arch build complete!"
	@echo "  Outputs in build/<arch>/"
	@echo "================================================================"

# Host architecture build
arch-host: build/$(TBOS_HOST_ARCH)/tbos_shell build/$(TBOS_HOST_ARCH)/tbos_hybrid build/$(TBOS_HOST_ARCH)/test_net
	@echo "[$(TBOS_HOST_ARCH)] Host build complete"

build/$(TBOS_HOST_ARCH)/tbos_shell: $(TBOS_SHELL_SRCS)
	@mkdir -p build/$(TBOS_HOST_ARCH)
	@echo "[$(TBOS_HOST_ARCH)] Building tbos_shell..."
	@gcc $(TBOS_COMMON_FLAGS) $(TBOS_DEBUG_FLAGS) -o $@ $<

build/$(TBOS_HOST_ARCH)/tbos_hybrid: $(TBOS_HYBRID_SRCS)
	@mkdir -p build/$(TBOS_HOST_ARCH)
	@echo "[$(TBOS_HOST_ARCH)] Building tbos_hybrid..."
	@gcc $(TBOS_COMMON_FLAGS) $(TBOS_DEBUG_FLAGS) -o $@ $<

build/$(TBOS_HOST_ARCH)/test_net: tests/test_net_quick.c $(TBOS_NET_SRCS)
	@mkdir -p build/$(TBOS_HOST_ARCH)
	@echo "[$(TBOS_HOST_ARCH)] Building test_net..."
	@gcc $(TBOS_COMMON_FLAGS) $(TBOS_DEBUG_FLAGS) -Isrc/network -o $@ $^

# x86_64 cross-compile (use native gcc for hosted builds since elf-gcc lacks libc)
arch-x86_64: build/x86_64/tbos_shell build/x86_64/tbos_hybrid
	@echo "[x86_64] Build complete"

build/x86_64/tbos_shell: $(TBOS_SHELL_SRCS)
ifeq ($(TBOS_HOST_ARCH),x86_64)
	@mkdir -p build/x86_64
	@echo "[x86_64] Building tbos_shell (native)..."
	@gcc $(TBOS_COMMON_FLAGS) -o $@ $<
else ifneq ($(X86_64_ELF_CC),)
	@mkdir -p build/x86_64
	@echo "[x86_64] Building tbos_shell (cross)..."
	@$(X86_64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[x86_64] Skipped (needs hosted libc)"
else
	@echo "[x86_64] Cross-compiler not found, skipping"
endif

build/x86_64/tbos_hybrid: $(TBOS_HYBRID_SRCS)
ifeq ($(TBOS_HOST_ARCH),x86_64)
	@mkdir -p build/x86_64
	@echo "[x86_64] Building tbos_hybrid (native)..."
	@gcc $(TBOS_COMMON_FLAGS) -o $@ $<
else ifneq ($(X86_64_ELF_CC),)
	@mkdir -p build/x86_64
	@echo "[x86_64] Building tbos_hybrid (cross)..."
	@$(X86_64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[x86_64] Skipped (needs hosted libc)"
else
	@echo "[x86_64] Cross-compiler not found, skipping"
endif

# ARM64 cross-compile
arch-arm64: build/arm64/tbos_shell build/arm64/tbos_hybrid
	@echo "[arm64] Build complete"

build/arm64/tbos_shell: $(TBOS_SHELL_SRCS)
ifneq ($(ARM64_ELF_CC),)
	@mkdir -p build/arm64
	@echo "[arm64] Building tbos_shell..."
	@$(ARM64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[arm64] Skipped (needs hosted libc)"
else ifeq ($(TBOS_HOST_ARCH),arm64)
	@mkdir -p build/arm64
	@echo "[arm64] Building tbos_shell (native)..."
	@gcc $(TBOS_COMMON_FLAGS) -o $@ $<
else
	@echo "[arm64] Cross-compiler not found, skipping"
endif

build/arm64/tbos_hybrid: $(TBOS_HYBRID_SRCS)
ifneq ($(ARM64_ELF_CC),)
	@mkdir -p build/arm64
	@echo "[arm64] Building tbos_hybrid..."
	@$(ARM64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[arm64] Skipped (needs hosted libc)"
else ifeq ($(TBOS_HOST_ARCH),arm64)
	@mkdir -p build/arm64
	@echo "[arm64] Building tbos_hybrid (native)..."
	@gcc $(TBOS_COMMON_FLAGS) -o $@ $<
else
	@echo "[arm64] Cross-compiler not found, skipping"
endif

# RISC-V 64 cross-compile
arch-riscv64: build/riscv64/tbos_shell build/riscv64/tbos_hybrid
	@echo "[riscv64] Build complete"

build/riscv64/tbos_shell: $(TBOS_SHELL_SRCS)
ifneq ($(RISCV64_ELF_CC),)
	@mkdir -p build/riscv64
	@echo "[riscv64] Building tbos_shell..."
	@$(RISCV64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[riscv64] Skipped (needs hosted libc)"
else
	@echo "[riscv64] Cross-compiler not found, skipping"
endif

build/riscv64/tbos_hybrid: $(TBOS_HYBRID_SRCS)
ifneq ($(RISCV64_ELF_CC),)
	@mkdir -p build/riscv64
	@echo "[riscv64] Building tbos_hybrid..."
	@$(RISCV64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[riscv64] Skipped (needs hosted libc)"
else
	@echo "[riscv64] Cross-compiler not found, skipping"
endif

# Run tests on host architecture
arch-test: arch-host
	@echo ""
	@echo "================================================================"
	@echo "  Running tests on $(TBOS_HOST_ARCH)..."
	@echo "================================================================"
	@build/$(TBOS_HOST_ARCH)/test_net

# Clean multi-arch builds
arch-clean:
	@echo "Cleaning multi-arch builds..."
	@rm -rf build/x86_64 build/arm64 build/riscv64 build/$(TBOS_HOST_ARCH)
	@echo "Clean complete."

# Docker-based cross-compilation (for full cross-platform testing)
.PHONY: docker-build-all docker-test-arm64 docker-test-riscv64

docker-build-all:
	@echo "Building TBOS in Docker for multiple architectures..."
	@docker run --rm -v $(PWD):/src -w /src --platform linux/amd64 gcc:latest make arch-host TBOS_HOST_ARCH=x86_64 || true
	@docker run --rm -v $(PWD):/src -w /src --platform linux/arm64 gcc:latest make arch-host TBOS_HOST_ARCH=arm64 || true
	@echo "Docker builds complete. Check build/ for outputs."

docker-test-arm64:
	@echo "Testing TBOS on ARM64 via Docker..."
	@docker run --rm -v $(PWD):/src -w /src --platform linux/arm64 gcc:latest sh -c "make arch-host TBOS_HOST_ARCH=arm64 && build/arm64/test_net"

docker-test-riscv64:
	@echo "Testing TBOS on RISC-V64 via QEMU user mode..."
	@docker run --rm -v $(PWD):/src -w /src riscv64/debian:latest sh -c "apt-get update && apt-get install -y gcc && make arch-host TBOS_HOST_ARCH=riscv64 && build/riscv64/test_net" || echo "RISC-V Docker not available"

# ═══════════════════════════════════════════════════════════════════════════
# HARDWARE TIER BUILDS (4-bit to Supercomputer)
# ═══════════════════════════════════════════════════════════════════════════

.PHONY: tier-all tier-calc tier-embed tier-retro tier-desktop tier-server tier-super tier-info tier-test
.PHONY: tier-shell-all tier-shell-calc tier-shell-embed tier-shell-retro tier-shell-desktop tier-shell-server tier-shell-super

TIER_BUILD_DIR := build/tiers
TIER_DEMO_SRC := demos/tbos_universal_demo.c
TIER_SHELL_SRC := src/shell/tbos_tier_shell.c

tier-info:
	@echo ""
	@echo "================================================================"
	@echo "  TBOS Hardware Tier Build System"
	@echo "  From 4-bit Calculators to Supercomputers"
	@echo "================================================================"
	@echo ""
	@echo "  Tier 0: Calculator   (4-8 bit,  256B-4KB RAM)   5 commands"
	@echo "  Tier 1: Embedded     (16-bit,   4KB-64KB RAM)  15 commands"
	@echo "  Tier 2: Retro        (32-bit,   64KB-16MB RAM) 25 commands"
	@echo "  Tier 3: Desktop      (64-bit,   16MB-16GB RAM) 35 commands"
	@echo "  Tier 4: Server       (64-bit,   16GB-1TB RAM)  40 commands"
	@echo "  Tier 5: Supercomputer (64-bit+, 1TB+ distributed) 45 commands"
	@echo ""
	@echo "  Demo targets (display tier info):"
	@echo "    make tier-all        Build all tier demos"
	@echo "    make tier-calc       Run calculator demo"
	@echo "    make tier-test       Run all tier demos"
	@echo ""
	@echo "  Shell targets (full interactive shells):"
	@echo "    make tier-shell-all     Build all tier shells"
	@echo "    make tier-shell-calc    Calculator shell (5 cmds)"
	@echo "    make tier-shell-embed   Embedded shell (15 cmds)"
	@echo "    make tier-shell-retro   Retro shell (25 cmds)"
	@echo "    make tier-shell-desktop Desktop shell (35 cmds)"
	@echo "    make tier-shell-server  Server shell (40 cmds)"
	@echo "    make tier-shell-super   Supercomputer shell (45 cmds)"
	@echo "================================================================"
	@echo ""

tier-all: $(TIER_BUILD_DIR)/tbos_calc $(TIER_BUILD_DIR)/tbos_embed $(TIER_BUILD_DIR)/tbos_retro $(TIER_BUILD_DIR)/tbos_desktop $(TIER_BUILD_DIR)/tbos_server $(TIER_BUILD_DIR)/tbos_super
	@echo ""
	@echo "All hardware tiers built:"
	@ls -la $(TIER_BUILD_DIR)/

$(TIER_BUILD_DIR):
	@mkdir -p $(TIER_BUILD_DIR)

$(TIER_BUILD_DIR)/tbos_calc: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 0] Building Calculator (8-bit, 256B RAM)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=0 -Wall -Wextra -Iinclude -o $@ $< 2>/dev/null || gcc -DTBOS_HOSTED -DTBOS_TIER=0 -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/tbos_embed: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 1] Building Embedded (16-bit, 4KB RAM)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=1 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/tbos_retro: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 2] Building Retro (32-bit, 64KB RAM)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=2 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/tbos_desktop: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 3] Building Desktop (64-bit, 1MB RAM)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=3 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/tbos_server: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 4] Building Server (64-bit SMP, 4MB RAM)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=4 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/tbos_super: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 5] Building Supercomputer (64-bit distributed, 16MB RAM)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=5 -Wall -Wextra -Iinclude -o $@ $<

tier-calc: $(TIER_BUILD_DIR)/tbos_calc
	@$(TIER_BUILD_DIR)/tbos_calc

tier-embed: $(TIER_BUILD_DIR)/tbos_embed
	@$(TIER_BUILD_DIR)/tbos_embed

tier-retro: $(TIER_BUILD_DIR)/tbos_retro
	@$(TIER_BUILD_DIR)/tbos_retro

tier-desktop: $(TIER_BUILD_DIR)/tbos_desktop
	@$(TIER_BUILD_DIR)/tbos_desktop

tier-server: $(TIER_BUILD_DIR)/tbos_server
	@$(TIER_BUILD_DIR)/tbos_server

tier-super: $(TIER_BUILD_DIR)/tbos_super
	@$(TIER_BUILD_DIR)/tbos_super

# ─────────────────────────────────────────────────────────────────────────────
# TIER-AWARE SHELLS (with actual commands, not just demos)
# ─────────────────────────────────────────────────────────────────────────────

tier-shell-all: $(TIER_BUILD_DIR)/shell_calc $(TIER_BUILD_DIR)/shell_embed $(TIER_BUILD_DIR)/shell_retro $(TIER_BUILD_DIR)/shell_desktop $(TIER_BUILD_DIR)/shell_server $(TIER_BUILD_DIR)/shell_super
	@echo ""
	@echo "All tier shells built:"
	@ls -la $(TIER_BUILD_DIR)/shell_*
	@echo ""
	@echo "Commands per tier:"
	@echo "  shell_calc    (Tier 0):  5 commands  - help, echo, calc, karma, exit"
	@echo "  shell_embed   (Tier 1): 15 commands  - + ls, cd, cat, mkdir, rm, etc."
	@echo "  shell_retro   (Tier 2): 25 commands  - + head, grep, wc, history, etc."
	@echo "  shell_desktop (Tier 3): 35 commands  - + meditate, sangha, dharma, ping"
	@echo "  shell_server  (Tier 4): 40 commands  - + top, netstat, free, lscpu"
	@echo "  shell_super   (Tier 5): 45 commands  - + cluster, consciousness, quantum"

$(TIER_BUILD_DIR)/shell_calc: $(TIER_SHELL_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 0] Building Calculator Shell (5 commands)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=0 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/shell_embed: $(TIER_SHELL_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 1] Building Embedded Shell (15 commands)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=1 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/shell_retro: $(TIER_SHELL_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 2] Building Retro Shell (25 commands)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=2 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/shell_desktop: $(TIER_SHELL_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 3] Building Desktop Shell (35 commands)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=3 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/shell_server: $(TIER_SHELL_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 4] Building Server Shell (40 commands)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=4 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/shell_super: $(TIER_SHELL_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 5] Building Supercomputer Shell (45 commands)..."
	@gcc -DTBOS_HOSTED -DTBOS_TIER=5 -Wall -Wextra -Iinclude -o $@ $<

# Run tier shells interactively
tier-shell-calc: $(TIER_BUILD_DIR)/shell_calc
	@$(TIER_BUILD_DIR)/shell_calc

tier-shell-embed: $(TIER_BUILD_DIR)/shell_embed
	@$(TIER_BUILD_DIR)/shell_embed

tier-shell-retro: $(TIER_BUILD_DIR)/shell_retro
	@$(TIER_BUILD_DIR)/shell_retro

tier-shell-desktop: $(TIER_BUILD_DIR)/shell_desktop
	@$(TIER_BUILD_DIR)/shell_desktop

tier-shell-server: $(TIER_BUILD_DIR)/shell_server
	@$(TIER_BUILD_DIR)/shell_server

tier-shell-super: $(TIER_BUILD_DIR)/shell_super
	@$(TIER_BUILD_DIR)/shell_super

# ═══════════════════════════════════════════════════════════════════════════
# TBOS COMPLETE SHELL (runs on any OS)
# ═══════════════════════════════════════════════════════════════════════════

.PHONY: tbos tbos-steppps tbos-install tbos-run tbos-clean

# Simple TBOS shell (no STEPPPS integration)
tbos: build/tbos
	@echo ""
	@echo "TBOS Complete Shell built: build/tbos"
	@echo "Run with: ./build/tbos"

build/tbos: src/shell/tbos_complete_shell.c
	@mkdir -p build
	@echo "Building TBOS Complete Shell..."
	@gcc -DTBOS_HOSTED -Wall -Wextra -O2 -o $@ $< -Iinclude
	@echo "Build complete!"

# TBOS shell with full STEPPPS integration (VFS, PXFS, security)
STEPPPS_VFS_SRCS := kernel/fs/steppps_vfs.c fs/pxfs_steppps.c
STEPPPS_V2_SRCS := steppps/v2/steppps_cli.c steppps/v2/steppps_runtime.c

tbos-steppps: build/tbos_steppps build/steppps build/steppps_fs_demo
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS + STEPPPS Build Complete                                ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Binaries:                                                    ║"
	@echo "║    build/tbos_steppps    - TBOS shell with STEPPPS VFS        ║"
	@echo "║    build/steppps         - STEPPPS v2 CLI                     ║"
	@echo "║    build/steppps_fs_demo - Filesystem integration demo        ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Install to ~/.tbos/bin:  make tbos-steppps-install           ║"
	@echo "║  Run shell:               ./build/tbos_steppps                ║"
	@echo "║  Run demo:                ./build/steppps_fs_demo             ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

build/tbos_steppps: src/shell/tbos_complete_shell.c $(STEPPPS_VFS_SRCS)
	@mkdir -p build
	@echo "Building TBOS + STEPPPS shell..."
	@gcc -DHOST_BUILD -Wall -Wextra -O2 -o $@ $^ -I. -Iinclude
	@echo "TBOS + STEPPPS shell built!"

build/steppps: $(STEPPPS_V2_SRCS)
	@mkdir -p build
	@echo "Building STEPPPS v2 CLI..."
	@gcc -DHOST_BUILD -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES)
	@echo "STEPPPS CLI built!"

build/steppps_fs_demo: demos/steppps_fs_demo.c $(STEPPPS_VFS_SRCS)
	@mkdir -p build
	@echo "Building STEPPPS filesystem demo..."
	@gcc -DHOST_BUILD -Wall -O2 -o $@ $^ -I. -Iinclude
	@echo "STEPPPS filesystem demo built!"

tbos-steppps-install: tbos-steppps
	@mkdir -p ~/.tbos/bin
	@cp build/tbos_steppps ~/.tbos/bin/tbos
	@cp build/steppps ~/.tbos/bin/steppps
	@echo ""
	@echo "Installed to ~/.tbos/bin/"
	@echo "Add to PATH: export PATH=\$$PATH:~/.tbos/bin"

tbos-install: build/tbos
	@echo "Installing TBOS to /usr/local/bin..."
	@sudo cp build/tbos /usr/local/bin/tbos
	@echo "TBOS installed! Run 'tbos' from anywhere."

tbos-run: build/tbos
	@./build/tbos

tbos-clean:
	@rm -f build/tbos build/tbos_steppps build/steppps build/steppps_fs_demo
	@rm -f build/steppps_entity_demo
	@rm -rf build/windows build/android build/ios
	@echo "TBOS build artifacts cleaned."

# ═══════════════════════════════════════════════════════════════════════════
# CROSS-PLATFORM BUILDS (Windows, Android, iOS)
# ═══════════════════════════════════════════════════════════════════════════

.PHONY: tbos-windows tbos-android tbos-ios tbos-macos tbos-all-platforms
.PHONY: android ios windows macos  # Shorthand aliases

# Shorthand aliases
android: tbos-android
ios: tbos-ios
windows: tbos-windows
macos: tbos-macos

# Common source files for cross-platform builds
TBOS_CROSS_SRCS := src/shell/tbos_complete_shell.c
TBOS_CROSS_STEPPPS_SRCS := kernel/fs/steppps_vfs.c fs/pxfs_steppps.c
TBOS_CROSS_INCLUDES := -I. -Iinclude

# ─────────────────────────────────────────────────────────────────────────────
# MACOS (Native build)
# ─────────────────────────────────────────────────────────────────────────────

tbos-macos: build/macos/tbos build/macos/steppps
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS macOS Build Complete                                    ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  build/macos/tbos      - TBOS shell                           ║"
	@echo "║  build/macos/steppps   - STEPPPS CLI                          ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Run: ./build/macos/tbos                                      ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

build/macos/tbos: $(TBOS_CROSS_SRCS) $(TBOS_CROSS_STEPPPS_SRCS)
	@mkdir -p build/macos
	@echo "Building TBOS for macOS..."
	@gcc -DHOST_BUILD -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES)
	@echo "macOS build complete!"

build/macos/steppps: $(STEPPPS_V2_SRCS)
	@mkdir -p build/macos
	@echo "Building STEPPPS CLI for macOS..."
	@gcc -DHOST_BUILD -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES)
	@echo "STEPPPS macOS build complete!"

# ─────────────────────────────────────────────────────────────────────────────
# WINDOWS (MinGW-w64 cross-compilation)
# ─────────────────────────────────────────────────────────────────────────────
# Install MinGW: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)

MINGW_CC := x86_64-w64-mingw32-gcc
MINGW_EXISTS := $(shell which $(MINGW_CC) 2>/dev/null)

tbos-windows: build/windows/tbos.exe build/windows/steppps.exe
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS Windows Build Complete                                  ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  build/windows/tbos.exe     - TBOS shell                      ║"
	@echo "║  build/windows/steppps.exe  - STEPPPS CLI                     ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Copy to Windows and run in cmd.exe or PowerShell             ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

build/windows/tbos.exe: $(TBOS_CROSS_SRCS) $(TBOS_CROSS_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with:"
	@echo "  macOS:  brew install mingw-w64"
	@echo "  Linux:  apt install mingw-w64"
	@exit 1
endif
	@mkdir -p build/windows
	@echo "Building TBOS for Windows (x86_64)..."
	@$(MINGW_CC) -DHOST_BUILD -D_WIN32 -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static
	@echo "Windows build complete!"

build/windows/steppps.exe: $(STEPPPS_V2_SRCS)
	@mkdir -p build/windows
	@echo "Building STEPPPS CLI for Windows..."
	@$(MINGW_CC) -DHOST_BUILD -D_WIN32 -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static
	@echo "STEPPPS Windows build complete!"

# ─────────────────────────────────────────────────────────────────────────────
# ANDROID (NDK cross-compilation)
# ─────────────────────────────────────────────────────────────────────────────
# Auto-detect NDK or set ANDROID_NDK_HOME manually
# Common locations: ~/Library/Android/sdk/ndk/*, ~/Android/Sdk/ndk/*

# Try to find NDK automatically (latest version)
ANDROID_SDK_MACOS := $(HOME)/Library/Android/sdk
ANDROID_SDK_LINUX := $(HOME)/Android/Sdk
ANDROID_NDK_AUTO := $(shell ls -d $(ANDROID_SDK_MACOS)/ndk/* 2>/dev/null | sort -V | tail -1)
ANDROID_NDK_AUTO_LINUX := $(shell ls -d $(ANDROID_SDK_LINUX)/ndk/* 2>/dev/null | sort -V | tail -1)

# Use auto-detected or user-specified
ifeq ($(ANDROID_NDK_HOME),)
  ifneq ($(ANDROID_NDK_AUTO),)
    ANDROID_NDK_HOME := $(ANDROID_NDK_AUTO)
  else ifneq ($(ANDROID_NDK_AUTO_LINUX),)
    ANDROID_NDK_HOME := $(ANDROID_NDK_AUTO_LINUX)
  endif
endif

# Android 13 = API 33
ANDROID_API ?= 33
ANDROID_ARCH ?= aarch64
ANDROID_CC := $(ANDROID_NDK_HOME)/toolchains/llvm/prebuilt/darwin-x86_64/bin/$(ANDROID_ARCH)-linux-android$(ANDROID_API)-clang
ANDROID_CC_ALT := $(ANDROID_NDK_HOME)/toolchains/llvm/prebuilt/linux-x86_64/bin/$(ANDROID_ARCH)-linux-android$(ANDROID_API)-clang

tbos-android: build/android/tbos build/android/steppps
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS Android Build Complete                                  ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  build/android/tbos      - TBOS shell ($(ANDROID_ARCH))             ║"
	@echo "║  build/android/steppps   - STEPPPS CLI                        ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Push to device:                                              ║"
	@echo "║    adb push build/android/tbos /data/local/tmp/               ║"
	@echo "║    adb shell /data/local/tmp/tbos                             ║"
	@echo "║  Or use Termux:                                               ║"
	@echo "║    Copy to ~/tbos and run ./tbos                              ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

build/android/tbos: $(TBOS_CROSS_SRCS) $(TBOS_CROSS_STEPPPS_SRCS)
	@mkdir -p build/android
	@if [ -f "$(ANDROID_CC)" ]; then \
		echo "Building TBOS for Android ($(ANDROID_ARCH), API $(ANDROID_API))..."; \
		$(ANDROID_CC) -DHOST_BUILD -D__ANDROID__ -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static-libgcc; \
	elif [ -f "$(ANDROID_CC_ALT)" ]; then \
		echo "Building TBOS for Android ($(ANDROID_ARCH), API $(ANDROID_API))..."; \
		$(ANDROID_CC_ALT) -DHOST_BUILD -D__ANDROID__ -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static-libgcc; \
	else \
		echo "Error: Android NDK not found"; \
		echo ""; \
		echo "Install NDK via Android Studio:"; \
		echo "  Android Studio → Preferences → SDK Manager → SDK Tools"; \
		echo "  Check 'NDK (Side by side)' and click Apply"; \
		echo ""; \
		echo "Or via command line:"; \
		echo "  sdkmanager --install 'ndk;25.2.9519653'"; \
		echo ""; \
		echo "Or set manually:"; \
		echo "  export ANDROID_NDK_HOME=/path/to/ndk"; \
		echo ""; \
		echo "Alternative: Build directly ON Android using Termux:"; \
		echo "  pkg install clang"; \
		echo "  make tbos-termux"; \
		exit 1; \
	fi

build/android/steppps: $(STEPPPS_V2_SRCS)
	@mkdir -p build/android
	@if [ -f "$(ANDROID_CC)" ]; then \
		$(ANDROID_CC) -DHOST_BUILD -D__ANDROID__ -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static-libgcc; \
	elif [ -f "$(ANDROID_CC_ALT)" ]; then \
		$(ANDROID_CC_ALT) -DHOST_BUILD -D__ANDROID__ -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static-libgcc; \
	fi

# Termux build (run this ON Android in Termux)
tbos-termux:
	@echo "Building TBOS for Termux..."
	@mkdir -p build/termux
	clang -DHOST_BUILD -D__ANDROID__ -Wall -O2 \
		-o build/termux/tbos $(TBOS_CROSS_SRCS) $(TBOS_CROSS_STEPPPS_SRCS) $(TBOS_CROSS_INCLUDES)
	clang -DHOST_BUILD -D__ANDROID__ -Wall -O2 \
		-o build/termux/steppps $(STEPPPS_V2_SRCS) $(TBOS_CROSS_INCLUDES)
	@echo "Termux build complete! Run: ./build/termux/tbos"

# ─────────────────────────────────────────────────────────────────────────────
# iOS (Limited - library only, no shell execution)
# ─────────────────────────────────────────────────────────────────────────────
# Note: iOS apps cannot execute system() or run shell commands.
# This builds a library that can be integrated into an iOS app.

IOS_SDK := $(shell xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
IOS_CC := $(shell xcrun --sdk iphoneos --find clang 2>/dev/null)
IOS_ARCH ?= arm64
IOS_MIN_VERSION ?= 14.0

tbos-ios: build/ios/libtbos.a
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS iOS Library Built                                       ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  build/ios/libtbos.a  - Static library for iOS apps           ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Note: iOS cannot run shell commands (sandboxed).             ║"
	@echo "║  Integrate libtbos.a into your Xcode project.                 ║"
	@echo "║  Use TBOS APIs: steppps_*, pxfs_*, karma functions.           ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

build/ios/libtbos.a: build/ios/steppps_vfs.o build/ios/pxfs_steppps.o build/ios/steppps_runtime.o
	@mkdir -p build/ios
	@echo "Creating iOS static library..."
	@ar rcs $@ $^
	@echo "iOS library created!"

build/ios/steppps_vfs.o: kernel/fs/steppps_vfs.c
ifndef IOS_SDK
	@echo "Error: iOS SDK not found. Install Xcode."
	@exit 1
endif
	@mkdir -p build/ios
	@echo "Compiling steppps_vfs.c for iOS..."
	@$(IOS_CC) -isysroot $(IOS_SDK) -arch $(IOS_ARCH) -mios-version-min=$(IOS_MIN_VERSION) \
		-DHOST_BUILD -D__APPLE__ -DTARGET_OS_IPHONE=1 \
		-Wall -O2 -c -o $@ $< $(TBOS_CROSS_INCLUDES)

build/ios/pxfs_steppps.o: fs/pxfs_steppps.c
	@$(IOS_CC) -isysroot $(IOS_SDK) -arch $(IOS_ARCH) -mios-version-min=$(IOS_MIN_VERSION) \
		-DHOST_BUILD -D__APPLE__ -DTARGET_OS_IPHONE=1 \
		-Wall -O2 -c -o $@ $< $(TBOS_CROSS_INCLUDES)

build/ios/steppps_runtime.o: steppps/v2/steppps_runtime.c
	@$(IOS_CC) -isysroot $(IOS_SDK) -arch $(IOS_ARCH) -mios-version-min=$(IOS_MIN_VERSION) \
		-DHOST_BUILD -D__APPLE__ -DTARGET_OS_IPHONE=1 \
		-Wall -O2 -c -o $@ $< $(TBOS_CROSS_INCLUDES)

# ─────────────────────────────────────────────────────────────────────────────
# BUILD ALL PLATFORMS
# ─────────────────────────────────────────────────────────────────────────────

tbos-all-platforms: tbos-steppps
	@echo "Building for all available platforms..."
	@$(MAKE) tbos-windows 2>/dev/null || echo "Windows: Skipped (no MinGW)"
	@$(MAKE) tbos-android 2>/dev/null || echo "Android: Skipped (no NDK)"
	@$(MAKE) tbos-ios 2>/dev/null || echo "iOS: Skipped (no Xcode)"
	@echo ""
	@echo "Cross-platform build complete!"
	@ls -la build/*/tbos* build/*/*.exe build/*/*.a 2>/dev/null || true

# ─────────────────────────────────────────────────────────────────────────────
# HELP
# ─────────────────────────────────────────────────────────────────────────────

tbos-help:
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS Cross-Platform Build System                             ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Native Builds:                                               ║"
	@echo "║    make tbos              - Simple TBOS shell                 ║"
	@echo "║    make tbos-steppps      - TBOS + STEPPPS integration        ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Cross-Platform:                                              ║"
	@echo "║    make tbos-windows      - Windows (requires MinGW)          ║"
	@echo "║    make tbos-android      - Android (requires NDK)            ║"
	@echo "║    make tbos-ios          - iOS library (requires Xcode)      ║"
	@echo "║    make tbos-termux       - Build ON Android in Termux        ║"
	@echo "║    make tbos-all-platforms - Build all available              ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Install:                                                     ║"
	@echo "║    make tbos-steppps-install  - Install to ~/.tbos/bin        ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Requirements:                                                ║"
	@echo "║    Windows: brew install mingw-w64                            ║"
	@echo "║    Android: export ANDROID_NDK_HOME=/path/to/ndk              ║"
	@echo "║    iOS:     Xcode with iOS SDK                                ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

tier-test: tier-all
	@echo ""
	@echo "================================================================"
	@echo "  Running all tier demos..."
	@echo "================================================================"
	@echo ""
	@echo "--- Tier 0: Calculator ---"
	@$(TIER_BUILD_DIR)/tbos_calc
	@echo ""
	@echo "--- Tier 1: Embedded ---"
	@$(TIER_BUILD_DIR)/tbos_embed
	@echo ""
	@echo "--- Tier 3: Desktop ---"
	@$(TIER_BUILD_DIR)/tbos_desktop | head -40
	@echo "..."
	@echo ""
	@echo "All tier demos complete!"
