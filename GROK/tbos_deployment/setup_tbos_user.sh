#!/bin/bash
# Setup TBOS user and environment

# Create TBOS user
useradd -m -s /bin/bash tbos
echo "tbos:tbos123" | chpasswd

# Copy TBOS files
cp TernaryBitOS.jar /home/tbos/
cp run_tbos.sh /home/tbos/
cp demo_tbos.sh /home/tbos/
chown -R tbos:tbos /home/tbos/

# Install TBOS service
cp tbos-autostart.service /etc/systemd/system/
systemctl daemon-reload
systemctl enable tbos-autostart

echo "✅ TBOS user and service installed"
echo "TBOS will autostart on boot"
echo "Login as 'tbos' user to interact with TBOS"
