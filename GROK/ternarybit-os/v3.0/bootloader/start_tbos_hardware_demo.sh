#!/bin/bash

echo "🚀 TBOS v3.0 Hardware Bridge Demo"
echo "================================="
echo ""

# Check if bridge executable exists
if [ ! -f "./tbos_web_bridge" ]; then
    echo "❌ TBOS Web Bridge not found. Please run compile_hardware_bridge.sh first"
    exit 1
fi

echo "🌉 Starting TBOS Hardware Bridge..."

# Start the hardware bridge in background
./tbos_web_bridge &
BRIDGE_PID=$!

echo "✅ Hardware bridge started (PID: $BRIDGE_PID)"
echo ""

# Start the web server for the HTML interface
echo "🌐 Starting web server for HTML interface..."

# Use Python's built-in server to serve the HTML interface
if command -v python3 &> /dev/null; then
    python3 -m http.server 9000 &
    WEB_PID=$!
    echo "✅ Web server started (PID: $WEB_PID)"
elif command -v python &> /dev/null; then
    python -m SimpleHTTPServer 9000 &
    WEB_PID=$!
    echo "✅ Web server started (PID: $WEB_PID)"
else
    echo "❌ Python not found. Please install Python to serve the web interface"
    kill $BRIDGE_PID
    exit 1
fi

echo ""
echo "🎉 TBOS v3.0 Hardware Bridge Demo is now running!"
echo ""
echo "🌟 REVOLUTIONARY ACHIEVEMENT:"
echo "   HTML5 web interface is now connected to real hardware!"
echo ""
echo "🌐 Access the hardware interface:"
echo "   Primary Interface: http://localhost:9000/tbos_hardware_interface.html"
echo "   Alternative:       http://localhost:9000/index.html"
echo ""
echo "🔧 Hardware Bridge:"
echo "   WebSocket endpoint: ws://localhost:9001"
echo "   Status: ACTIVE"
echo ""
echo "✨ Features available:"
echo "   🔥 Real CPU monitoring via /proc/stat"
echo "   💾 Real memory usage via /proc/meminfo"
echo "   🌡️ CPU temperature via thermal sensors"
echo "   🌐 Network stats via /proc/net/dev"
echo "   💻 Execute actual system commands"
echo "   🔄 Hardware-level persona switching"
echo "   📊 Live system performance data"
echo ""
echo "🎮 Try these actions in the web interface:"
echo "   • Click on system info icons"
echo "   • Open the hardware terminal (Ctrl+H)"
echo "   • Execute commands like 'htop', 'free', 'lscpu'"
echo "   • Switch personas and see hardware adapt"
echo "   • Monitor real-time CPU and memory usage"
echo ""
echo "🛑 To stop the demo:"
echo "   Press Ctrl+C or run: kill $BRIDGE_PID $WEB_PID"
echo ""

# Function to handle cleanup on exit
cleanup() {
    echo ""
    echo "🛑 Stopping TBOS Hardware Bridge Demo..."
    kill $BRIDGE_PID $WEB_PID 2>/dev/null
    echo "✅ Demo stopped"
    exit 0
}

# Set up signal handlers
trap cleanup INT TERM

# Keep the script running
echo "📱 Demo is running... Press Ctrl+C to stop"
echo ""

# Show live stats while running
while true; do
    sleep 5
    if ps -p $BRIDGE_PID > /dev/null 2>&1 && ps -p $WEB_PID > /dev/null 2>&1; then
        echo "⚡ $(date): Hardware bridge and web server running..."
    else
        echo "❌ One or more services stopped unexpectedly"
        cleanup
    fi
done
