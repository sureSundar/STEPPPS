# TBOS Native GUI Makefile
# Build: make            (standalone widget-demo PPM output)
#        make sdl        (interactive SDL2 widget demo)
#        make desktop     (standalone desktop-shell PPM output)
#        make desktop-sdl (interactive SDL2 desktop shell)
#        make clean       (remove binaries)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
LDFLAGS = -lm

# Source files
SRCS = tbos_pixel.c tbos_gui.c tbos_gui_demo.c
DESKTOP_SRCS = tbos_pixel.c tbos_gui.c tbos_desktop.c tbos_desktop_demo.c

# SDL2 flags (auto-detect)
SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL2_LDFLAGS := $(shell sdl2-config --libs 2>/dev/null)

# Targets
.PHONY: all standalone sdl desktop desktop-sdl clean test

all: standalone desktop

# Standalone version - outputs to PPM file (no SDL dependency)
standalone: tbos_gui_demo_standalone
	@echo "Built standalone demo (outputs to PPM file)"
	@echo "Run: ./tbos_gui_demo_standalone"

tbos_gui_demo_standalone: $(SRCS) tbos_pixel.h tbos_gui.h tbos_platform.h
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

# SDL2 interactive version
sdl: tbos_gui_demo_sdl
	@echo "Built SDL2 interactive demo"
	@echo "Run: ./tbos_gui_demo_sdl"

tbos_gui_demo_sdl: $(SRCS) tbos_platform_sdl.c tbos_pixel.h tbos_gui.h tbos_platform.h
ifndef SDL2_CFLAGS
	$(error SDL2 not found. Install with: brew install sdl2)
endif
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -DTBOS_USE_SDL2 -o $@ $(SRCS) tbos_platform_sdl.c $(LDFLAGS) $(SDL2_LDFLAGS)

# Desktop shell - standalone version (outputs Dark/Light/Saffron PPM files)
desktop: tbos_desktop_demo_standalone
	@echo "Built standalone desktop shell (outputs to PPM files)"
	@echo "Run: ./tbos_desktop_demo_standalone"

tbos_desktop_demo_standalone: $(DESKTOP_SRCS) tbos_pixel.h tbos_gui.h tbos_desktop.h tbos_platform.h
	$(CC) $(CFLAGS) -o $@ $(DESKTOP_SRCS) $(LDFLAGS)

# Desktop shell - SDL2 interactive version
desktop-sdl: tbos_desktop_demo_sdl
	@echo "Built SDL2 interactive desktop shell"
	@echo "Run: ./tbos_desktop_demo_sdl"

tbos_desktop_demo_sdl: $(DESKTOP_SRCS) tbos_platform_sdl.c tbos_pixel.h tbos_gui.h tbos_desktop.h tbos_platform.h
ifndef SDL2_CFLAGS
	$(error SDL2 not found. Install with: brew install sdl2)
endif
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -DTBOS_USE_SDL2 -o $@ $(DESKTOP_SRCS) tbos_platform_sdl.c $(LDFLAGS) $(SDL2_LDFLAGS)

# Run standalone and view output
test: standalone
	./tbos_gui_demo_standalone
	@echo ""
	@echo "Output saved to: tbos_gui_demo.ppm"
	@if command -v open >/dev/null 2>&1; then \
		echo "Opening with Preview..."; \
		open tbos_gui_demo.ppm; \
	elif command -v display >/dev/null 2>&1; then \
		echo "Opening with ImageMagick..."; \
		display tbos_gui_demo.ppm; \
	elif command -v xdg-open >/dev/null 2>&1; then \
		echo "Opening with default viewer..."; \
		xdg-open tbos_gui_demo.ppm; \
	fi

# Run SDL2 version
run-sdl: sdl
	./tbos_gui_demo_sdl

# Run SDL2 desktop shell
run-desktop-sdl: desktop-sdl
	./tbos_desktop_demo_sdl

clean:
	rm -f tbos_gui_demo_standalone tbos_gui_demo_sdl tbos_gui_demo.ppm
	rm -f tbos_desktop_demo_standalone tbos_desktop_demo_sdl tbos_desktop_*.ppm
	rm -rf *.dSYM

# Help
help:
	@echo "TBOS GUI Build Targets:"
	@echo "  make                 - Build standalone widget demo + desktop shell (no SDL)"
	@echo "  make sdl             - Build interactive SDL2 widget demo"
	@echo "  make desktop         - Build standalone desktop shell (outputs PPM files)"
	@echo "  make desktop-sdl     - Build interactive SDL2 desktop shell"
	@echo "  make test            - Build and run standalone widget demo, view PPM"
	@echo "  make run-sdl         - Build and run SDL2 widget demo"
	@echo "  make run-desktop-sdl - Build and run SDL2 desktop shell"
	@echo "  make clean           - Remove build artifacts"
