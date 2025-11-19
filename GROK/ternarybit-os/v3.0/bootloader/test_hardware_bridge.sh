#!/bin/bash

echo "🧪 TBOS Hardware Bridge Test"
echo "============================"
echo ""

echo "🔍 Testing hardware information access..."

echo "1. CPU Information:"
if [ -f "/proc/cpuinfo" ]; then
    echo "   ✅ /proc/cpuinfo accessible"
    CPU_MODEL=$(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)
    echo "   CPU: $CPU_MODEL"
else
    echo "   ❌ /proc/cpuinfo not accessible"
fi

echo ""
echo "2. Memory Information:"
if [ -f "/proc/meminfo" ]; then
    echo "   ✅ /proc/meminfo accessible"
    TOTAL_MEM=$(grep "MemTotal" /proc/meminfo | awk '{print $2}')
    echo "   Total Memory: $((TOTAL_MEM / 1024)) MB"
else
    echo "   ❌ /proc/meminfo not accessible"
fi

echo ""
echo "3. CPU Statistics:"
if [ -f "/proc/stat" ]; then
    echo "   ✅ /proc/stat accessible"
    echo "   CPU stats available for real-time monitoring"
else
    echo "   ❌ /proc/stat not accessible"
fi

echo ""
echo "4. Network Interface:"
if [ -f "/proc/net/dev" ]; then
    echo "   ✅ /proc/net/dev accessible"
    echo "   Network statistics available"
else
    echo "   ❌ /proc/net/dev not accessible"
fi

echo ""
echo "5. Temperature Sensors:"
if [ -f "/sys/class/thermal/thermal_zone0/temp" ]; then
    echo "   ✅ Thermal sensors accessible"
    TEMP=$(cat /sys/class/thermal/thermal_zone0/temp)
    echo "   CPU Temperature: $((TEMP / 1000))°C"
else
    echo "   ⚠️  Thermal sensors not available (normal in some environments)"
fi

echo ""
echo "6. System Commands:"
echo "   Testing command execution..."
echo "   Number of CPU cores: $(nproc)"
echo "   System uptime: $(uptime -p)"
echo "   Disk usage summary:"
df -h / | tail -1 | awk '{print "   Root filesystem: " $3 " used of " $2 " (" $5 " full)"}'

echo ""
echo "✅ Hardware bridge test completed"
echo "🚀 System is ready for TBOS hardware integration"
