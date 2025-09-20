# TernaryBit OS Universal Container
# Supports deployment on Linux, Windows, macOS, Android, iOS
FROM ubuntu:22.04

LABEL maintainer="TBOS Development Team"
LABEL description="TernaryBit OS - Universal Multi-Platform Operating System"
LABEL version="1.0.0"

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    qemu-system-x86 \
    qemu-system-arm \
    qemu-system-misc \
    qemu-utils \
    gcc-aarch64-linux-gnu \
    gcc-riscv64-unknown-elf \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    nodejs \
    npm \
    emscripten \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /tbos

# Copy source code
COPY . .

# Build TBOS for multiple architectures
RUN make clean && \
    make ARCH=x86_64 && \
    make ARCH=arm64 && \
    make ARCH=riscv64

# Build WebAssembly version
RUN make wasm

# Create runtime directories
RUN mkdir -p /tbos/runtime/{x86_64,arm64,riscv64,wasm} && \
    mkdir -p /tbos/configs && \
    mkdir -p /tbos/logs

# Copy built binaries to runtime directories
RUN cp build/x86_64/* /tbos/runtime/x86_64/ 2>/dev/null || true && \
    cp build/arm64/* /tbos/runtime/arm64/ 2>/dev/null || true && \
    cp build/riscv64/* /tbos/runtime/riscv64/ 2>/dev/null || true && \
    cp build/wasm/* /tbos/runtime/wasm/ 2>/dev/null || true

# Install TBOS CLI tools
RUN pip3 install -e tools/tbos-cli/

# Create startup script
RUN cat > /tbos/start-tbos.sh << 'EOF'
#!/bin/bash

# TernaryBit OS Universal Launcher
# Detects host architecture and launches appropriate TBOS version

ARCH=$(uname -m)
TBOS_MODE=${TBOS_MODE:-auto}

echo "🔱 TernaryBit OS Universal Container"
echo "📊 Host Architecture: $ARCH"
echo "🎯 Launch Mode: $TBOS_MODE"

case $ARCH in
    x86_64|amd64)
        TBOS_ARCH="x86_64"
        ;;
    aarch64|arm64)
        TBOS_ARCH="arm64"
        ;;
    riscv64)
        TBOS_ARCH="riscv64"
        ;;
    *)
        echo "⚠️  Unknown architecture, defaulting to x86_64"
        TBOS_ARCH="x86_64"
        ;;
esac

# Launch mode selection
case $TBOS_MODE in
    "qemu"|"vm")
        echo "🖥️  Launching TBOS in QEMU virtualization mode..."
        exec qemu-system-${TBOS_ARCH} \
            -kernel /tbos/runtime/${TBOS_ARCH}/tbos.bin \
            -m 512M \
            -serial stdio \
            -display none \
            "$@"
        ;;
    "native")
        echo "⚡ Launching TBOS in native mode..."
        exec /tbos/runtime/${TBOS_ARCH}/tbos-native "$@"
        ;;
    "wasm"|"web")
        echo "🌐 Launching TBOS WebAssembly version..."
        cd /tbos/runtime/wasm
        python3 -m http.server 8080 &
        echo "📱 TBOS Web Interface: http://localhost:8080"
        exec node tbos-wasm-server.js "$@"
        ;;
    "auto"|*)
        echo "🤖 Auto-detecting best launch mode..."
        if [ -f "/tbos/runtime/${TBOS_ARCH}/tbos.bin" ]; then
            echo "🎮 Using QEMU mode"
            exec qemu-system-${TBOS_ARCH} \
                -kernel /tbos/runtime/${TBOS_ARCH}/tbos.bin \
                -m 512M \
                -serial stdio \
                -nographic \
                "$@"
        else
            echo "🌐 Falling back to WebAssembly mode"
            exec $0 wasm
        fi
        ;;
esac
EOF

RUN chmod +x /tbos/start-tbos.sh

# Expose web interface port
EXPOSE 8080

# Set environment variables
ENV TBOS_HOME=/tbos
ENV TBOS_ARCH=auto
ENV TBOS_MODE=auto
ENV PATH="/tbos/tools:$PATH"

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD tbos-cli status || exit 1

# Default command
ENTRYPOINT ["/tbos/start-tbos.sh"]
CMD []