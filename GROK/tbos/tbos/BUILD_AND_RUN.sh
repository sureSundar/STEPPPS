#!/bin/bash
# TernaryBit OS - Docker Build and Run Script

echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                                                                  ║"
echo "║              TernaryBit OS - Docker Deployment                   ║"
echo "║                                                                  ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

BASE_DIR="/home/vboxuser/ssr/tbos"
cd "$BASE_DIR"

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker not installed"
    echo ""
    echo "Install Docker:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y docker.io docker-compose"
    echo "  sudo usermod -aG docker $USER"
    echo "  newgrp docker"
    exit 1
fi

echo "Step 1: Building TernaryBit OS Docker image..."
echo "────────────────────────────────────────────────────────────────"

# Build the Docker image
docker build -t ternarybit-os:1.0 . 2>&1 | grep -E "(Step|Successfully|ERROR)" || \
    docker build -t ternarybit-os:1.0 .

if [ $? -eq 0 ]; then
    echo "✓ Docker image built successfully"
else
    echo "❌ Docker build failed"
    exit 1
fi

echo ""
echo "Step 2: Creating workspace directory..."
echo "────────────────────────────────────────────────────────────────"

mkdir -p workspace
echo "✓ Workspace created at: $BASE_DIR/workspace"

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                                                                  ║"
echo "║                   BUILD COMPLETE!                                ║"
echo "║                                                                  ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

echo "To run TernaryBit OS:"
echo ""
echo "  Option 1: Interactive shell"
echo "    docker run -it --rm ternarybit-os:1.0"
echo ""
echo "  Option 2: Run a specific program"
echo "    docker run -it --rm ternarybit-os:1.0 tbrun /opt/tbos/examples/hello.tbs"
echo ""
echo "  Option 3: Use docker-compose"
echo "    docker-compose up -d"
echo "    docker-compose exec tbos /bin/sh"
echo ""
echo "  Option 4: Mount your own programs"
echo "    docker run -it --rm -v \$(pwd)/workspace:/workspace ternarybit-os:1.0"
echo ""

echo "Quick test:"
echo "────────────────────────────────────────────────────────────────"
echo "Running hello.tbs in container..."
echo ""

docker run --rm ternarybit-os:1.0 tbrun /opt/tbos/examples/hello.tbs

echo ""
echo "✓ TernaryBit OS is ready to use!"
echo ""
