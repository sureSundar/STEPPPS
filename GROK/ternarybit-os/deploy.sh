#!/bin/bash

# TernaryBit OS Universal Deployment Script
# Builds and deploys TBOS across all supported platforms

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
NC='\033[0m' # No Color

# TBOS deployment banner
echo -e "${PURPLE}"
echo "🔱 ========================================== 🔱"
echo "    TernaryBit OS Universal Deployment"
echo "       Multi-Platform Build System"
echo "🔱 ========================================== 🔱"
echo -e "${NC}"

# Parse command line arguments
PLATFORM="all"
BUILD_TYPE="release"
SKIP_TESTS="false"
PUSH_CONTAINERS="false"

while [[ $# -gt 0 ]]; do
    case $1 in
        --platform)
            PLATFORM="$2"
            shift 2
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --skip-tests)
            SKIP_TESTS="true"
            shift
            ;;
        --push-containers)
            PUSH_CONTAINERS="true"
            shift
            ;;
        --help)
            echo "TernaryBit OS Deployment Script"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --platform PLATFORM    Target platform (all|docker|wasm|android|ios|linux|windows|macos)"
            echo "  --build-type TYPE       Build type (release|debug)"
            echo "  --skip-tests           Skip test execution"
            echo "  --push-containers      Push Docker containers to registry"
            echo "  --help                 Show this help message"
            echo ""
            echo "Supported Platforms:"
            echo "  🐳 docker    - Universal Docker container"
            echo "  🌐 wasm      - WebAssembly for browsers"
            echo "  🤖 android   - Android APK"
            echo "  🍎 ios       - iOS App Store package"
            echo "  🐧 linux     - Native Linux binaries"
            echo "  🪟 windows   - Windows executable"
            echo "  🍎 macos     - macOS application"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo -e "${CYAN}📋 Deployment Configuration:${NC}"
echo "   Platform: $PLATFORM"
echo "   Build Type: $BUILD_TYPE"
echo "   Skip Tests: $SKIP_TESTS"
echo "   Push Containers: $PUSH_CONTAINERS"
echo ""

# Create build directories
echo -e "${BLUE}📁 Creating build directories...${NC}"
mkdir -p build/{docker,wasm,android,ios,linux,windows,macos}
mkdir -p dist/{docker,wasm,android,ios,linux,windows,macos}

# Function to build Docker container
build_docker() {
    echo -e "${GREEN}🐳 Building Docker container...${NC}"

    docker build -t ternarybit/tbos:latest .
    docker build -t ternarybit/tbos:$BUILD_TYPE .

    if [ "$PUSH_CONTAINERS" = "true" ]; then
        echo -e "${YELLOW}📤 Pushing to Docker registry...${NC}"
        docker push ternarybit/tbos:latest
        docker push ternarybit/tbos:$BUILD_TYPE
    fi

    # Save container for offline distribution
    docker save ternarybit/tbos:latest | gzip > dist/docker/tbos-latest.tar.gz

    echo -e "${GREEN}✅ Docker build complete${NC}"
}

# Function to build WebAssembly
build_wasm() {
    echo -e "${GREEN}🌐 Building WebAssembly...${NC}"

    if ! command -v emcc &> /dev/null; then
        echo -e "${RED}❌ Emscripten not found. Please install Emscripten SDK.${NC}"
        return 1
    fi

    make -f Makefile.wasm clean
    make -f Makefile.wasm

    # Package for distribution
    cp -r build/wasm/* dist/wasm/
    cd dist/wasm
    tar -czf ../tbos-wasm.tar.gz *
    cd - > /dev/null

    echo -e "${GREEN}✅ WebAssembly build complete${NC}"
}

# Function to build Android APK
build_android() {
    echo -e "${GREEN}🤖 Building Android APK...${NC}"

    if [ ! -d "mobile/android" ]; then
        echo -e "${RED}❌ Android project not found${NC}"
        return 1
    fi

    cd mobile/android

    if [ "$BUILD_TYPE" = "release" ]; then
        ./gradlew assembleRelease
        cp app/build/outputs/apk/release/app-release.apk ../../dist/android/tbos-release.apk
    else
        ./gradlew assembleDebug
        cp app/build/outputs/apk/debug/app-debug.apk ../../dist/android/tbos-debug.apk
    fi

    cd - > /dev/null
    echo -e "${GREEN}✅ Android build complete${NC}"
}

# Function to build iOS app
build_ios() {
    echo -e "${GREEN}🍎 Building iOS app...${NC}"

    if [ ! -d "mobile/ios" ]; then
        echo -e "${RED}❌ iOS project not found${NC}"
        return 1
    fi

    if [ "$(uname)" != "Darwin" ]; then
        echo -e "${YELLOW}⚠️  iOS builds require macOS. Creating placeholder...${NC}"
        echo "iOS build requires macOS with Xcode" > dist/ios/README.txt
        return 0
    fi

    cd mobile/ios
    xcodebuild -workspace TernaryBitOS.xcworkspace -scheme TernaryBitOS -configuration $BUILD_TYPE archive
    cd - > /dev/null

    echo -e "${GREEN}✅ iOS build complete${NC}"
}

# Function to build Linux binaries
build_linux() {
    echo -e "${GREEN}🐧 Building Linux binaries...${NC}"

    make clean
    make ARCH=x86_64 BUILD_TYPE=$BUILD_TYPE
    make ARCH=arm64 BUILD_TYPE=$BUILD_TYPE

    # Package binaries
    cp build/x86_64/* dist/linux/ 2>/dev/null || true
    cp build/arm64/* dist/linux/ 2>/dev/null || true

    cd dist/linux
    tar -czf ../tbos-linux.tar.gz *
    cd - > /dev/null

    echo -e "${GREEN}✅ Linux build complete${NC}"
}

# Function to build Windows executable
build_windows() {
    echo -e "${GREEN}🪟 Building Windows executable...${NC}"

    if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
        echo -e "${YELLOW}⚠️  MinGW-w64 not found. Installing...${NC}"
        sudo apt-get update && sudo apt-get install -y mingw-w64
    fi

    make clean
    make ARCH=x86_64 BUILD_TYPE=$BUILD_TYPE CROSS_COMPILE=x86_64-w64-mingw32-

    cp build/x86_64/*.exe dist/windows/ 2>/dev/null || true

    echo -e "${GREEN}✅ Windows build complete${NC}"
}

# Function to build macOS application
build_macos() {
    echo -e "${GREEN}🍎 Building macOS application...${NC}"

    if [ "$(uname)" != "Darwin" ]; then
        echo -e "${YELLOW}⚠️  macOS builds require macOS. Creating placeholder...${NC}"
        echo "macOS build requires macOS with Xcode" > dist/macos/README.txt
        return 0
    fi

    make clean
    make ARCH=x86_64 BUILD_TYPE=$BUILD_TYPE
    make ARCH=arm64 BUILD_TYPE=$BUILD_TYPE  # Apple Silicon

    # Create universal binary
    lipo -create -output dist/macos/tbos build/x86_64/tbos build/arm64/tbos

    echo -e "${GREEN}✅ macOS build complete${NC}"
}

# Function to run tests
run_tests() {
    if [ "$SKIP_TESTS" = "true" ]; then
        echo -e "${YELLOW}⏭️  Skipping tests${NC}"
        return 0
    fi

    echo -e "${BLUE}🧪 Running tests...${NC}"

    # Run TBOS test suite
    make test || {
        echo -e "${RED}❌ Tests failed${NC}"
        return 1
    }

    echo -e "${GREEN}✅ All tests passed${NC}"
}

# Main deployment logic
case $PLATFORM in
    "all")
        echo -e "${CYAN}🚀 Building all platforms...${NC}"
        run_tests
        build_docker
        build_wasm
        build_linux
        build_android
        build_ios
        build_windows
        build_macos
        ;;
    "docker")
        run_tests
        build_docker
        ;;
    "wasm")
        run_tests
        build_wasm
        ;;
    "android")
        run_tests
        build_android
        ;;
    "ios")
        run_tests
        build_ios
        ;;
    "linux")
        run_tests
        build_linux
        ;;
    "windows")
        run_tests
        build_windows
        ;;
    "macos")
        run_tests
        build_macos
        ;;
    *)
        echo -e "${RED}❌ Unknown platform: $PLATFORM${NC}"
        echo "Supported platforms: all, docker, wasm, android, ios, linux, windows, macos"
        exit 1
        ;;
esac

# Generate deployment summary
echo ""
echo -e "${PURPLE}🔱 ========================================== 🔱${NC}"
echo -e "${GREEN}✅ TernaryBit OS Deployment Complete!${NC}"
echo -e "${PURPLE}🔱 ========================================== 🔱${NC}"
echo ""
echo -e "${CYAN}📦 Build Artifacts:${NC}"
ls -la dist/*/
echo ""
echo -e "${CYAN}🚀 Deployment Instructions:${NC}"
echo ""
echo -e "${YELLOW}Docker:${NC}"
echo "  docker run --rm -it ternarybit/tbos:latest"
echo ""
echo -e "${YELLOW}WebAssembly:${NC}"
echo "  cd dist/wasm && python3 -m http.server 8080"
echo "  Open http://localhost:8080"
echo ""
echo -e "${YELLOW}Android:${NC}"
echo "  adb install dist/android/tbos-$BUILD_TYPE.apk"
echo ""
echo -e "${YELLOW}Linux:${NC}"
echo "  tar -xzf dist/tbos-linux.tar.gz && ./tbos"
echo ""
echo -e "${GREEN}🎉 TBOS is now ready for multi-platform deployment!${NC}"