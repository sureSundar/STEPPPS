# TernaryBit Universal OS - Development Makefile
# Traceability: Implementation of all requirements through complete toolchain

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2 -g
INCLUDES = -Isrc -Isrc/core -Isrc/boot -Isrc/shell
TEST_DIR = tests
SRC_DIR = src
BUILD_DIR = build

# Source files
HW_DETECTOR_SRCS = $(SRC_DIR)/core/hardware_detector.c
BOOT_SRCS = $(SRC_DIR)/boot/universal_boot.c
STEPPPS_SRCS = $(SRC_DIR)/core/steppps_framework.c
MEMORY_SRCS = $(SRC_DIR)/core/memory_manager.c
SHELL_SRCS = $(SRC_DIR)/shell/command_shell.c $(SRC_DIR)/shell/universal_shell_compat.c

# New DI and Standards-Compliant Components
IOC_SRCS = $(SRC_DIR)/core/tbos_ioc_container.c $(SRC_DIR)/core/tbos_core_impl.c
DI_BOOTSTRAP_SRCS = $(SRC_DIR)/core/tbos_di_bootstrap.c
FILESYSTEM_V2_SRCS = $(SRC_DIR)/core/tbos_filesystem.c
PROCESS_V2_SRCS = $(SRC_DIR)/core/tbos_process.c

# All core sources for shell (including DI components)
ALL_CORE_SRCS = $(HW_DETECTOR_SRCS) $(BOOT_SRCS) $(STEPPPS_SRCS) $(MEMORY_SRCS) $(SHELL_SRCS)
ALL_DI_SRCS = $(IOC_SRCS) $(DI_BOOTSTRAP_SRCS) $(FILESYSTEM_V2_SRCS) $(PROCESS_V2_SRCS)
ALL_SRCS = $(ALL_CORE_SRCS) $(ALL_DI_SRCS)

# Test executables
TESTS = test_hardware_detector test_universal_boot test_steppps_framework test_memory_manager test_ioc_container test_di_bootstrap

# Interactive shell executables
SHELL_MAIN = ternarybit_shell
DI_SHELL_MAIN = ternarybit_shell_di

# Default target
all: directories $(TESTS) $(SHELL_MAIN)
	@echo "🏗️  TernaryBit OS Components Built Successfully"
	@echo "Run 'make test' to execute all tests"
	@echo "Run 'make shell' to start interactive shell"

# Create build directory
directories:
	@mkdir -p $(BUILD_DIR)

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

test-steppps: $(BUILD_DIR)/test_steppps_framework
	@echo "🧪 Testing STEPPPS Framework..."
	@./$(BUILD_DIR)/test_steppps_framework

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