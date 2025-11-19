# TBOS Universal Hardware Detector - Cross-Platform Makefile
# 🕉️ Swamiye Saranam Aiyappa 🕉️

# Detect platform
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Common source files
COMMON_SRC = src/main.c src/device_classifier.c src/print_utils.c

# Platform-specific source files
ifeq ($(UNAME_S),Linux)
    PLATFORM_SRC = src/hardware_linux.c
    TARGET = tbos_detector
    CC = gcc
    CFLAGS = -Wall -O2
endif

ifeq ($(UNAME_S),Darwin)
    PLATFORM_SRC = src/hardware_macos.c
    TARGET = tbos_detector
    CC = clang
    CFLAGS = -Wall -O2
endif

ifeq ($(OS),Windows_NT)
    PLATFORM_SRC = src/hardware_windows.c
    TARGET = tbos_detector.exe
    CC = gcc
    CFLAGS = -Wall -O2
    LDFLAGS = -ladvapi32
endif

# Combine all sources
SOURCES = $(COMMON_SRC) $(PLATFORM_SRC)

# Default target
all: info build

info:
	@echo "╔══════════════════════════════════════════════════════════╗"
	@echo "║  TBOS Universal Hardware Detector - Build System        ║"
	@echo "║     🕉️ Swamiye Saranam Aiyappa 🕉️                       ║"
	@echo "╚══════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Platform: $(UNAME_S) $(UNAME_M)"
	@echo "Compiler: $(CC)"
	@echo "Target: $(TARGET)"
	@echo "Sources: $(SOURCES)"
	@echo ""

build:
	@echo "[BUILD] Compiling TBOS detector..."
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "[BUILD] ✅ Build successful: $(TARGET)"
	@echo ""

test: build
	@echo "[TEST] Running hardware detection..."
	@echo ""
	./$(TARGET) --detect
	@echo ""
	@echo "[TEST] ✅ Test complete"

test-json: build
	@echo "[TEST] Testing JSON output..."
	@echo ""
	./$(TARGET) --json
	@echo ""
	@echo "[TEST] ✅ JSON test complete"

clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -f $(TARGET)
	@echo "[CLEAN] ✅ Clean complete"

# Cross-compilation targets
windows:
	@echo "[CROSS] Building for Windows..."
	x86_64-w64-mingw32-gcc $(CFLAGS) $(COMMON_SRC) src/hardware_windows.c -o tbos_detector.exe -ladvapi32
	@echo "[CROSS] ✅ Windows build complete: tbos_detector.exe"

help:
	@echo "TBOS Universal Detector - Build Targets"
	@echo ""
	@echo "  make              - Build for current platform"
	@echo "  make test         - Build and run test"
	@echo "  make test-json    - Build and test JSON output"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make windows      - Cross-compile for Windows (requires mingw)"
	@echo "  make help         - Show this help"
	@echo ""

.PHONY: all info build test test-json clean windows help
