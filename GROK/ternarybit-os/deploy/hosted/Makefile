CC ?= cc
BUILD_DIR ?= build
GUI_DIR := ../../src/gui

CPPFLAGS += -I$(GUI_DIR)
CFLAGS ?= -O2 -g
CFLAGS += -std=c11 -Wall -Wextra
LDLIBS += -lm

COMMON_SOURCES := \
	$(GUI_DIR)/tbos_pixel.c \
	$(GUI_DIR)/tbos_gui.c \
	$(GUI_DIR)/tbos_desktop.c \
	$(GUI_DIR)/tbos_desktop_demo.c

SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL2_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)

.PHONY: all headless desktop check clean install

all: headless

$(BUILD_DIR):
	mkdir -p $@

headless: $(BUILD_DIR)/tbos-desktop-headless

$(BUILD_DIR)/tbos-desktop-headless: $(COMMON_SOURCES) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(COMMON_SOURCES) $(LDLIBS)

desktop: $(BUILD_DIR)/tbos-desktop

$(BUILD_DIR)/tbos-desktop: $(COMMON_SOURCES) $(GUI_DIR)/tbos_platform_sdl.c | $(BUILD_DIR)
	@test -n "$(SDL2_LIBS)" || { echo "SDL2 development files not found (pkg-config sdl2)" >&2; exit 1; }
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SDL2_CFLAGS) -DTBOS_USE_SDL2 \
		-o $@ $(COMMON_SOURCES) $(GUI_DIR)/tbos_platform_sdl.c $(LDLIBS) $(SDL2_LIBS)

check: headless
	cd $(BUILD_DIR) && ./tbos-desktop-headless
	@test -s $(BUILD_DIR)/tbos_desktop_dark.ppm
	@test -s $(BUILD_DIR)/tbos_desktop_light.ppm
	@test -s $(BUILD_DIR)/tbos_desktop_saffron.ppm

install: desktop
	install -Dm755 $(BUILD_DIR)/tbos-desktop $(DESTDIR)/usr/bin/tbos-desktop
	install -Dm755 tbos-desktop-session $(DESTDIR)/usr/bin/tbos-desktop-session
	install -Dm644 tbos-desktop.desktop $(DESTDIR)/usr/share/applications/tbos-desktop.desktop

clean:
	rm -rf $(BUILD_DIR)
