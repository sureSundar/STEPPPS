# Rock Solid OS Components Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I. -I.. -pthread
LDFLAGS = -pthread

# Source files
ERROR_SRCS = error/error.c
PANIC_SRCS = panic/panic.c
WATCHDOG_SRCS = watchdog/watchdog.c
HEALTH_SRCS = health/health.c
DAEMON_SRCS = daemon/tbosd.c

# Object files
ERROR_OBJS = $(ERROR_SRCS:.c=.o)
PANIC_OBJS = $(PANIC_SRCS:.c=.o)
WATCHDOG_OBJS = $(WATCHDOG_SRCS:.c=.o)
HEALTH_OBJS = $(HEALTH_SRCS:.c=.o)
DAEMON_OBJS = $(DAEMON_SRCS:.c=.o)

# Targets
LIB_ROCK = librocksolid.a
TBOSD = tbosd

.PHONY: all clean

all: $(LIB_ROCK) $(TBOSD)

$(LIB_ROCK): $(ERROR_OBJS) $(PANIC_OBJS) $(WATCHDOG_OBJS) $(HEALTH_OBJS)
	ar rcs $@ $^

$(TBOSD): $(DAEMON_OBJS) $(LIB_ROCK)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(ERROR_OBJS) $(PANIC_OBJS) $(WATCHDOG_OBJS) $(HEALTH_OBJS) $(DAEMON_OBJS)
	rm -f $(LIB_ROCK) $(TBOSD)

# Dependencies
error/error.o: error/error.h
panic/panic.o: panic/panic.h error/error.h
watchdog/watchdog.o: watchdog/watchdog.h error/error.h
health/health.o: health/health.h error/error.h
daemon/tbosd.o: error/error.h panic/panic.h watchdog/watchdog.h health/health.h
