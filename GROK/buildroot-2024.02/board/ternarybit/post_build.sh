#!/bin/bash

# TernaryBit OS Post-Build Script
# Customizes the root filesystem after Buildroot completes

set -e

TARGET_DIR="$1"

echo "TernaryBit OS: Customizing root filesystem..."

# Create STEPPPS framework directories
mkdir -p "${TARGET_DIR}/opt/ternarybit/steppps"
mkdir -p "${TARGET_DIR}/var/log/ternarybit"
mkdir -p "${TARGET_DIR}/etc/ternarybit"

# Install TernaryBit configuration files
cat > "${TARGET_DIR}/etc/ternarybit/steppps.conf" << 'EOF'
# STEPPPS Framework Configuration
# Space, Time, Event, Psychology, Pixel, Prompt, Script

[dimensions]
space=enabled
time=enabled
event=enabled
psychology=enabled
pixel=enabled
prompt=enabled
script=enabled

[logging]
level=info
destination=/var/log/ternarybit/steppps.log

[features]
hardware_detection=true
power_management=true
divine_protection=true
EOF

# Create custom motd
cat > "${TARGET_DIR}/etc/motd" << 'EOF'

████████╗███████╗██████╗ ███╗   ██╗ █████╗ ██████╗ ██╗   ██╗██████╗ ██╗████████╗
╚══██╔══╝██╔════╝██╔══██╗████╗  ██║██╔══██╗██╔══██╗╚██╗ ██╔╝██╔══██╗██║╚══██╔══╝
   ██║   █████╗  ██████╔╝██╔██╗ ██║███████║██████╔╝ ╚████╔╝ ██████╔╝██║   ██║
   ██║   ██╔══╝  ██╔══██╗██║╚██╗██║██╔══██║██╔══██╗  ╚██╔╝  ██╔══██╗██║   ██║
   ██║   ███████╗██║  ██║██║ ╚████║██║  ██║██║  ██║   ██║   ██████╔╝██║   ██║
   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═════╝ ╚═╝   ╚═╝

           STEPPPS Framework - Universal Computing Architecture
           Space • Time • Event • Psychology • Pixel • Prompt • Script

EOF

# Install custom init scripts
mkdir -p "${TARGET_DIR}/etc/init.d"

cat > "${TARGET_DIR}/etc/init.d/S99ternarybit" << 'EOF'
#!/bin/sh
#
# TernaryBit OS Initialization Script
#

case "$1" in
  start)
    echo "Starting TernaryBit STEPPPS Framework..."

    # Initialize STEPPPS dimensions
    echo "Initializing STEPPPS dimensions..."

    # Start system monitoring
    if [ -x /opt/ternarybit/bin/steppps-monitor ]; then
        /opt/ternarybit/bin/steppps-monitor &
    fi

    echo "TernaryBit OS initialization complete."
    ;;
  stop)
    echo "Stopping TernaryBit services..."
    killall steppps-monitor 2>/dev/null || true
    ;;
  restart)
    $0 stop
    $0 start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart}"
    exit 1
esac

exit 0
EOF

chmod +x "${TARGET_DIR}/etc/init.d/S99ternarybit"

# Create custom profile additions
cat > "${TARGET_DIR}/etc/profile.d/ternarybit.sh" << 'EOF'
# TernaryBit OS Environment

export TERNARYBIT_HOME="/opt/ternarybit"
export STEPPPS_CONFIG="/etc/ternarybit/steppps.conf"
export PATH="$PATH:$TERNARYBIT_HOME/bin"

# STEPPPS Framework aliases
alias steppps-status='cat /proc/steppps/status 2>/dev/null || echo "STEPPPS not loaded"'
alias steppps-dimensions='ls -la /sys/ternarybit/dimensions/ 2>/dev/null || echo "Dimensions not available"'

# Divine computing aliases (Hindu mythology integration)
alias ganesha='echo "Om Gam Ganapataye Namaha - Obstacle Remover Active"'
alias saraswati='echo "Om Aim Saraswatyai Namaha - Wisdom Flow Activated"'
alias vishnu='echo "Om Namo Narayanaya - System Preservation Mode"'

echo "STEPPPS Framework Environment Loaded"
EOF

# Set proper permissions
chmod 644 "${TARGET_DIR}/etc/ternarybit/steppps.conf"
chmod 644 "${TARGET_DIR}/etc/profile.d/ternarybit.sh"

echo "TernaryBit OS customization completed successfully."