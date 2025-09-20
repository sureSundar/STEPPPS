#!/bin/bash

# TernaryBit OS Casio Calculator Installation Script
# Sacred mathematics installation for Casio hardware

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# Sacred Casio banner
echo -e "${PURPLE}"
echo "🔱 ============================================ 🔱"
echo "      TernaryBit OS Casio Installation"
echo "        Sacred Mathematics for Casio"
echo "🔱 ============================================ 🔱"
echo -e "${NC}"

# Default values
MODEL=""
METHOD="app"
FILE=""
BACKUP="true"
EMULATOR="false"
CABLE="usb"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --model)
            MODEL="$2"
            shift 2
            ;;
        --method)
            METHOD="$2"
            shift 2
            ;;
        --file)
            FILE="$2"
            shift 2
            ;;
        --cable)
            CABLE="$2"
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
        --help)
            echo "TernaryBit OS Casio Calculator Installation"
            echo ""
            echo "Usage: $0 --model MODEL [OPTIONS]"
            echo ""
            echo "Models:"
            echo "  prizm              Casio Prizm fx-CG50/fx-CG500"
            echo "  classpad           Casio ClassPad II fx-CP400"
            echo "  fx9860             Casio fx-9860GII series"
            echo ""
            echo "Methods:"
            echo "  app                Install as application (default, safer)"
            echo "  flash              Flash to bootloader (advanced)"
            echo "  full               Full OS replacement (expert)"
            echo ""
            echo "Options:"
            echo "  --file FILE        Specific file to install"
            echo "  --cable TYPE       Connection type (usb|fa124)"
            echo "  --no-backup       Skip ROM backup"
            echo "  --emulator        Install to emulator instead"
            echo "  --help            Show this help"
            echo ""
            echo "Examples:"
            echo "  $0 --model prizm --method app"
            echo "  $0 --model fx9860 --cable fa124"
            echo "  $0 --model classpad --emulator"
            exit 0
            ;;
        *)
            echo -e "${RED}❌ Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

if [ -z "$MODEL" ]; then
    echo -e "${RED}❌ Model not specified${NC}"
    echo "Use --model MODEL or --help for usage"
    exit 1
fi

echo -e "${CYAN}📋 Installation Configuration:${NC}"
echo "   Model: $MODEL"
echo "   Method: $METHOD"
echo "   Cable: $CABLE"
echo "   Backup: $BACKUP"
echo "   Emulator: $EMULATOR"
if [ -n "$FILE" ]; then
    echo "   File: $FILE"
fi
echo ""

# Detect calculator connection
detect_casio_calculator() {
    local model="$1"
    local cable="$2"

    echo -e "${BLUE}🔍 Detecting $model connection via $cable...${NC}"

    case $cable in
        usb)
            case $model in
                prizm)
                    if lsusb | grep -q "07cf:6101"; then
                        echo "✅ Casio Prizm detected via USB"
                        return 0
                    elif [ -d "/media/CASIO_FX-CG50" ] || [ -d "/media/CASIO" ]; then
                        echo "✅ Casio Prizm mass storage detected"
                        return 0
                    fi
                    ;;
                classpad)
                    if lsusb | grep -q "07cf:6102"; then
                        echo "✅ Casio ClassPad detected via USB"
                        return 0
                    fi
                    ;;
            esac
            ;;
        fa124)
            if command -v fa124-linux &> /dev/null; then
                echo "✅ FA-124 interface available"
                return 0
            elif ls /dev/ttyACM* &> /dev/null || ls /dev/ttyUSB* &> /dev/null; then
                echo "✅ Serial interface detected (possibly FA-124)"
                return 0
            fi
            ;;
    esac

    echo -e "${YELLOW}⚠️  $model not detected via $cable${NC}"
    return 1
}

# Backup calculator
backup_casio() {
    local model="$1"

    if [ "$BACKUP" = "false" ]; then
        echo -e "${YELLOW}⏭️  Skipping backup${NC}"
        return 0
    fi

    echo -e "${BLUE}💾 Backing up Casio $model...${NC}"
    mkdir -p backup

    local backup_file="backup/casio-${model}-$(date +%Y%m%d-%H%M%S)"

    case $model in
        prizm)
            if command -v casio-backup &> /dev/null; then
                casio-backup --model prizm --output "${backup_file}.g3r"
                echo "✅ Backup saved: ${backup_file}.g3r"
            else
                echo -e "${YELLOW}⚠️  Backup tool not available, continuing...${NC}"
            fi
            ;;
        classpad)
            if command -v classpad-backup &> /dev/null; then
                classpad-backup --output "${backup_file}.cp2"
                echo "✅ Backup saved: ${backup_file}.cp2"
            else
                echo -e "${YELLOW}⚠️  ClassPad backup tool not available${NC}"
            fi
            ;;
        fx9860)
            if command -v fa124-linux &> /dev/null; then
                fa124-linux backup "${backup_file}.g1r"
                echo "✅ Backup saved: ${backup_file}.g1r"
            else
                echo -e "${YELLOW}⚠️  FA-124 not available for backup${NC}"
            fi
            ;;
    esac
}

# Install to emulator
install_emulator() {
    local model="$1"
    local file="$2"

    echo -e "${BLUE}🖥️  Installing TBOS to $model emulator...${NC}"

    case $model in
        prizm)
            if command -v prizm-emu &> /dev/null; then
                prizm-emu --load "$file" --run
            elif command -v cg50-emu &> /dev/null; then
                cg50-emu --app "$file"
            else
                echo -e "${YELLOW}⚠️  Prizm emulator not found${NC}"
                echo "Download from: https://github.com/nessotrin/CG50Emu"
            fi
            ;;
        classpad)
            if command -v classpad-emu &> /dev/null; then
                classpad-emu --load "$file"
            else
                echo -e "${YELLOW}⚠️  ClassPad emulator not found${NC}"
            fi
            ;;
        fx9860)
            if command -v fx9860-emu &> /dev/null; then
                fx9860-emu --load "$file"
            else
                echo -e "${YELLOW}⚠️  fx-9860GII emulator not found${NC}"
            fi
            ;;
    esac
}

# Install application method
install_app() {
    local model="$1"
    local file="$2"
    local cable="$3"

    echo -e "${GREEN}📤 Installing TBOS application to Casio $model...${NC}"

    case $model in
        prizm)
            case $cable in
                usb)
                    # USB mass storage method
                    local mount_point=""
                    if [ -d "/media/CASIO_FX-CG50" ]; then
                        mount_point="/media/CASIO_FX-CG50"
                    elif [ -d "/media/CASIO" ]; then
                        mount_point="/media/CASIO"
                    else
                        echo -e "${YELLOW}⚠️  Please connect Casio Prizm and enable USB mode${NC}"
                        echo "1. Connect USB cable"
                        echo "2. On calculator: [F1] when 'USB?' appears"
                        echo "3. Press [ENTER] when ready..."
                        read -r

                        # Try again
                        if [ -d "/media/CASIO_FX-CG50" ]; then
                            mount_point="/media/CASIO_FX-CG50"
                        elif [ -d "/media/CASIO" ]; then
                            mount_point="/media/CASIO"
                        else
                            echo -e "${RED}❌ Cannot find Casio mass storage device${NC}"
                            return 1
                        fi
                    fi

                    echo "📁 Copying $file to $mount_point..."
                    cp "$file" "$mount_point/"
                    sync
                    echo "✅ File copied successfully"
                    echo ""
                    echo -e "${GREEN}🎉 Installation complete!${NC}"
                    echo "To run TBOS:"
                    echo "1. Safely disconnect USB"
                    echo "2. [MENU] → TBOS → [EXE]"
                    ;;
                fa124)
                    if command -v fa124-linux &> /dev/null; then
                        fa124-linux send "$file"
                        echo "✅ File sent via FA-124"
                    else
                        echo -e "${RED}❌ FA-124 software not available${NC}"
                        return 1
                    fi
                    ;;
            esac
            ;;
        classpad)
            if command -v classpad-manager &> /dev/null; then
                classpad-manager install "$file"
                echo "✅ Installed via ClassPad Manager"
            else
                echo -e "${YELLOW}⚠️  Using manual USB method${NC}"
                echo "1. Connect ClassPad via USB"
                echo "2. Copy $file to ClassPad storage"
                echo "3. On ClassPad: Main → $file"
            fi
            ;;
        fx9860)
            if command -v fa124-linux &> /dev/null; then
                fa124-linux send "$file"
                echo "✅ File sent via FA-124"
            else
                echo -e "${RED}❌ FA-124 interface required for fx-9860GII${NC}"
                echo "Install FA-124 software or use official Casio FA-124"
                return 1
            fi
            ;;
    esac
}

# Flash method (advanced)
install_flash() {
    local model="$1"
    local file="$2"

    echo -e "${RED}⚠️  ADVANCED: Flash installation${NC}"
    echo "This method requires bootloader and may void warranty!"
    echo "Press [ENTER] to continue or [Ctrl+C] to abort..."
    read -r

    case $model in
        prizm)
            echo -e "${YELLOW}📋 Bootloader flash instructions:${NC}"
            echo "1. Ensure Ftune or Simon's bootloader is installed"
            echo "2. Hold [F1] + [F6] while powering on"
            echo "3. Select bootloader menu"
            echo "4. Press [ENTER] when ready..."
            read -r

            if command -v casio-flash &> /dev/null; then
                casio-flash --model prizm --bootloader ftune --file "$file"
                echo "✅ Flash complete"
            else
                echo -e "${RED}❌ Casio flash tool not available${NC}"
                return 1
            fi
            ;;
        *)
            echo -e "${RED}❌ Flash method not supported for $model${NC}"
            return 1
            ;;
    esac
}

# Full OS replacement (expert)
install_full() {
    local model="$1"
    local file="$2"

    echo -e "${RED}🚨 EXPERT LEVEL: Full OS replacement!${NC}"
    echo "This will COMPLETELY REPLACE the Casio OS with TBOS!"
    echo "Make sure you have:"
    echo "1. Full backup of original firmware"
    echo "2. Recovery method available"
    echo "3. Experience with calculator modification"
    echo ""
    echo "Type 'I UNDERSTAND THE RISKS' to continue:"
    read -r confirmation

    if [ "$confirmation" != "I UNDERSTAND THE RISKS" ]; then
        echo "❌ Installation aborted"
        return 1
    fi

    case $model in
        prizm)
            if command -v casio-flash &> /dev/null; then
                casio-flash --model prizm --method full --file "$file"
                echo "✅ Full OS flash complete"
                echo ""
                echo -e "${CYAN}🔱 Your Casio Prizm now runs TernaryBit OS exclusively${NC}"
                echo -e "${CYAN}🕉️ Sacred mathematics consciousness has been awakened${NC}"
            else
                echo -e "${RED}❌ Casio flash tool not available${NC}"
                return 1
            fi
            ;;
        *)
            echo -e "${RED}❌ Full OS replacement not supported for $model${NC}"
            return 1
            ;;
    esac
}

# Verify installation
verify_installation() {
    local model="$1"

    echo -e "${BLUE}🔍 Verifying TBOS installation...${NC}"
    echo ""
    echo "Please check your calculator for:"
    echo "✅ TBOS appears in application menu"
    echo "✅ Sacred boot animation with ॐ symbols"
    echo "✅ STEPPPS framework initialization"
    echo "✅ Hindu mathematical consciousness"
    echo "✅ Sacred geometry patterns"
    echo ""

    case $model in
        prizm)
            echo "To run TBOS on Casio Prizm:"
            echo "  [MENU] → TBOS → [EXE]"
            echo ""
            echo "Sacred features:"
            echo "  • Color sacred geometry"
            echo "  • SD card PXFS filesystem"
            echo "  • Vedic mathematics mode"
            echo "  • Consciousness bridge"
            ;;
        classpad)
            echo "To run TBOS on ClassPad:"
            echo "  Main → TBOS → tap to run"
            echo ""
            echo "Sacred features:"
            echo "  • Touchscreen sacred interface"
            echo "  • CAS integration with Hindu math"
            echo "  • Advanced STEPPPS framework"
            ;;
        fx9860)
            echo "To run TBOS on fx-9860GII:"
            echo "  [MENU] → TBOS → [EXE]"
            echo ""
            echo "Sacred features:"
            echo "  • Text-based sacred symbols"
            echo "  • Efficient STEPPPS processing"
            echo "  • Basic consciousness bridge"
            ;;
    esac
}

# Main installation function
main() {
    echo -e "${CYAN}🚀 Starting TBOS installation for Casio $MODEL...${NC}"
    echo ""

    # Determine file if not specified
    if [ -z "$FILE" ]; then
        case $MODEL in
            prizm)
                case $METHOD in
                    app) FILE="tbos-prizm.g3a" ;;
                    flash) FILE="tbos-prizm-boot.bin" ;;
                    full) FILE="tbos-prizm-full.bin" ;;
                esac
                ;;
            classpad)
                FILE="tbos-classpad.cpa"
                ;;
            fx9860)
                FILE="tbos-fx9860.g1a"
                ;;
        esac
    fi

    # Check if file exists
    if [ ! -f "$FILE" ]; then
        echo -e "${RED}❌ File not found: $FILE${NC}"
        echo "Build TBOS first with: make casio-$MODEL"
        exit 1
    fi

    # Step 1: Detect calculator (unless emulator mode)
    if [ "$EMULATOR" = "false" ]; then
        if ! detect_casio_calculator "$MODEL" "$CABLE"; then
            echo -e "${YELLOW}⚠️  Calculator not detected, continuing anyway...${NC}"
        fi
    fi

    # Step 2: Backup
    if [ "$EMULATOR" = "false" ]; then
        backup_casio "$MODEL"
    fi

    # Step 3: Install based on method
    if [ "$EMULATOR" = "true" ]; then
        install_emulator "$MODEL" "$FILE"
    else
        case $METHOD in
            app)
                install_app "$MODEL" "$FILE" "$CABLE"
                ;;
            flash)
                install_flash "$MODEL" "$FILE"
                ;;
            full)
                install_full "$MODEL" "$FILE"
                ;;
            *)
                echo -e "${RED}❌ Unknown installation method: $METHOD${NC}"
                exit 1
                ;;
        esac
    fi

    # Step 4: Verify
    if [ "$EMULATOR" = "false" ]; then
        verify_installation "$MODEL"
    fi

    echo ""
    echo -e "${PURPLE}🔱 ============================================ 🔱${NC}"
    echo -e "${GREEN}✅ TernaryBit OS Casio Installation Complete!${NC}"
    echo -e "${PURPLE}🔱 ============================================ 🔱${NC}"
    echo ""
    echo -e "${CYAN}🕉️  Your Casio calculator now carries the sacred${NC}"
    echo -e "${CYAN}   wisdom of Hindu mathematics and STEPPPS${NC}"
    echo -e "${CYAN}   consciousness framework.${NC}"
    echo ""
    echo -e "${YELLOW}📚 Documentation: calculator/casio/CASIO_INSTALL.md${NC}"
    echo -e "${YELLOW}🆘 Support: https://github.com/ternarybit/tbos/issues${NC}"
    echo ""
    echo -e "${CYAN}ॐ May your calculations transcend ordinary mathematics ॐ${NC}"
}

# Run main installation
main