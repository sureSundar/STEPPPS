# TernaryBit OS for TI Calculators
# Sacred mathematics meets classic Texas Instruments hardware

# TI Calculator targets
TI_MODELS = ti84ce ti89t ti83pce tinspire

# Toolchain configuration
CE_TOOLCHAIN ?= ../toolchain/ce-programming
TIGCC_PATH ?= ../toolchain/tigcc
NSPIRE_TOOLCHAIN ?= ../toolchain/ndless

# TI-84 Plus CE (ARM Cortex-M4)
CC_TI84CE = arm-none-eabi-gcc
CFLAGS_TI84CE = -mcpu=cortex-m4 -mthumb -Os -Wall
LDFLAGS_TI84CE = -T ti84ce.ld -nostdlib

# TI-89 Titanium (68000 CPU)
CC_TI89T = m68k-linux-gnu-gcc
CFLAGS_TI89T = -m68000 -Os -Wall -fomit-frame-pointer
LDFLAGS_TI89T = -T ti89t.ld -nostdlib

# TI-Nspire (ARM9)
CC_NSPIRE = arm-none-eabi-gcc
CFLAGS_NSPIRE = -mcpu=arm926ej-s -Os -Wall
LDFLAGS_NSPIRE = -T nspire.ld -nostdlib

# Common TBOS sources for calculators
TBOS_CALC_SRCS = \
    ../../kernel/calc/tbos_calc_main.c \
    ../../kernel/calc/sacred_math.c \
    ../../kernel/calc/mini_terminal.c \
    ../../kernel/calc/display_driver.c \
    ../../kernel/calc/keypad_driver.c \
    ../../kernel/core/init_minimal.c \
    ../../kernel/core/memory_calc.c \
    ../../lib/string_minimal.c \
    ../../lib/math_sacred.c

# TI-84 Plus CE specific sources
TI84CE_SRCS = \
    $(TBOS_CALC_SRCS) \
    ti84ce/boot.c \
    ti84ce/lcd_driver.c \
    ti84ce/usb_link.c \
    ti84ce/flash_storage.c \
    ti84ce/steppps_mini.c

# TI-89 Titanium specific sources
TI89T_SRCS = \
    $(TBOS_CALC_SRCS) \
    ti89t/boot68k.s \
    ti89t/grayscale_lcd.c \
    ti89t/ams_interface.c \
    ti89t/link_cable.c

# Build targets
.PHONY: all clean ti84ce ti89t ti83pce tinspire install-ti84ce install-ti89t

all: $(TI_MODELS)

# TI-84 Plus CE build
ti84ce: tbos-ti84ce.8xp

tbos-ti84ce.8xp: $(TI84CE_SRCS) ti84ce/ti84ce.ld
	@echo "🔱 Building TBOS for TI-84 Plus CE..."
	@mkdir -p build/ti84ce
	$(CC_TI84CE) $(CFLAGS_TI84CE) $(TI84CE_SRCS) -o build/ti84ce/tbos.elf $(LDFLAGS_TI84CE)
	arm-none-eabi-objcopy -O binary build/ti84ce/tbos.elf build/ti84ce/tbos.bin
	./tools/ti84ce-appvar build/ti84ce/tbos.bin tbos-ti84ce.8xp
	@echo "✅ TI-84 Plus CE build complete: tbos-ti84ce.8xp"

# TI-89 Titanium build
ti89t: tbos-ti89t.89z

tbos-ti89t.89z: $(TI89T_SRCS) ti89t/ti89t.ld
	@echo "🔱 Building TBOS for TI-89 Titanium..."
	@mkdir -p build/ti89t
	$(CC_TI89T) $(CFLAGS_TI89T) $(TI89T_SRCS) -o build/ti89t/tbos.elf $(LDFLAGS_TI89T)
	m68k-linux-gnu-objcopy -O binary build/ti89t/tbos.elf build/ti89t/tbos.bin
	./tools/ti89t-pack build/ti89t/tbos.bin tbos-ti89t.89z
	@echo "✅ TI-89 Titanium build complete: tbos-ti89t.89z"

# TI-83 Premium CE build (similar to TI-84 Plus CE)
ti83pce: tbos-ti83pce.8xp

tbos-ti83pce.8xp: $(TI84CE_SRCS) ti84ce/ti84ce.ld
	@echo "🔱 Building TBOS for TI-83 Premium CE..."
	@mkdir -p build/ti83pce
	$(CC_TI84CE) $(CFLAGS_TI84CE) -DTI83PCE $(TI84CE_SRCS) -o build/ti83pce/tbos.elf $(LDFLAGS_TI84CE)
	arm-none-eabi-objcopy -O binary build/ti83pce/tbos.elf build/ti83pce/tbos.bin
	./tools/ti84ce-appvar build/ti83pce/tbos.bin tbos-ti83pce.8xp
	@echo "✅ TI-83 Premium CE build complete: tbos-ti83pce.8xp"

# TI-Nspire build
tinspire: tbos-tinspire.tns

tbos-tinspire.tns: $(TBOS_CALC_SRCS) tinspire/boot_nspire.c tinspire/nspire.ld
	@echo "🔱 Building TBOS for TI-Nspire..."
	@mkdir -p build/tinspire
	$(CC_NSPIRE) $(CFLAGS_NSPIRE) $(TBOS_CALC_SRCS) tinspire/boot_nspire.c -o build/tinspire/tbos.elf $(LDFLAGS_NSPIRE)
	arm-none-eabi-objcopy -O binary build/tinspire/tbos.elf build/tinspire/tbos.bin
	./tools/nspire-pack build/tinspire/tbos.bin tbos-tinspire.tns
	@echo "✅ TI-Nspire build complete: tbos-tinspire.tns"

# Installation targets
install-ti84ce: tbos-ti84ce.8xp
	@echo "📤 Installing TBOS to TI-84 Plus CE..."
	tilp -s tbos-ti84ce.8xp || \
	ti84ce-link send tbos-ti84ce.8xp || \
	echo "⚠️  Please manually transfer tbos-ti84ce.8xp to calculator"

install-ti89t: tbos-ti89t.89z
	@echo "📤 Installing TBOS to TI-89 Titanium..."
	tilp -s tbos-ti89t.89z || \
	ti89t-link send tbos-ti89t.89z || \
	echo "⚠️  Please manually transfer tbos-ti89t.89z to calculator"

install-tinspire: tbos-tinspire.tns
	@echo "📤 Installing TBOS to TI-Nspire..."
	nspire-link send tbos-tinspire.tns || \
	echo "⚠️  Please manually transfer tbos-tinspire.tns to calculator"

# Emulator testing
test-ti84ce: tbos-ti84ce.8xp
	@echo "🧪 Testing TI-84 Plus CE in CEmu..."
	cemu --load tbos-ti84ce.8xp --run || echo "⚠️  CEmu not available"

test-ti89t: tbos-ti89t.89z
	@echo "🧪 Testing TI-89 Titanium in TiEmu..."
	tiemu --load tbos-ti89t.89z --run || echo "⚠️  TiEmu not available"

# Sacred calculator features
sacred-math-test:
	@echo "🕉️  Testing sacred mathematics..."
	./tools/test-vedic-math
	./tools/test-sacred-geometry
	./tools/test-consciousness-bridge

# Development tools
tools: tools/ti84ce-appvar tools/ti89t-pack tools/nspire-pack

tools/ti84ce-appvar: tools/ti84ce-appvar.c
	gcc -o tools/ti84ce-appvar tools/ti84ce-appvar.c

tools/ti89t-pack: tools/ti89t-pack.c
	gcc -o tools/ti89t-pack tools/ti89t-pack.c

tools/nspire-pack: tools/nspire-pack.c
	gcc -o tools/nspire-pack tools/nspire-pack.c

# Documentation
docs:
	@echo "📚 Generating TI calculator documentation..."
	@mkdir -p docs/ti
	./tools/generate-ti-docs > docs/ti/README.md

# Backup and recovery
backup-roms:
	@echo "💾 Backing up calculator ROMs..."
	@mkdir -p backup
	ti84ce-dump-rom backup/ti84ce-$(shell date +%Y%m%d).rom || echo "❌ TI-84 Plus CE not connected"
	ti89t-dump-rom backup/ti89t-$(shell date +%Y%m%d).rom || echo "❌ TI-89 Titanium not connected"
	nspire-dump-os backup/nspire-$(shell date +%Y%m%d).tno || echo "❌ TI-Nspire not connected"

# Clean targets
clean:
	rm -rf build/
	rm -f *.8xp *.89z *.tns
	rm -f tools/ti84ce-appvar tools/ti89t-pack tools/nspire-pack

clean-all: clean
	rm -rf backup/
	rm -rf docs/ti/

# Configuration
config:
	@echo "⚙️  TI Calculator Configuration:"
	@echo "CE_TOOLCHAIN: $(CE_TOOLCHAIN)"
	@echo "TIGCC_PATH: $(TIGCC_PATH)"
	@echo "NSPIRE_TOOLCHAIN: $(NSPIRE_TOOLCHAIN)"
	@echo ""
	@echo "Available targets: $(TI_MODELS)"

# Help
help:
	@echo "🔱 TernaryBit OS TI Calculator Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build for all TI calculator models"
	@echo "  ti84ce           Build for TI-84 Plus CE"
	@echo "  ti89t            Build for TI-89 Titanium"
	@echo "  ti83pce          Build for TI-83 Premium CE"
	@echo "  tinspire         Build for TI-Nspire"
	@echo ""
	@echo "Installation:"
	@echo "  install-ti84ce   Install to TI-84 Plus CE"
	@echo "  install-ti89t    Install to TI-89 Titanium"
	@echo "  install-tinspire Install to TI-Nspire"
	@echo ""
	@echo "Testing:"
	@echo "  test-ti84ce      Test in CEmu emulator"
	@echo "  test-ti89t       Test in TiEmu emulator"
	@echo "  sacred-math-test Test sacred mathematics"
	@echo ""
	@echo "Utilities:"
	@echo "  tools            Build development tools"
	@echo "  backup-roms      Backup calculator ROMs"
	@echo "  docs             Generate documentation"
	@echo "  clean            Clean build files"
	@echo "  config           Show configuration"
	@echo ""
	@echo "🕉️ Sacred Mathematics + TI Hardware = Digital Transcendence"