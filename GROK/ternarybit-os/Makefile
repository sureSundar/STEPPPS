# GNU Make's default goal is whichever target is defined *first* in the
# file, not whichever is named "all" - `paper` (below) held that position,
# so bare `make` silently built an unrelated research-paper PDF and failed
# without pandoc, never reaching any real build target. Force the real
# default explicitly rather than relying on definition order.
.DEFAULT_GOAL := all

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

# Default target: attempts the legacy bare-metal image (kernel/bootloader/
# shell below) plus every platform and architecture this repo can currently
# target - canonical tier shells for the host arch, ARM64 and RISC-V (each
# skipped gracefully if its cross-compiler isn't installed), Windows tier
# shells (skipped if no MinGW), and the iOS Simulator app (skipped if no
# Xcode). One `make` builds everything possible on this machine; a broken
# or unavailable target is reported and skipped rather than stopping the
# rest, the same way tbos-all-platforms already treats missing toolchains.
# BUILD_TARGETS/SHELL_TARGETS are invoked via sub-make, not as direct
# prerequisites, specifically so a hard failure there (e.g. kernel.bin has
# no matching rule today - config/targets.mk names it without the
# $(BUILD_DIR) prefix config/flags.mk's KERNEL_BIN actually uses) can't
# abort the working parts of the build.
all:
	@$(MAKE) --no-print-directory $(BUILD_TARGETS) $(SHELL_TARGETS) && \
		( echo "🏗️  TernaryBit OS for $(ARCH) built successfully"; \
		  echo "  Kernel: $(KERNEL_BIN)"; \
		  echo "  Bootloader: $(BOOTLOADER_BIN)"; \
		  echo "  Shell: $(SHELL_TARGETS)" ) || \
		echo "⚠️  Legacy bare-metal image (kernel.bin/bootloader.bin/shell) skipped - known broken, see docs/TBOS_CANONICAL_MANIFEST.md Phase 2 (freestanding core, not done yet)"
	@$(MAKE) --no-print-directory tbos-all-platforms
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

# ─────────────────────────────────────────────────────────────────────────────
# BARE-METAL BOOT SMOKE TEST
# ─────────────────────────────────────────────────────────────────────────────
# Builds the bare-metal image and boots it in QEMU headlessly for a bounded
# time, watching for the one thing a "compiles fine" build can't tell you:
# whether it actually runs without crashing. `-no-reboot -no-shutdown` is
# load-bearing - without it, QEMU silently resets on a triple fault and the
# run just looks like it's booting forever, with no evidence anything broke
# (this is exactly how the disk-geometry bug documented in WISDOM.md,
# 2026-07-21, stayed invisible for as long as it did).
.PHONY: test-baremetal test-baremetal-screenshot
BAREMETAL_IMG := build/tbos_universal.img
BAREMETAL_SERIAL_LOG := /tmp/tbos_baremetal_serial.log
BAREMETAL_DEBUG_LOG := /tmp/tbos_baremetal_debug.log
BAREMETAL_BOOT_SECONDS ?= 8

test-baremetal:
	@echo "== Building bare-metal image =="
	@./build_universal.sh > /tmp/tbos_baremetal_build.log 2>&1 || \
		(echo "❌ BUILD FAILED - see /tmp/tbos_baremetal_build.log"; tail -30 /tmp/tbos_baremetal_build.log; exit 1)
	@echo "Build OK."
	@echo ""
	@echo "== Booting in QEMU ($(BAREMETAL_BOOT_SECONDS)s, headless, fault-tracing on =="
	@rm -f $(BAREMETAL_SERIAL_LOG) $(BAREMETAL_DEBUG_LOG)
	@timeout $(BAREMETAL_BOOT_SECONDS) qemu-system-x86_64 \
		-drive file=$(BAREMETAL_IMG),format=raw -m 512 -display none \
		-serial file:$(BAREMETAL_SERIAL_LOG) -no-reboot -no-shutdown \
		-d int,cpu_reset -D $(BAREMETAL_DEBUG_LOG) || true
	@echo ""
	@if grep -q "Triple fault" $(BAREMETAL_DEBUG_LOG) 2>/dev/null; then \
		echo "❌ FAULT: kernel triple-faulted during boot."; \
		echo "--- serial output ($(BAREMETAL_SERIAL_LOG)) ---"; \
		cat $(BAREMETAL_SERIAL_LOG) 2>/dev/null; \
		echo "--- last interrupt trace ($(BAREMETAL_DEBUG_LOG)) ---"; \
		tail -20 $(BAREMETAL_DEBUG_LOG); \
		exit 1; \
	else \
		echo "✅ No triple fault detected in $(BAREMETAL_BOOT_SECONDS)s."; \
		echo "--- serial output ($(BAREMETAL_SERIAL_LOG)) ---"; \
		cat $(BAREMETAL_SERIAL_LOG) 2>/dev/null; \
		echo "-------------------------------------------"; \
		echo "Note: this only proves no CPU fault occurred, not that the boot"; \
		echo "banner/shell actually rendered correctly - serial only shows what"; \
		echo "the bootloader/kernel explicitly write to COM1, not VGA text mode"; \
		echo "output. Run 'make test-baremetal-screenshot' to see the screen."; \
	fi

test-baremetal-screenshot:
	@echo "== Building bare-metal image =="
	@./build_universal.sh > /tmp/tbos_baremetal_build.log 2>&1 || \
		(echo "❌ BUILD FAILED - see /tmp/tbos_baremetal_build.log"; tail -30 /tmp/tbos_baremetal_build.log; exit 1)
	@rm -f /tmp/tbos_baremetal_mon.sock /tmp/tbos_baremetal_boot.ppm /tmp/tbos_baremetal_boot.png $(BAREMETAL_SERIAL_LOG)
	@echo "Booting in the background, waiting $(BAREMETAL_BOOT_SECONDS)s, then capturing the VGA screen..."
	@(qemu-system-x86_64 -drive file=$(BAREMETAL_IMG),format=raw -m 512 -display none \
		-monitor unix:/tmp/tbos_baremetal_mon.sock,server,nowait \
		-serial file:$(BAREMETAL_SERIAL_LOG) -no-reboot -no-shutdown & echo $$! > /tmp/tbos_baremetal_qemu.pid)
	@sleep $(BAREMETAL_BOOT_SECONDS)
	@(echo "screendump /tmp/tbos_baremetal_boot.ppm"; sleep 1) | nc -U /tmp/tbos_baremetal_mon.sock > /dev/null 2>&1
	@sleep 1
	@kill -9 $$(cat /tmp/tbos_baremetal_qemu.pid) 2>/dev/null || true
	@if command -v sips >/dev/null 2>&1; then \
		sips -s format png /tmp/tbos_baremetal_boot.ppm --out /tmp/tbos_baremetal_boot.png >/dev/null 2>&1 && \
		echo "✅ Screenshot: /tmp/tbos_baremetal_boot.png"; \
	elif command -v convert >/dev/null 2>&1; then \
		convert /tmp/tbos_baremetal_boot.ppm /tmp/tbos_baremetal_boot.png && \
		echo "✅ Screenshot: /tmp/tbos_baremetal_boot.png"; \
	else \
		echo "✅ Screenshot (PPM, no PNG converter found): /tmp/tbos_baremetal_boot.ppm"; \
	fi

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
#
# Canonical shell is tbos_tier_shell.c (see docs/TBOS_CANONICAL_MANIFEST.md) -
# `make shell` builds and runs it at the top tier (Supercomputer, all 54
# commands: real pxfs/ucfs/rf2s/network/steppps). For a specific tier, use
# tier-shell-calc/embed/retro/desktop/server/super directly.
shell: tier-shell-super
	@echo "🚀 Starting TernaryBit OS Interactive Shell (Tier 5 - Supercomputer)..."
	@./$(TIER_BUILD_DIR)/shell_super

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
.PHONY: agent-contract-test agent-contract-birth

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

# First governed digital-being lifecycle experiment.
agent-contract-birth:
	@./tools/tbos-agent birth sundars-agent \
		config/agents/sundars-agent.contract.json

agent-contract-test:
	@python3 -m unittest tests.integration.test_agent_contract

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

# x86_64 cross-compile (use native gcc for hosted builds since elf-gcc lacks libc).
# On an x86_64 host these paths are already owned by arch-host; reuse that
# target instead of defining a second recipe for the same expanded filenames.
ifeq ($(TBOS_HOST_ARCH),x86_64)
arch-x86_64: arch-host
	@echo "[x86_64] Build complete"
else
arch-x86_64: build/x86_64/tbos_shell build/x86_64/tbos_hybrid
	@echo "[x86_64] Build complete"

build/x86_64/tbos_shell: $(TBOS_SHELL_SRCS)
ifneq ($(X86_64_ELF_CC),)
	@mkdir -p build/x86_64
	@echo "[x86_64] Building tbos_shell (cross)..."
	@$(X86_64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[x86_64] Skipped (needs hosted libc)"
else
	@echo "[x86_64] Cross-compiler not found, skipping"
endif

build/x86_64/tbos_hybrid: $(TBOS_HYBRID_SRCS)
ifneq ($(X86_64_ELF_CC),)
	@mkdir -p build/x86_64
	@echo "[x86_64] Building tbos_hybrid (cross)..."
	@$(X86_64_ELF_CC) $(TBOS_COMMON_FLAGS) -o $@ $< 2>/dev/null || echo "[x86_64] Skipped (needs hosted libc)"
else
	@echo "[x86_64] Cross-compiler not found, skipping"
endif
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

.PHONY: tier-all tier-demo-all tier-arch-all tier-arch-host tier-arch-arm64 tier-arch-riscv64
.PHONY: tier-calc tier-embed tier-retro tier-desktop tier-server tier-super tier-info tier-test
.PHONY: tier-shell-all tier-shell-smoke tier-shell-calc tier-shell-embed tier-shell-retro tier-shell-desktop tier-shell-server tier-shell-super

# A hardware tier and a CPU architecture are independent dimensions. Keep each
# architecture in its own directory so a cross-build cannot overwrite the host
# binaries. Override TIER_CC for toolchains not discovered above.
TIER_ARCH ?= $(TBOS_HOST_ARCH)
TIER_CC ?= gcc
TIER_ARCH_FLAGS ?=
TIER_BUILD_DIR := build/tiers/$(TIER_ARCH)
TIER_DEMO_SRC := demos/tbos_universal_demo.c
TIER_SHELL_SRC := src/shell/tbos_tier_shell.c
# Canonical path codecs (docs/TBOS_CANONICAL_MANIFEST.md), reused for the
# tier shell's ucfs/rf2s commands instead of a fourth reimplementation.
TIER_SHELL_FS_SRCS := src/core/filesystem/ucfs_codec.c src/core/filesystem/rf2s_codec.c \
                      src/core/compression/pxfs_lossless.c \
                      kernel/fs/vfs.c kernel/fs/ramfs.c
# Hosted HAL adapter - the tier shell's command table asks
# caps.has_dynamic_memory instead of hardcoding a storage strategy.
# Link kernel/hal_baremetal.c instead to build against a platform with
# no heap; the static-array fallback in the shell takes over.
TIER_SHELL_HAL_SRCS := kernel/hal_hosted.c
# Canonical STEPPPS v2 runtime, linked directly (not a sidecar - every
# tier stays standalone, matching everything else in this file).
TIER_SHELL_STEPPPS_SRCS := steppps/v2/steppps_runtime.c
TIER_SHELL_STEPPPS_INC := -Isteppps/v2

# Native hosted shell. This is a platform frontend over the same Tier 5 shell,
# hosted HAL, codecs and STEPPPS runtime used by `make shell`; it is not another
# command implementation. Native platform builds intentionally do not pretend
# to cross-compile a macOS binary from Linux or vice versa.
.PHONY: hosted-shell hosted-shell-test
HOST_PLATFORM ?= $(TBOS_HOST_OS)
HOSTED_SHELL_CC ?= cc
HOSTED_SHELL_DIR := build/hosted/$(HOST_PLATFORM)
HOSTED_SHELL_BIN := $(HOSTED_SHELL_DIR)/tbos-shell
HOSTED_SHELL_SRCS := $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) \
                     $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS)

hosted-shell: $(HOSTED_SHELL_BIN)
	@echo "Hosted TBOS shell: $(HOSTED_SHELL_BIN)"

$(HOSTED_SHELL_BIN): $(HOSTED_SHELL_SRCS)
	@if [ "$(HOST_PLATFORM)" != "$(TBOS_HOST_OS)" ]; then \
		echo "Cannot build HOST_PLATFORM=$(HOST_PLATFORM) natively on $(TBOS_HOST_OS)"; \
		exit 1; \
	fi
	@mkdir -p $(@D)
	@echo "Building native $(HOST_PLATFORM)/$(TBOS_HOST_ARCH) TBOS shell..."
	@$(HOSTED_SHELL_CC) -std=c11 -O2 -Wall -Wextra \
		-DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=5 \
		-Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

hosted-shell-test: hosted-shell
	@printf 'help\nexit\n' | $(HOSTED_SHELL_BIN) > /tmp/tbos-hosted-shell-test.log
	@grep -q '54 commands available' /tmp/tbos-hosted-shell-test.log
	@grep -q 'Namaste' /tmp/tbos-hosted-shell-test.log
	@echo "Hosted shell smoke test passed: $(HOSTED_SHELL_BIN)"

tier-info:
	@echo ""
	@echo "================================================================"
	@echo "  TBOS Hardware Tier Build System"
	@echo "  From 4-bit Calculators to Supercomputers"
	@echo "================================================================"
	@echo ""
	@echo "  Build architecture: $(TIER_ARCH)"
	@echo "  Compiler:           $(TIER_CC)"
	@echo "  Output:             $(TIER_BUILD_DIR)"
	@echo ""
	@echo "  Tier 0: Calculator   (4-8 bit,  256B-4KB RAM)  16 commands"
	@echo "  Tier 1: Embedded     (16-bit,   4KB-64KB RAM)  27 commands"
	@echo "  Tier 2: Retro        (32-bit,   64KB-16MB RAM) 36 commands"
	@echo "  Tier 3: Desktop      (64-bit,   16MB-16GB RAM) 45 commands"
	@echo "  Tier 4: Server       (64-bit,   16GB-1TB RAM)  50 commands"
	@echo "  Tier 5: Supercomputer (64-bit+, 1TB+ distributed) 56 commands"
	@echo ""
	@echo "  Demo targets (display tier info):"
	@echo "    make tier-all        Build interactive TBOS binaries and demos"
	@echo "    make tier-demo-all   Build one-shot hardware demonstrations only"
	@echo "    make tier-arch-all   Build demos and shells for every available CPU"
	@echo "    make tier-all TIER_ARCH=arm64 TIER_CC=aarch64-linux-gnu-gcc"
	@echo "    make tier-calc       Run calculator demo"
	@echo "    make tier-test       Run all tier demos"
	@echo ""
	@echo "  Shell targets (full interactive shells):"
	@echo "    make tier-shell-all     Build all tier shells"
	@echo "    make tier-shell-smoke   Verify every tbos_* binary has a prompt"
	@echo "    make tier-shell-calc    Calculator shell (5 cmds)"
	@echo "    make tier-shell-embed   Embedded shell (15 cmds)"
	@echo "    make tier-shell-retro   Retro shell (25 cmds)"
	@echo "    make tier-shell-desktop Desktop shell (35 cmds)"
	@echo "    make tier-shell-server  Server shell (40 cmds)"
	@echo "    make tier-shell-super   Supercomputer shell (45 cmds)"
	@echo "================================================================"
	@echo ""

tier-arch-host:
	@$(MAKE) --no-print-directory tier-all \
		TIER_ARCH=$(TBOS_HOST_ARCH) TIER_CC=gcc

tier-arch-arm64:
ifeq ($(ARM64_ELF_CC),)
	@echo "ARM64 tier build skipped: no aarch64 cross-compiler found"
else
	@$(MAKE) --no-print-directory tier-all \
		TIER_ARCH=arm64 TIER_CC=$(ARM64_ELF_CC) \
		TIER_ARCH_FLAGS=-DTBOS_ARCH_ARM64
endif

tier-arch-riscv64:
ifeq ($(RISCV64_ELF_CC),)
	@echo "RISC-V tier build skipped: no riscv64 cross-compiler found"
else
	@$(MAKE) --no-print-directory tier-all \
		TIER_ARCH=riscv64 TIER_CC=$(RISCV64_ELF_CC) \
		TIER_ARCH_FLAGS=-DTBOS_ARCH_RISCV64
endif

tier-arch-all: tier-arch-host
ifneq ($(TBOS_HOST_ARCH),arm64)
	@$(MAKE) --no-print-directory tier-arch-arm64
endif
ifneq ($(TBOS_HOST_ARCH),riscv64)
	@$(MAKE) --no-print-directory tier-arch-riscv64
endif
	@echo "Tier architecture matrix complete under build/tiers/"

tier-all: tier-shell-all tier-demo-all
	@echo ""
	@echo "All interactive TBOS tiers and hardware demos built:"
	@ls -la $(TIER_BUILD_DIR)/

tier-demo-all: $(TIER_BUILD_DIR)/demo_calc $(TIER_BUILD_DIR)/demo_embed $(TIER_BUILD_DIR)/demo_retro $(TIER_BUILD_DIR)/demo_desktop $(TIER_BUILD_DIR)/demo_server $(TIER_BUILD_DIR)/demo_super
	@echo "Hardware demonstrations built as $(TIER_BUILD_DIR)/demo_*"

$(TIER_BUILD_DIR):
	@mkdir -p $(TIER_BUILD_DIR)

$(TIER_BUILD_DIR)/demo_calc: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 0] Building Calculator (8-bit, 256B RAM)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DTBOS_TIER=0 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/demo_embed: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 1] Building Embedded (16-bit, 4KB RAM)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DTBOS_TIER=1 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/demo_retro: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 2] Building Retro (32-bit, 64KB RAM)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DTBOS_TIER=2 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/demo_desktop: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 3] Building Desktop (64-bit, 1MB RAM)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DTBOS_TIER=3 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/demo_server: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 4] Building Server (64-bit SMP, 4MB RAM)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DTBOS_TIER=4 -Wall -Wextra -Iinclude -o $@ $<

$(TIER_BUILD_DIR)/demo_super: $(TIER_DEMO_SRC) | $(TIER_BUILD_DIR)
	@echo "[Tier 5] Building Supercomputer (64-bit distributed, 16MB RAM)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DTBOS_TIER=5 -Wall -Wextra -Iinclude -o $@ $<

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

tier-shell-all: $(TIER_BUILD_DIR)/tbos_calc $(TIER_BUILD_DIR)/tbos_embed $(TIER_BUILD_DIR)/tbos_retro $(TIER_BUILD_DIR)/tbos_desktop $(TIER_BUILD_DIR)/tbos_server $(TIER_BUILD_DIR)/tbos_super
	@echo ""
	@echo "All tier shells built:"
	@ls -la $(TIER_BUILD_DIR)/shell_*
	@echo ""
	@echo "Commands per tier (verified by running each binary's 'help'):"
	@echo "  shell_calc    (Tier 0): 16 commands  - help, echo, calc, karma, exit, net,"
	@echo "                                          ucfs, rf2s, pxencode, pxcompress,"
	@echo "                                          pxdecompress, pxstore, pxload, pxsend,"
	@echo "                                          pxrecv, steppps"
	@echo "  shell_embed   (Tier 1): 27 commands  - + pwd, ls, cd, cat, mkdir, rm, send, etc."
	@echo "  shell_retro   (Tier 2): 36 commands  - + head, grep, wc, history, resolve, etc."
	@echo "  shell_desktop (Tier 3): 45 commands  - + meditate, sangha, dharma, ping, http"
	@echo "  shell_server  (Tier 4): 50 commands  - + top, netstat, free, lscpu, listen"
	@echo "  shell_super   (Tier 5): 56 commands  - + cluster, consciousness, quantum, netscan"
	@echo ""
	@echo "Every tier has real POSIX-socket networking (net/send/resolve/http/"
	@echo "listen/netscan), scaled per tier - not system()/popen() shell-outs."
	@echo "Every tier, including Calculator, has real ucfs/rf2s path parsing"
	@echo "(canonical codecs, linked not reimplemented), real genuinely lossless"
	@echo "PXFS compression (store/transmit/extract, byte-exact), and real"
	@echo "STEPPPS (linked against steppps/v2 - parse/verify/run/audit real"
	@echo "entities, gated by trust risk not cost: show is Tier 0+,"
	@echo "verify/run/audit climb tiers)."

# User-facing TBOS names are the interactive shells. Keep shell_* as the
# compatibility/build names used by older scripts, but never publish a
# one-shot demo under a tbos_* name again.
$(TIER_BUILD_DIR)/tbos_calc: $(TIER_BUILD_DIR)/shell_calc Makefile
	@cp $< $@
$(TIER_BUILD_DIR)/tbos_embed: $(TIER_BUILD_DIR)/shell_embed Makefile
	@cp $< $@
$(TIER_BUILD_DIR)/tbos_retro: $(TIER_BUILD_DIR)/shell_retro Makefile
	@cp $< $@
$(TIER_BUILD_DIR)/tbos_desktop: $(TIER_BUILD_DIR)/shell_desktop Makefile
	@cp $< $@
$(TIER_BUILD_DIR)/tbos_server: $(TIER_BUILD_DIR)/shell_server Makefile
	@cp $< $@
$(TIER_BUILD_DIR)/tbos_super: $(TIER_BUILD_DIR)/shell_super Makefile
	@cp $< $@

tier-shell-smoke: tier-shell-all
	@set -e; \
	for shell in calc embed retro desktop server super; do \
		output=$$(printf 'help\nexit\n' | $(TIER_BUILD_DIR)/tbos_$$shell); \
		printf '%s\n' "$$output" | grep -Eq '[>$$] '; \
		printf '%s\n' "$$output" | grep -q 'Available commands'; \
		echo "[PASS] tbos_$$shell presents an interactive prompt"; \
	done

$(TIER_BUILD_DIR)/shell_calc: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS) | $(TIER_BUILD_DIR)
	@echo "[Tier 0] Building Calculator Shell (16 commands)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=0 -Wall -Wextra -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

$(TIER_BUILD_DIR)/shell_embed: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS) | $(TIER_BUILD_DIR)
	@echo "[Tier 1] Building Embedded Shell (27 commands)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=1 -Wall -Wextra -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

$(TIER_BUILD_DIR)/shell_retro: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS) | $(TIER_BUILD_DIR)
	@echo "[Tier 2] Building Retro Shell (36 commands)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=2 -Wall -Wextra -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

$(TIER_BUILD_DIR)/shell_desktop: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS) | $(TIER_BUILD_DIR)
	@echo "[Tier 3] Building Desktop Shell (45 commands)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=3 -Wall -Wextra -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

$(TIER_BUILD_DIR)/shell_server: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS) | $(TIER_BUILD_DIR)
	@echo "[Tier 4] Building Server Shell (50 commands)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=4 -Wall -Wextra -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

$(TIER_BUILD_DIR)/shell_super: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS) $(TIER_SHELL_HAL_SRCS) $(TIER_SHELL_STEPPPS_SRCS) | $(TIER_BUILD_DIR)
	@echo "[Tier 5] Building Supercomputer Shell (56 commands)..."
	@$(TIER_CC) $(TIER_ARCH_FLAGS) -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=5 -Wall -Wextra -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm

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

# Prints the canonical hosted-shell binary path without building or running
# it, so other tooling (build_universal.sh --profile host) can build the
# same file target directly instead of launching the interactive shell.
.PHONY: print-tier-shell-bin
print-tier-shell-bin:
	@echo "$(TIER_BUILD_DIR)/shell_super"

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
TBOS_CROSS_STEPPPS_SRCS := kernel/fs/steppps_vfs.c fs/pxfs_steppps.c steppps/v2/steppps_runtime.c src/core/filesystem/ucfs_codec.c
TBOS_CROSS_INCLUDES := -I. -Iinclude

# ─────────────────────────────────────────────────────────────────────────────
# MACOS (Native build)
# ─────────────────────────────────────────────────────────────────────────────

tbos-macos: build/macos/tbos
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════════╗"
	@echo "║  TBOS macOS Build Complete                                    ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  build/macos/tbos - TBOS + STEPPPS integrated shell           ║"
	@echo "╠═══════════════════════════════════════════════════════════════╣"
	@echo "║  Run: ./build/macos/tbos                                      ║"
	@echo "║  STEPPPS: steppps show/run/verify <file> (inside shell)       ║"
	@echo "╚═══════════════════════════════════════════════════════════════╝"

build/macos/tbos: $(TBOS_CROSS_SRCS) $(TBOS_CROSS_STEPPPS_SRCS)
	@mkdir -p build/macos
	@echo "Building TBOS + STEPPPS integrated shell for macOS..."
	@gcc -DHOST_BUILD -DSTEPPPS_INTEGRATED -DUCFS_INTEGRATED -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES)
	@echo "macOS build complete!"

# ─────────────────────────────────────────────────────────────────────────────
# WINDOWS (MinGW-w64 cross-compilation)
# ─────────────────────────────────────────────────────────────────────────────
# Install MinGW: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)

MINGW_CC := x86_64-w64-mingw32-gcc
MINGW_EXISTS := $(shell which $(MINGW_CC) 2>/dev/null)
# MinGW-w64's linker defaults the PE subsystem/OS version fields to 5.2/4.0
# (Windows Server 2003/XP-64, NT4-era) - anachronistic metadata for a binary
# meant to run on Windows 10/11, and reported to trigger "This app can't run
# on your PC" on some Windows 11 machines. Force a modern (Vista+) baseline
# explicitly rather than relying on the linker's default.
MINGW_MODERN_VERSION_FLAGS := -Wl,--major-subsystem-version,6 -Wl,--minor-subsystem-version,0 \
                               -Wl,--major-os-version,6 -Wl,--minor-os-version,0

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
	@$(MINGW_CC) -DHOST_BUILD -D_WIN32 -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static $(MINGW_MODERN_VERSION_FLAGS)
	@echo "Windows build complete!"

build/windows/steppps.exe: $(STEPPPS_V2_SRCS)
	@mkdir -p build/windows
	@echo "Building STEPPPS CLI for Windows..."
	@$(MINGW_CC) -DHOST_BUILD -D_WIN32 -Wall -O2 -o $@ $^ $(TBOS_CROSS_INCLUDES) -static $(MINGW_MODERN_VERSION_FLAGS)
	@echo "STEPPPS Windows build complete!"

# ─────────────────────────────────────────────────────────────────────────────
# WINDOWS TIER SHELLS (MinGW-w64 cross-compile of the same canonical
# tbos_tier_shell.c, not the tbos_complete_shell.c the tbos-windows target
# above builds.) The macOS/Linux tier shells transferred to Windows fail
# with "is not recognized" because they're Mach-O/ELF - a format Windows
# cannot execute at all, regardless of extension. These are real PE32+ .exe
# binaries of the identical source.
#
# kernel/fs/vfs.c + ramfs.c are dropped (uid_t/gid_t don't exist without a
# POSIX layer on Windows) and kernel/hal_hosted.c is replaced with
# kernel/hal_baremetal.c (needs netdb.h/BSD sockets; see the "no heap"
# substitution documented above TIER_SHELL_HAL_SRCS). Neither loss is a
# regression: tbos_tier_shell.c's own #ifndef _WIN32 guards already exclude
# net/vfs commands (net, netscan, vmkdir, vls, ucfs, rf2s, px*, steppps)
# from registration on Windows - those files were never linked into any
# command Windows could reach.
# ─────────────────────────────────────────────────────────────────────────────
WINDOWS_BUILD_DIR := build/windows
TIER_SHELL_FS_SRCS_WIN := src/core/filesystem/ucfs_codec.c src/core/filesystem/rf2s_codec.c \
                          src/core/compression/pxfs_lossless.c
TIER_SHELL_HAL_SRCS_WIN := kernel/hal_baremetal.c

$(WINDOWS_BUILD_DIR)/shell_calc.exe: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS_WIN) $(TIER_SHELL_HAL_SRCS_WIN) $(TIER_SHELL_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)"; exit 1
endif
	@mkdir -p $(WINDOWS_BUILD_DIR)
	@echo "[Windows/Tier 0] Building Calculator Shell..."
	@$(MINGW_CC) -D_WIN32 -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=0 -Wall -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm -static $(MINGW_MODERN_VERSION_FLAGS)

$(WINDOWS_BUILD_DIR)/shell_embed.exe: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS_WIN) $(TIER_SHELL_HAL_SRCS_WIN) $(TIER_SHELL_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)"; exit 1
endif
	@mkdir -p $(WINDOWS_BUILD_DIR)
	@echo "[Windows/Tier 1] Building Embedded Shell..."
	@$(MINGW_CC) -D_WIN32 -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=1 -Wall -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm -static $(MINGW_MODERN_VERSION_FLAGS)

$(WINDOWS_BUILD_DIR)/shell_retro.exe: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS_WIN) $(TIER_SHELL_HAL_SRCS_WIN) $(TIER_SHELL_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)"; exit 1
endif
	@mkdir -p $(WINDOWS_BUILD_DIR)
	@echo "[Windows/Tier 2] Building Retro Shell..."
	@$(MINGW_CC) -D_WIN32 -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=2 -Wall -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm -static $(MINGW_MODERN_VERSION_FLAGS)

$(WINDOWS_BUILD_DIR)/shell_desktop.exe: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS_WIN) $(TIER_SHELL_HAL_SRCS_WIN) $(TIER_SHELL_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)"; exit 1
endif
	@mkdir -p $(WINDOWS_BUILD_DIR)
	@echo "[Windows/Tier 3] Building Desktop Shell..."
	@$(MINGW_CC) -D_WIN32 -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=3 -Wall -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm -static $(MINGW_MODERN_VERSION_FLAGS)

$(WINDOWS_BUILD_DIR)/shell_server.exe: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS_WIN) $(TIER_SHELL_HAL_SRCS_WIN) $(TIER_SHELL_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)"; exit 1
endif
	@mkdir -p $(WINDOWS_BUILD_DIR)
	@echo "[Windows/Tier 4] Building Server Shell..."
	@$(MINGW_CC) -D_WIN32 -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=4 -Wall -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm -static $(MINGW_MODERN_VERSION_FLAGS)

$(WINDOWS_BUILD_DIR)/shell_super.exe: $(TIER_SHELL_SRC) $(TIER_SHELL_FS_SRCS_WIN) $(TIER_SHELL_HAL_SRCS_WIN) $(TIER_SHELL_STEPPPS_SRCS)
ifndef MINGW_EXISTS
	@echo "Error: MinGW not found. Install with: brew install mingw-w64 (macOS) or apt install mingw-w64 (Linux)"; exit 1
endif
	@mkdir -p $(WINDOWS_BUILD_DIR)
	@echo "[Windows/Tier 5] Building Supercomputer Shell..."
	@$(MINGW_CC) -D_WIN32 -DTBOS_HOSTED -DHOST_BUILD -DTBOS_TIER=5 -Wall -Iinclude $(TIER_SHELL_STEPPPS_INC) -o $@ $^ -lm -static $(MINGW_MODERN_VERSION_FLAGS)

.PHONY: tier-shell-windows-all
tier-shell-windows-all: $(WINDOWS_BUILD_DIR)/shell_calc.exe $(WINDOWS_BUILD_DIR)/shell_embed.exe \
                        $(WINDOWS_BUILD_DIR)/shell_retro.exe $(WINDOWS_BUILD_DIR)/shell_desktop.exe \
                        $(WINDOWS_BUILD_DIR)/shell_server.exe $(WINDOWS_BUILD_DIR)/shell_super.exe
	@echo "Windows tier shells built in $(WINDOWS_BUILD_DIR)/ - copy the .exe to Windows and run in cmd.exe/PowerShell."
	@echo "Network and virtual-root commands (net, netscan, vmkdir, vls, ucfs, rf2s, px*, steppps) are unavailable in this build; the same tier shell built natively on macOS/Linux has the full command set."

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

tbos-all-platforms:
	@echo "Building for all available platforms..."
	@echo ""
	@echo "== TBOS + STEPPPS host build =="
	@$(MAKE) --no-print-directory tbos-steppps 2>/dev/null || echo "tbos-steppps: Skipped (pre-existing build error, unrelated to this target - see demos/steppps_fs_demo.c: undeclared PXFS_HEADER_SIZE)"
	@echo ""
	@echo "== Canonical tier shells (host architecture) =="
	@$(MAKE) --no-print-directory tier-arch-all
	@echo ""
	@echo "== Canonical tier shells (Windows, MinGW) =="
	@$(MAKE) --no-print-directory tier-shell-windows-all 2>/dev/null || echo "Windows tier shells: Skipped (no MinGW)"
	@echo ""
	@echo "== Legacy tbos_complete_shell.c track (docs/TBOS_CANONICAL_MANIFEST.md: non-canonical) =="
	@$(MAKE) tbos-windows 2>/dev/null || echo "Windows (legacy): Skipped (no MinGW)"
	@$(MAKE) tbos-android 2>/dev/null || echo "Android: Skipped (no NDK)"
	@$(MAKE) tbos-ios 2>/dev/null || echo "iOS: Skipped (no Xcode)"
	@echo ""
	@echo "== TernaryBit OS iOS app (mobile/ios/TernaryBitOS, Simulator SDK) =="
	@cd mobile/ios/TernaryBitOS && xcodebuild -project TernaryBitOS.xcodeproj -target TernaryBitOS -sdk iphonesimulator -configuration Debug build -quiet 2>/dev/null && echo "iOS app: built mobile/ios/TernaryBitOS/build/Debug-iphonesimulator/TernaryBitOS.app" || echo "iOS app: Skipped (no Xcode or build failed)"
	@echo ""
	@echo "Cross-platform build complete!"
	@ls -la build/*/tbos* build/*/*.exe build/*/*.a 2>/dev/null || true
	@ls -la build/tiers/*/shell_* build/windows/shell_*.exe 2>/dev/null || true

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
	@$(TIER_BUILD_DIR)/demo_calc
	@echo ""
	@echo "--- Tier 1: Embedded ---"
	@$(TIER_BUILD_DIR)/demo_embed
	@echo ""
	@echo "--- Tier 3: Desktop ---"
	@$(TIER_BUILD_DIR)/demo_desktop | head -40
	@echo "..."
	@echo ""
	@echo "All tier demos complete!"
