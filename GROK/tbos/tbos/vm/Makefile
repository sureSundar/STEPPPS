# TernaryBit Lean VM - Nano Edition Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS =

# Targets
TARGET = test_vm
SOURCES = tblvm_nano.c test_vm.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = tblvm_nano.h

# Size optimization flags (for embedded targets)
CFLAGS_TINY = -Os -ffunction-sections -fdata-sections
LDFLAGS_TINY = -Wl,--gc-sections

# Default target
all: $(TARGET)

# Build test program
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Built: $@"
	@size $@

# Compile objects
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Size-optimized build (for embedded)
tiny: CFLAGS += $(CFLAGS_TINY)
tiny: LDFLAGS += $(LDFLAGS_TINY)
tiny: clean $(TARGET)
	@echo "=== Size-optimized build ==="
	@size $(TARGET)

# Run tests
test: $(TARGET)
	@echo "=== Running VM tests ==="
	./$(TARGET)

# Interactive calculator test
interactive: $(TARGET)
	@echo "=== Interactive calculator ==="
	./$(TARGET) i

# Check VM code size
check-size: tblvm_nano.o
	@echo "=== VM Core Size ==="
	@size tblvm_nano.o
	@echo ""
	@echo "Target: < 4KB code"
	@nm --size-sort tblvm_nano.o | tail -20

# Disassembly (for analysis)
disasm: tblvm_nano.o
	objdump -d tblvm_nano.o > tblvm_nano.asm
	@echo "Disassembly written to tblvm_nano.asm"

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET) *.asm

.PHONY: all test interactive tiny check-size disasm clean
