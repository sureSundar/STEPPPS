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

# Source directories
SRC_DIR = src
TEST_DIR = tests
ARCH_DIR = arch/$(ARCH)

# Source files
CORE_SRCS = $(wildcard $(SRC_DIR)/core/*.c) \
            $(wildcard $(ARCH_DIR)/*.c)
BOOT_SRCS = $(wildcard $(SRC_DIR)/boot/*.c)
SHELL_SRCS = $(wildcard $(SRC_DIR)/shell/*.c)

# Object files
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CORE_SRCS))
BOOT_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(BOOT_SRCS))
SHELL_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SHELL_SRCS))

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

# Install (placeholder)
install:
	@echo "📦 Install target not yet implemented"

# Help
help:
	@echo "TernaryBit Universal OS - Development Targets"
	@echo "============================================"
	@echo "all          - Build all components"
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