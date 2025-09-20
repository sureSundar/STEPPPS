# Compiler and linker flags
# Traceability: PRD-046 (Build Configuration)

# Toolchain
CC = gcc
LD = $(CC)
AR = ar
RANLIB = ranlib

# Warning flags
WARN_FLAGS = -Wall -Wextra -Werror -pedantic

# Debug flags
DEBUG_FLAGS = -g -DDEBUG -D_GNU_SOURCE

# Optimization flags
OPTIMIZE_FLAGS = -O2 -flto -fno-omit-frame-pointer

# Standard flags
STD_FLAGS = -std=gnu99

# Include paths
INCLUDE_FLAGS = -Isrc -Isrc/core -Isrc/boot -Isrc/shell

# Common C flags
CFLAGS = $(WARN_FLAGS) $(DEBUG_FLAGS) $(OPTIMIZE_FLAGS) $(STD_FLAGS) $(INCLUDE_FLAGS)

# Linker flags
LDFLAGS = $(OPTIMIZE_FLAGS) -Wl,--gc-sections

# Test flags
TEST_CFLAGS = $(CFLAGS) -DTESTING
TEST_LDFLAGS = $(LDFLAGS) -lcmocka

# Build directory
BUILD_DIR = build

# Output files
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOTLOADER_BIN = $(BUILD_DIR)/bootloader.bin
