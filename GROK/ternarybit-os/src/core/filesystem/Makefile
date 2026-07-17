# PXFS - Pixel eXchange FileSystem
# Build tools and FUSE driver

CC ?= gcc
CFLAGS := -Wall -Wextra -O2 -g
LDFLAGS :=

# FUSE configuration (macFUSE on macOS, FUSE3 on Linux)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    FUSE_CFLAGS := -I/usr/local/include/fuse -D_FILE_OFFSET_BITS=64
    FUSE_LDFLAGS := -L/usr/local/lib -lfuse -lpthread
else
    FUSE_CFLAGS := $(shell pkg-config --cflags fuse 2>/dev/null || echo "-I/usr/include/fuse -D_FILE_OFFSET_BITS=64")
    FUSE_LDFLAGS := $(shell pkg-config --libs fuse 2>/dev/null || echo "-lfuse -lpthread")
endif

# Directories (relative to src/core/filesystem)
SRCDIR := .
INCDIR := ../../../include
TOOLSDIR := ../../../tools
BINDIR := ../../../bin

# Include paths
INCLUDES := -I$(INCDIR)/fs

# Source files
PXFS_CORE_SRC := pxfs_core.c
PXFS_FUSE_SRC := pxfs_fuse.c
STEPPPS_XATTR_SRC := steppps_xattr.c
PXFS_STEPPPS_SRC := ../../../fs/pxfs_steppps.c
UCFS_FUSE_SRC := ucfs_fuse.c
RF2S_FUSE_SRC := rf2s_fuse.c
MKFS_SRC := $(TOOLSDIR)/mkfs.pxfs.c

# Targets
PXFS_FUSE := $(BINDIR)/pxfs.fuse
UCFS_FUSE := $(BINDIR)/ucfs.fuse
RF2S_FUSE := $(BINDIR)/rf2s.fuse
MKFS_PXFS := $(BINDIR)/mkfs.pxfs
MOUNT_PXFS := $(BINDIR)/mount.pxfs
MOUNT_UCFS := $(BINDIR)/mount.ucfs

# Object files
PXFS_CORE_OBJ := $(PXFS_CORE_SRC:.c=.o)

# Check if FUSE is available
HAVE_FUSE := $(shell pkg-config --exists fuse 2>/dev/null && echo yes || (test -f /usr/local/include/fuse/fuse.h && echo yes) || echo no)

.PHONY: all clean install

ifeq ($(HAVE_FUSE),yes)
all: $(BINDIR) $(PXFS_FUSE) $(UCFS_FUSE) $(RF2S_FUSE) $(MKFS_PXFS) $(MOUNT_PXFS)
else
all: $(BINDIR) $(MKFS_PXFS) $(MOUNT_PXFS)
	@echo "Note: FUSE not found, skipping FUSE builds"
	@echo "Install macFUSE: brew install --cask macfuse"
endif

$(BINDIR):
	mkdir -p $(BINDIR)

# PXFS core object
$(PXFS_CORE_OBJ): $(PXFS_CORE_SRC) $(INCDIR)/fs/pxfs.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# PXFS FUSE driver
$(PXFS_FUSE): $(PXFS_FUSE_SRC) $(STEPPPS_XATTR_SRC) $(PXFS_STEPPPS_SRC) \
              $(PXFS_CORE_OBJ) $(INCDIR)/fs/pxfs.h steppps_xattr.h
	$(CC) $(CFLAGS) $(INCLUDES) $(FUSE_CFLAGS) \
		$(PXFS_FUSE_SRC) $(STEPPPS_XATTR_SRC) $(PXFS_STEPPPS_SRC) \
		$(PXFS_CORE_OBJ) -o $@ $(FUSE_LDFLAGS)

# UCFS FUSE driver
$(UCFS_FUSE): $(UCFS_FUSE_SRC)
	$(CC) $(CFLAGS) $(FUSE_CFLAGS) $< -o $@ $(FUSE_LDFLAGS)

# RF2S FUSE driver
$(RF2S_FUSE): $(RF2S_FUSE_SRC)
	$(CC) $(CFLAGS) $(FUSE_CFLAGS) $< -o $@ $(FUSE_LDFLAGS)

# mkfs.pxfs tool
$(MKFS_PXFS): $(MKFS_SRC) $(PXFS_CORE_OBJ) $(INCDIR)/fs/pxfs.h
	$(CC) $(CFLAGS) $(INCLUDES) $< $(PXFS_CORE_OBJ) -o $@

# mount.pxfs helper script
$(MOUNT_PXFS): $(TOOLSDIR)/mount.pxfs
	cp $< $@
	chmod +x $@

# Install to system directories
install: all
	install -d /usr/local/bin
	install -m 755 $(PXFS_FUSE) /usr/local/bin/
	install -m 755 $(MKFS_PXFS) /usr/local/bin/
	install -m 755 $(MOUNT_PXFS) /usr/local/bin/
	# Create /sbin symlink for mount helper
	install -d /sbin
	ln -sf /usr/local/bin/mount.pxfs /sbin/mount.pxfs || true

# Uninstall
uninstall:
	rm -f /usr/local/bin/pxfs.fuse
	rm -f /usr/local/bin/mkfs.pxfs
	rm -f /usr/local/bin/mount.pxfs
	rm -f /sbin/mount.pxfs

clean:
	rm -f *.o
	rm -f $(PXFS_FUSE) $(UCFS_FUSE) $(RF2S_FUSE) $(MKFS_PXFS) $(MOUNT_PXFS)

# Test filesystem creation and mount
test: all
	@echo "=== PXFS Test ==="
	@echo "Creating test image..."
	dd if=/dev/zero of=/tmp/test.pxfs bs=1M count=4 2>/dev/null
	$(MKFS_PXFS) -v /tmp/test.pxfs
	@echo ""
	@echo "Creating mountpoint..."
	mkdir -p /tmp/pxfs_test
	@echo ""
	@echo "Mounting filesystem (foreground)..."
	@echo "Run: $(PXFS_FUSE) -f /tmp/test.pxfs /tmp/pxfs_test"
	@echo ""
	@echo "In another terminal:"
	@echo "  ls /tmp/pxfs_test"
	@echo "  echo 'hello' > /tmp/pxfs_test/test.txt"
	@echo "  cat /tmp/pxfs_test/test.txt"
	@echo ""
	@echo "Unmount with: fusermount -u /tmp/pxfs_test"

# Quick build without FUSE (for testing core compression)
core-only: $(PXFS_CORE_OBJ)
	@echo "PXFS core built successfully"

# Compression test - run the test suite
test-compression:
	@echo "=== Compression Test ==="
	@echo "Run: cd ../../../tests/filesystem && make test"
	cd ../../../tests/filesystem && make test

.PHONY: test test-compression core-only install uninstall
