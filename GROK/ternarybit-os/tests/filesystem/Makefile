# PXFS Test Suite Makefile

CC ?= gcc
CFLAGS := -Wall -Wextra -O2 -g -std=c99

SRCDIR := ../../src/core/filesystem
INCDIR := ../../include

# Use system includes, not TBOS custom includes
INCLUDES := -I$(INCDIR)/fs

# Source files
PXFS_CORE := $(SRCDIR)/pxfs_core.c
TEST_PXFS := test_pxfs.c

# Targets
TEST_BIN := test_pxfs

.PHONY: all clean test

all: $(TEST_BIN)

$(TEST_BIN): $(TEST_PXFS) $(PXFS_CORE)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)
