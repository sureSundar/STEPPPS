# TernaryBit OS for Casio Calculators
# Sacred mathematics for Casio hardware platforms

# Casio calculator targets
CASIO_MODELS = casio-prizm casio-classpad casio-fx9860

# Toolchain paths
LIBFXCG_PATH ?= ../toolchain/libfxcg
SH_TOOLCHAIN ?= /usr/bin/sh4-linux-gnu-
ARM_TOOLCHAIN ?= /usr/bin/arm-linux-gnueabi-

# Casio Prizm fx-CG50 (SH4A-4A CPU)
CC_PRIZM = $(SH_TOOLCHAIN)gcc
CFLAGS_PRIZM = -m4a-nofpu -mb -Os -Wall -Wextra -I$(LIBFXCG_PATH)/include
LDFLAGS_PRIZM = -T $(LIBFXCG_PATH)/toolchain/prizm.ld -Wl,-Map=prizm.map -L$(LIBFXCG_PATH)/lib -lfxcg

# Casio ClassPad II (ARM926EJ-S)
CC_CLASSPAD = $(ARM_TOOLCHAIN)gcc
CFLAGS_CLASSPAD = -mcpu=arm926ej-s -Os -Wall -Wextra
LDFLAGS_CLASSPAD = -T classpad.ld -Wl,-Map=classpad.map

# Casio fx-9860GII (SH3 CPU)
CC_FX9860 = $(SH_TOOLCHAIN)gcc
CFLAGS_FX9860 = -m3 -mb -Os -Wall -Wextra
LDFLAGS_FX9860 = -T fx9860.ld -Wl,-Map=fx9860.map

# Common TBOS sources for Casio calculators
TBOS_CASIO_SRCS = \
    ../../kernel/calc/tbos_calc_main.c \
    ../../kernel/calc/sacred_math.c \
    ../../kernel/calc/mini_terminal.c \
    ../../kernel/calc/display_driver.c \
    ../../kernel/calc/keypad_driver.c \
    ../../kernel/core/init_minimal.c \
    ../../kernel/core/memory_calc.c \
    ../../lib/string_minimal.c \
    ../../lib/math_sacred.c \
    casio_common/casio_hardware.c \
    casio_common/casio_sacred.c \
    casio_common/casio_steppps.c

# Casio Prizm specific sources
PRIZM_SRCS = \
    $(TBOS_CASIO_SRCS) \
    prizm/prizm_boot.c \
    prizm/prizm_lcd.c \
    prizm/prizm_keypad.c \
    prizm/prizm_filesystem.c \
    prizm/prizm_sacred_graphics.c \
    prizm/prizm_consciousness.c

# Casio ClassPad specific sources
CLASSPAD_SRCS = \
    $(TBOS_CASIO_SRCS) \
    classpad/classpad_boot.c \
    classpad/classpad_touchscreen.c \
    classpad/classpad_graphics.c \
    classpad/classpad_cas_integration.c

# Casio fx-9860GII specific sources
FX9860_SRCS = \
    $(TBOS_CASIO_SRCS) \
    fx9860/fx9860_boot.c \
    fx9860/fx9860_mono_display.c \
    fx9860/fx9860_keypad.c \
    fx9860/fx9860_sacred_text.c

# Build targets
.PHONY: all clean casio-prizm casio-classpad casio-fx9860 install-prizm install-classpad install-fx9860

all: $(CASIO_MODELS)

# Casio Prizm fx-CG50 build
casio-prizm: tbos-prizm.g3a

tbos-prizm.g3a: $(PRIZM_SRCS) prizm/prizm.ld
	@echo "🔱 Building TBOS for Casio Prizm fx-CG50..."
	@mkdir -p build/prizm
	$(CC_PRIZM) $(CFLAGS_PRIZM) $(PRIZM_SRCS) -o build/prizm/tbos.elf $(LDFLAGS_PRIZM)
	$(SH_TOOLCHAIN)objcopy -O binary build/prizm/tbos.elf build/prizm/tbos.bin
	./tools/mkg3a build/prizm/tbos.bin tbos-prizm.g3a "TernaryBit OS" "TBOS" "@TBOS"
	@echo "✅ Casio Prizm build complete: tbos-prizm.g3a"

# Casio ClassPad II build
casio-classpad: tbos-classpad.cpa

tbos-classpad.cpa: $(CLASSPAD_SRCS) classpad/classpad.ld
	@echo "🔱 Building TBOS for Casio ClassPad II..."
	@mkdir -p build/classpad
	$(CC_CLASSPAD) $(CFLAGS_CLASSPAD) $(CLASSPAD_SRCS) -o build/classpad/tbos.elf $(LDFLAGS_CLASSPAD)
	$(ARM_TOOLCHAIN)objcopy -O binary build/classpad/tbos.elf build/classpad/tbos.bin
	./tools/mkcpa build/classpad/tbos.bin tbos-classpad.cpa "TernaryBit OS"
	@echo "✅ Casio ClassPad build complete: tbos-classpad.cpa"

# Casio fx-9860GII build
casio-fx9860: tbos-fx9860.g1a

tbos-fx9860.g1a: $(FX9860_SRCS) fx9860/fx9860.ld
	@echo "🔱 Building TBOS for Casio fx-9860GII..."
	@mkdir -p build/fx9860
	$(CC_FX9860) $(CFLAGS_FX9860) $(FX9860_SRCS) -o build/fx9860/tbos.elf $(LDFLAGS_FX9860)
	$(SH_TOOLCHAIN)objcopy -O binary build/fx9860/tbos.elf build/fx9860/tbos.bin
	./tools/mkg1a build/fx9860/tbos.bin tbos-fx9860.g1a "TBOS" "TernaryBitOS"
	@echo "✅ Casio fx-9860GII build complete: tbos-fx9860.g1a"

# Bootloader versions (advanced)
casio-prizm-boot: tbos-prizm-boot.bin

tbos-prizm-boot.bin: $(PRIZM_SRCS) prizm/prizm_bootloader.ld
	@echo "🔱 Building TBOS bootloader for Casio Prizm..."
	@mkdir -p build/prizm-boot
	$(CC_PRIZM) $(CFLAGS_PRIZM) -DTBOS_BOOTLOADER $(PRIZM_SRCS) -o build/prizm-boot/tbos.elf -T prizm/prizm_bootloader.ld
	$(SH_TOOLCHAIN)objcopy -O binary build/prizm-boot/tbos.elf tbos-prizm-boot.bin
	@echo "✅ Casio Prizm bootloader complete: tbos-prizm-boot.bin"

# Full OS replacement (expert level)
casio-prizm-full: tbos-prizm-full.bin

tbos-prizm-full.bin: $(PRIZM_SRCS) prizm/prizm_full_os.ld
	@echo "🔱 Building TBOS full OS for Casio Prizm..."
	@mkdir -p build/prizm-full
	$(CC_PRIZM) $(CFLAGS_PRIZM) -DTBOS_FULL_OS $(PRIZM_SRCS) -o build/prizm-full/tbos.elf -T prizm/prizm_full_os.ld
	$(SH_TOOLCHAIN)objcopy -O binary build/prizm-full/tbos.elf tbos-prizm-full.bin
	@echo "✅ Casio Prizm full OS complete: tbos-prizm-full.bin"

# Installation targets
install-prizm: tbos-prizm.g3a
	@echo "📤 Installing TBOS to Casio Prizm..."
	./casio-install.sh --model prizm --method app --file tbos-prizm.g3a

install-classpad: tbos-classpad.cpa
	@echo "📤 Installing TBOS to Casio ClassPad..."
	./casio-install.sh --model classpad --method app --file tbos-classpad.cpa

install-fx9860: tbos-fx9860.g1a
	@echo "📤 Installing TBOS to Casio fx-9860GII..."
	./casio-install.sh --model fx9860 --method app --file tbos-fx9860.g1a

# Emulator testing
test-prizm: tbos-prizm.g3a
	@echo "🧪 Testing Casio Prizm in emulator..."
	./tools/prizm-emulator --load tbos-prizm.g3a --run || echo "⚠️  Prizm emulator not available"

test-classpad: tbos-classpad.cpa
	@echo "🧪 Testing Casio ClassPad in emulator..."
	./tools/classpad-emulator --load tbos-classpad.cpa --run || echo "⚠️  ClassPad emulator not available"

test-fx9860: tbos-fx9860.g1a
	@echo "🧪 Testing Casio fx-9860GII in emulator..."
	./tools/fx9860-emulator --load tbos-fx9860.g1a --run || echo "⚠️  fx-9860GII emulator not available"

# Sacred mathematics testing
sacred-test:
	@echo "🕉️  Testing sacred mathematics on Casio..."
	./tools/test-casio-sacred-math
	./tools/test-casio-consciousness
	./tools/test-casio-steppps

# Development tools
tools: tools/mkg3a tools/mkcpa tools/mkg1a tools/casio-install.sh

tools/mkg3a: tools/mkg3a.c
	gcc -o tools/mkg3a tools/mkg3a.c -lz

tools/mkcpa: tools/mkcpa.c
	gcc -o tools/mkcpa tools/mkcpa.c

tools/mkg1a: tools/mkg1a.c
	gcc -o tools/mkg1a tools/mkg1a.c

tools/casio-install.sh: tools/casio-install.sh.template
	cp tools/casio-install.sh.template tools/casio-install.sh
	chmod +x tools/casio-install.sh

# Backup and recovery tools
backup-tools: tools/casio-backup.sh tools/casio-restore.sh

tools/casio-backup.sh: tools/casio-backup.sh.template
	cp tools/casio-backup.sh.template tools/casio-backup.sh
	chmod +x tools/casio-backup.sh

tools/casio-restore.sh: tools/casio-restore.sh.template
	cp tools/casio-restore.sh.template tools/casio-restore.sh
	chmod +x tools/casio-restore.sh

# Documentation
docs:
	@echo "📚 Generating Casio calculator documentation..."
	@mkdir -p docs/casio
	./tools/generate-casio-docs > docs/casio/README.md

# Configuration
config:
	@echo "⚙️  Casio Calculator Configuration:"
	@echo "LIBFXCG_PATH: $(LIBFXCG_PATH)"
	@echo "SH_TOOLCHAIN: $(SH_TOOLCHAIN)"
	@echo "ARM_TOOLCHAIN: $(ARM_TOOLCHAIN)"
	@echo ""
	@echo "Available targets: $(CASIO_MODELS)"
	@echo ""
	@echo "Sacred STEPPPS Framework: Enabled"
	@echo "Hindu Mathematics: Integrated"
	@echo "Consciousness Bridge: Active"

# Clean targets
clean:
	rm -rf build/
	rm -f *.g3a *.cpa *.g1a *.bin
	rm -f tools/mkg3a tools/mkcpa tools/mkg1a
	rm -f tools/casio-install.sh tools/casio-backup.sh tools/casio-restore.sh

clean-all: clean
	rm -rf docs/casio/
	rm -rf backup/

# Flash tools (advanced users only)
flash-prizm: tbos-prizm-boot.bin
	@echo "⚠️  ADVANCED: Flashing TBOS to Casio Prizm bootloader"
	@echo "This requires Ftune or Simon's bootloader installed!"
	@echo "Press [ENTER] to continue or [Ctrl+C] to abort..."
	@read
	./tools/casio-flash.sh --model prizm --bootloader ftune --file tbos-prizm-boot.bin

flash-full-prizm: tbos-prizm-full.bin
	@echo "🚨 EXPERT LEVEL: Full OS replacement!"
	@echo "This will completely replace Casio OS with TBOS!"
	@echo "Have recovery plan ready! Press [ENTER] to continue or [Ctrl+C] to abort..."
	@read
	./tools/casio-flash.sh --model prizm --method full --file tbos-prizm-full.bin

# Help
help:
	@echo "🔱 TernaryBit OS Casio Calculator Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all                 Build for all Casio calculator models"
	@echo "  casio-prizm         Build for Casio Prizm fx-CG50"
	@echo "  casio-classpad      Build for Casio ClassPad II"
	@echo "  casio-fx9860        Build for Casio fx-9860GII"
	@echo ""
	@echo "Installation:"
	@echo "  install-prizm       Install to Casio Prizm"
	@echo "  install-classpad    Install to Casio ClassPad"
	@echo "  install-fx9860      Install to Casio fx-9860GII"
	@echo ""
	@echo "Advanced:"
	@echo "  casio-prizm-boot    Build bootloader version"
	@echo "  casio-prizm-full    Build full OS replacement"
	@echo "  flash-prizm         Flash to bootloader (RISKY)"
	@echo "  flash-full-prizm    Full OS flash (VERY RISKY)"
	@echo ""
	@echo "Testing:"
	@echo "  test-prizm          Test in Prizm emulator"
	@echo "  test-classpad       Test in ClassPad emulator"
	@echo "  test-fx9860         Test in fx-9860GII emulator"
	@echo "  sacred-test         Test sacred mathematics"
	@echo ""
	@echo "Utilities:"
	@echo "  tools               Build development tools"
	@echo "  backup-tools        Build backup/recovery tools"
	@echo "  docs                Generate documentation"
	@echo "  clean               Clean build files"
	@echo "  config              Show configuration"
	@echo ""
	@echo "🕉️ Sacred Mathematics + Casio Hardware = Digital Enlightenment"