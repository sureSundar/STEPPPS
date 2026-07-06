# TernaryBit OS - Universal Shell Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -DTBOS_HOSTED -I../../../include -I../../core -I../common
LDFLAGS = -lm -liconv

# Source files
SHELL_SRCS = \
    tbos_universal_shell.c \
    commands_core.c \
    commands_week1.c \
    commands_week2.c \
    commands_week3.c \
    commands_week4.c \
    commands_week5.c \
    commands_week6.c \
    commands_week7.c \
    commands_week8.c \
    commands_week9.c \
    commands_week10.c \
    commands_pxfs.c \
    main_shell.c
SHELL_OBJS = $(SHELL_SRCS:.c=.o)

# Dependencies (stubs + network + wow + dns)
DEPS_SRCS = ../../core/stubs/tbos_ramdisk.c \
            ../../core/stubs/tbos_hal.c \
            ../../core/stubs/tbos_consciousness.c \
            ../../core/stubs/tbos_karma.c \
            ../../core/stubs/tbos_sangha.c \
            ../../core/stubs/tbos_boot_stub.c \
            ../../network/tbos_net.c \
            ../../network/wow/tbos_wow.c \
            ../../network/dns/tbos_dns.c \
            ../../../kernel/hal_hosted.c
DEPS_OBJS = $(DEPS_SRCS:.c=.o)

# Target
TARGET = tbos_shell

# Build rules
all: $(TARGET)

$(TARGET): $(SHELL_OBJS) $(DEPS_OBJS)
	@echo "🔨 Linking Universal Shell..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ Shell built successfully: ./$(TARGET)"
	@echo ""
	@echo "╔══════════════════════════════════════════════════════╗"
	@echo "║   TernaryBit OS Universal Shell - Ready!             ║"
	@echo "╠══════════════════════════════════════════════════════╣"
	@echo "║   Run: ./$(TARGET)                                   ║"
	@echo "║   Test: make test                                    ║"
	@echo "╚══════════════════════════════════════════════════════╝"
	@echo ""

%.o: %.c
	@echo "📝 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

../../core/stubs/%.o: ../../core/stubs/%.c
	@echo "📝 Compiling stub $<..."
	@mkdir -p ../../core/stubs
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo ""
	@echo "╔══════════════════════════════════════════════════════╗"
	@echo "║   Running Universal Shell                            ║"
	@echo "╚══════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Type 'help' for commands, 'exit' to quit"
	@echo ""
	./$(TARGET)

clean:
	@echo "🧹 Cleaning..."
	rm -f $(SHELL_OBJS) $(DEPS_OBJS) $(TARGET)
	@echo "✅ Clean complete"

.PHONY: all test clean
