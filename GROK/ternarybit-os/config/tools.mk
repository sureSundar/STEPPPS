# Build tools configuration
# Traceability: PRD-047 (Build Tools Management)

# Default architecture (can be overridden)
ARCH ?= x86_64

# QEMU settings
QEMU = qemu-system-$(QEMU_ARCH)
QEMU_ARCH = $(if $(filter arm arm64,$(ARCH)),arm,aarch64,$(if $(filter riscv64,$(ARCH)),riscv64,i386))
QEMU_FLAGS = $(if $(filter x86_64 i386,$(ARCH)),-fda $(BUILD_DIR)/om.img,) -nographic -serial mon:stdio

# Debugger settings
GDB = gdb
GDB_ARCH = $(if $(filter arm arm64,$(ARCH)),arm-none-eabi-,$(if $(filter riscv64,$(ARCH)),riscv64-unknown-elf-,))gdb
GDB_FLAGS = -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL_BIN)"

# Code coverage
GCOV = gcov
LCOV = lcov
GENHTML = genhtml

# Static analysis tools
CPPCHECK = cppcheck
CPPCHECK_FLAGS = --enable=all --inconclusive --std=c99 --suppress=missingInclude

# Formatting tools
CLANG_FORMAT = clang-format
CLANG_FORMAT_FLAGS = -i --style=file

# Documentation
DOXYGEN = doxygen

# Test framework
TEST_RUNNER = scripts/run-tests.sh

# Deployment tools
DEPLOY_SCRIPT = scripts/deploy.sh

# Architecture-specific toolchain configuration
ifeq ($(ARCH),x86_64)
    # x86_64 toolchain
    CC = x86_64-elf-gcc
    LD = x86_64-elf-ld
    OBJCOPY = x86_64-elf-objcopy
    CFLAGS += -m64 -march=x86-64 -mabi=sysv
    LDFLAGS += -m elf_x86_64
    
    # Windows cross-compilation
    WIN_CC = x86_64-w64-mingw32-gcc
    WIN_LD = x86_64-w64-mingw32-ld
    WIN_OBJCOPY = x86_64-w64-mingw32-objcopy
    
else ifeq ($(ARCH),arm)
    # 32-bit ARM toolchain
    CC = arm-none-eabi-gcc
    LD = arm-none-eabi-ld
    OBJCOPY = arm-none-eabi-objcopy
    CFLAGS += -march=armv7-a -mthumb -mfpu=neon
    LDFLAGS += -m armelf_linux_eabi
    
else ifeq ($(ARCH),arm64)
    # 64-bit ARM (AArch64) toolchain
    CC = aarch64-linux-gnu-gcc
    LD = aarch64-linux-gnu-ld
    OBJCOPY = aarch64-linux-gnu-objcopy
    CFLAGS += -march=armv8-a
    LDFLAGS += -m aarch64linux
    
else ifeq ($(ARCH),riscv64)
    # 64-bit RISC-V toolchain
    CC = riscv64-unknown-elf-gcc
    LD = riscv64-unknown-elf-ld
    OBJCOPY = riscv64-unknown-elf-objcopy
    CFLAGS += -march=rv64imac -mabi=lp64
    LDFLAGS += -m elf64lriscv
    
else
    $(error Unsupported architecture: $(ARCH). Supported: x86_64, arm, arm64, riscv64)
endif

# Common cross-compilation settings
CFLAGS += -ffreestanding -nostdlib -fno-builtin -fno-stack-protector
LDFLAGS += -nostdlib -z max-page-size=0x1000
