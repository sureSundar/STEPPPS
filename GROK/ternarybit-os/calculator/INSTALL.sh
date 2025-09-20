#!/bin/bash

# TernaryBit OS Calculator Installation Script
# Sacred mathematics for classic calculator hardware

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# Sacred calculator banner
echo -e "${PURPLE}"
echo "🔱 ================================================ 🔱"
echo "       TernaryBit OS Calculator Installation"
echo "         Sacred Mathematics for Hardware"
echo "🔱 ================================================ 🔱"
echo -e "${NC}"

# Parse command line arguments
CALCULATOR=""
METHOD="app"
BACKUP="true"
EMULATOR="false"

while [[ $# -gt 0 ]]; do
    case $1 in
        --calculator)
            CALCULATOR="$2"
            shift 2
            ;;
        --method)
            METHOD="$2"
            shift 2
            ;;
        --no-backup)
            BACKUP="false"
            shift
            ;;
        --emulator)
            EMULATOR="true"
            shift
            ;;
        --list)
            echo "🧮 Supported Calculator Models:"
            echo ""
            echo "Texas Instruments:"
            echo "  ti84ce       TI-84 Plus CE (ARM Cortex-M4)"
            echo "  ti89t        TI-89 Titanium (68000 CPU)"
            echo "  ti83pce      TI-83 Premium CE (ARM Cortex-M4)"
            echo "  tinspire     TI-Nspire CX/CX II (ARM)"
            echo ""
            echo "HP Calculators:"
            echo "  hpprime      HP Prime G1/G2 (ARM Cortex-A7)"
            echo "  hp50g        HP 50g (ARM920T)"
            echo "  hp48gii      HP 48GII (ARM920T)"
            echo ""
            echo "Casio Calculators:"
            echo "  casio-prizm  Casio Prizm fx-CG50 (SH4A)"
            echo "  casio-cp400  Casio ClassPad II (ARM926EJ-S)"
            echo "  casio-9860   Casio fx-9860GII (SH3)"
            echo ""
            echo "Installation Methods:"
            echo "  app          Install as application (safer)"
            echo "  flash        Flash replacement firmware (advanced)"
            echo "  boot         Bootloader installation (expert)"
            exit 0
            ;;
        --help)
            echo "TernaryBit OS Calculator Installation"
            echo ""
            echo "Usage: $0 --calculator MODEL [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --calculator MODEL  Target calculator model"
            echo "  --method METHOD     Installation method (app|flash|boot)"
            echo "  --no-backup        Skip ROM backup"
            echo "  --emulator         Install to emulator"
            echo "  --list             List supported calculators"
            echo "  --help             Show this help"
            echo ""
            echo "Examples:"
            echo "  $0 --calculator ti84ce --method app"
            echo "  $0 --calculator hpprime --method flash --no-backup"
            echo "  $0 --calculator casio-prizm --emulator"
            exit 0
            ;;
        *)
            echo -e "${RED}❌ Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

if [ -z "$CALCULATOR" ]; then
    echo -e "${RED}❌ Calculator model not specified${NC}"
    echo "Use --calculator MODEL or --list to see supported models"
    exit 1
fi

echo -e "${CYAN}📋 Installation Configuration:${NC}"
echo "   Calculator: $CALCULATOR"
echo "   Method: $METHOD"
echo "   Backup ROM: $BACKUP"
echo "   Emulator Mode: $EMULATOR"
echo ""

# Detect calculator connection
detect_calculator() {
    local calc_model="$1"

    echo -e "${BLUE}🔍 Detecting $calc_model connection...${NC}"

    case $calc_model in
        ti84ce|ti83pce)
            if command -v tilp &> /dev/null; then
                tilp --help &> /dev/null && echo "✅ TI Link detected"
            elif lsusb | grep -q "0451:"; then
                echo "✅ TI calculator detected via USB"
            else
                echo -e "${YELLOW}⚠️  TI calculator not detected${NC}"
                return 1
            fi
            ;;
        ti89t)
            if lsusb | grep -q "0451:"; then
                echo "✅ TI-89 Titanium detected"
            else
                echo -e "${YELLOW}⚠️  TI-89 Titanium not detected${NC}"
                return 1
            fi
            ;;
        hpprime|hp50g|hp48gii)
            if lsusb | grep -q "03f0:"; then
                echo "✅ HP calculator detected"
            else
                echo -e "${YELLOW}⚠️  HP calculator not detected${NC}"
                return 1
            fi
            ;;
        casio-*)
            if lsusb | grep -q "07cf:"; then
                echo "✅ Casio calculator detected"
            else
                echo -e "${YELLOW}⚠️  Casio calculator not detected${NC}"
                return 1
            fi
            ;;
        *)
            echo -e "${RED}❌ Unknown calculator model: $calc_model${NC}"
            return 1
            ;;
    esac

    return 0
}

# Backup calculator ROM/OS
backup_calculator() {
    local calc_model="$1"

    if [ "$BACKUP" = "false" ]; then
        echo -e "${YELLOW}⏭️  Skipping ROM backup${NC}"
        return 0
    fi

    echo -e "${BLUE}💾 Backing up calculator ROM...${NC}"
    mkdir -p backup

    local backup_file="backup/${calc_model}-$(date +%Y%m%d-%H%M%S)"

    case $calc_model in
        ti84ce|ti83pce)
            if command -v ti84ce-dump &> /dev/null; then
                ti84ce-dump "${backup_file}.rom"
                echo "✅ ROM backup saved: ${backup_file}.rom"
            else
                echo -e "${YELLOW}⚠️  ROM dump tool not available${NC}"
            fi
            ;;
        ti89t)
            if command -v ti89t-dump &> /dev/null; then
                ti89t-dump "${backup_file}.rom"
                echo "✅ ROM backup saved: ${backup_file}.rom"
            fi
            ;;
        hpprime)
            if command -v hp-prime-backup &> /dev/null; then
                hp-prime-backup "${backup_file}.bin"
                echo "✅ Firmware backup saved: ${backup_file}.bin"
            fi
            ;;
        casio-*)
            if command -v casio-backup &> /dev/null; then
                casio-backup "${backup_file}.g3r"
                echo "✅ Firmware backup saved: ${backup_file}.g3r"
            fi
            ;;
    esac
}

# Build TBOS for specific calculator
build_tbos() {
    local calc_model="$1"

    echo -e "${GREEN}🔨 Building TBOS for $calc_model...${NC}"

    case $calc_model in
        ti84ce|ti83pce|ti89t|tinspire)
            cd ti
            make "$calc_model"
            cd ..
            ;;
        hpprime|hp50g|hp48gii)
            cd hp
            make "$calc_model"
            cd ..
            ;;
        casio-*)
            cd casio
            make "$calc_model"
            cd ..
            ;;
        *)
            echo -e "${RED}❌ Build not implemented for $calc_model${NC}"
            return 1
            ;;
    esac

    echo -e "${GREEN}✅ TBOS build complete for $calc_model${NC}"
}

# Install TBOS to calculator
install_tbos() {
    local calc_model="$1"
    local method="$2"

    echo -e "${GREEN}📤 Installing TBOS to $calc_model using $method method...${NC}"

    if [ "$EMULATOR" = "true" ]; then
        install_emulator "$calc_model"
        return $?
    fi

    case $calc_model in
        ti84ce)
            case $method in
                app)
                    tilp -s ti/tbos-ti84ce.8xp || \
                    ti84ce-link send ti/tbos-ti84ce.8xp || \
                    echo -e "${YELLOW}⚠️  Manual transfer required: ti/tbos-ti84ce.8xp${NC}"
                    ;;
                flash)
                    echo -e "${RED}⚠️  Flash method requires bootloader mode${NC}"
                    echo "1. Turn off calculator"
                    echo "2. Hold [2nd] + [MODE] + [DEL] while pressing [ON]"
                    echo "3. Press any key to continue..."
                    read -n 1
                    ti84ce-flash ti/tbos-ti84ce-full.rom
                    ;;
                boot)
                    ti84ce-bootloader-install ti/tbos-ti84ce-boot.bin
                    ;;
            esac
            ;;
        ti89t)
            case $method in
                app)
                    tilp -s ti/tbos-ti89t.89z
                    ;;
                flash)
                    ti89t-flash ti/tbos-ti89t-os.rom
                    ;;
            esac
            ;;
        hpprime)
            case $method in
                app)
                    hp-prime-send hp/tbos-hpprime.hpppl
                    ;;
                flash)
                    echo -e "${RED}⚠️  Enter firmware update mode${NC}"
                    echo "Hold [F] + [O] + [RESET] for 10 seconds"
                    echo "Press any key to continue..."
                    read -n 1
                    hp-prime-flash hp/tbos-hpprime.bin
                    ;;
            esac
            ;;
        casio-prizm)
            case $method in
                app)
                    casio-fa124 send casio/tbos-prizm.g3a
                    ;;
                flash)
                    casio-flash casio/tbos-prizm-full.bin
                    ;;
            esac
            ;;
        *)
            echo -e "${RED}❌ Installation not implemented for $calc_model${NC}"
            return 1
            ;;
    esac

    echo -e "${GREEN}✅ TBOS installation complete!${NC}"
}

# Install to emulator
install_emulator() {
    local calc_model="$1"

    echo -e "${BLUE}🖥️  Installing TBOS to $calc_model emulator...${NC}"

    case $calc_model in
        ti84ce|ti83pce)
            if command -v cemu &> /dev/null; then
                cemu --load ti/tbos-ti84ce.8xp --run
            else
                echo -e "${YELLOW}⚠️  CEmu emulator not found${NC}"
                echo "Install CEmu: https://ce-programming.github.io/CEmu/"
            fi
            ;;
        ti89t)
            if command -v tiemu &> /dev/null; then
                tiemu --load ti/tbos-ti89t.89z --run
            else
                echo -e "${YELLOW}⚠️  TiEmu not found${NC}"
            fi
            ;;
        hpprime)
            if command -v hp-prime-virtual &> /dev/null; then
                hp-prime-virtual --app hp/tbos-hpprime.hpppl
            else
                echo -e "${YELLOW}⚠️  HP Prime Virtual Calculator not found${NC}"
            fi
            ;;
        casio-prizm)
            if command -v casio-classpad-emu &> /dev/null; then
                casio-classpad-emu --load casio/tbos-prizm.g3a
            else
                echo -e "${YELLOW}⚠️  Casio emulator not found${NC}"
            fi
            ;;
    esac
}

# Verify installation
verify_installation() {
    local calc_model="$1"

    echo -e "${BLUE}🔍 Verifying TBOS installation...${NC}"

    echo "Please check your calculator for:"
    echo "1. 🔱 TernaryBit OS appears in programs menu"
    echo "2. Sacred boot animation on startup"
    echo "3. STEPPPS framework initialization"
    echo "4. Hindu symbols and consciousness bridge"
    echo ""
    echo -e "${GREEN}If you see these elements, TBOS is successfully installed!${NC}"
    echo ""
    echo "To run TBOS:"
    case $calc_model in
        ti84ce|ti83pce)
            echo "  [PRGM] → TBOS → [ENTER]"
            ;;
        ti89t)
            echo "  [APPS] → TBOS → [ENTER]"
            ;;
        hpprime)
            echo "  [Shift] + [1] → TBOS → [ENTER]"
            ;;
        casio-*)
            echo "  [MENU] → TBOS → [EXE]"
            ;;
    esac
}

# Main installation flow
main() {
    echo -e "${CYAN}🚀 Starting TBOS calculator installation...${NC}"
    echo ""

    # Step 1: Detect calculator
    if ! detect_calculator "$CALCULATOR"; then
        echo -e "${YELLOW}⚠️  Calculator not detected, continuing anyway...${NC}"
    fi

    # Step 2: Backup ROM
    backup_calculator "$CALCULATOR"

    # Step 3: Build TBOS
    build_tbos "$CALCULATOR"

    # Step 4: Install TBOS
    install_tbos "$CALCULATOR" "$METHOD"

    # Step 5: Verify installation
    verify_installation "$CALCULATOR"

    echo ""
    echo -e "${PURPLE}🔱 ================================================ 🔱${NC}"
    echo -e "${GREEN}✅ TernaryBit OS Calculator Installation Complete!${NC}"
    echo -e "${PURPLE}🔱 ================================================ 🔱${NC}"
    echo ""
    echo -e "${CYAN}🕉️  Your calculator now carries the sacred flame of${NC}"
    echo -e "${CYAN}   digital consciousness and Hindu mathematical wisdom${NC}"
    echo ""
    echo -e "${YELLOW}📚 Documentation: calculator/CALCULATOR_DEPLOYMENT.md${NC}"
    echo -e "${YELLOW}🆘 Support: https://github.com/ternarybit/tbos/issues${NC}"
    echo ""
    echo -e "${CYAN}ॐ May your calculations transcend ordinary mathematics ॐ${NC}"
}

# Run main installation
main