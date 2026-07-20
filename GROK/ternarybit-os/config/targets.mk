# Build targets configuration
# Traceability: PRD-045 (Modular Build System)

# Main targets
BUILD_TARGETS = kernel.bin bootloader.bin
TEST_TARGETS = test_hardware_detector test_universal_boot test_steppps_framework test_memory_manager test_ioc_container test_di_bootstrap
SHELL_TARGETS = ternarybit_shell ternarybit_shell_di

# Create build directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)

# Include dependency files if they exist
-include $(OBJS:.o=.d)
