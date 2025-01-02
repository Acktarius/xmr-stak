#!/bin/bash
# Shortcut installer for xmr-stak-gui-ccx for Ubuntu users
# Copyright (c) 2024-2025, Acktarius

# Check if script is run as root
if [ "$EUID" -ne 0 ]; then
    zenity --error \
        --title="Root Privileges Required" \
        --text="This script needs to be run as root.\n\nPlease run:\nsudo $0"
    exit 1
fi

# Get the actual user who ran the script with sudo
REAL_USER=${SUDO_USER:-$USER}
REAL_HOME=$(getent passwd "$REAL_USER" | cut -d: -f6)
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Create xmrstak group if it doesn't exist
groupadd -f xmrstak

# Add the real user to xmrstak group
usermod -a -G xmrstak "$REAL_USER"

# Set directory ownership and permissions
chown -R root:xmrstak "${SCRIPT_DIR}"
chmod -R 775 "${SCRIPT_DIR}"  # rwxrwxr-x

# Create sudoers rule for xmrstak group
cat > /etc/sudoers.d/xmrstak << EOF
# Allow members of group xmrstak to execute xmr-stak-gui-ccx
%xmrstak ALL=(root) NOPASSWD: ${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx
EOF

# Set correct permissions for sudoers file
chmod 0440 /etc/sudoers.d/xmrstak

# Create user applications directory if it doesn't exist
mkdir -p "${REAL_HOME}/.local/share/applications"
# Create application launcher
cat > "${REAL_HOME}/.local/share/applications/xmr-stak-gui-ccx.desktop" << EOF
[Desktop Entry]
Version=3.0
Type=Application
Name=Xmr-Stak-gui-CCX
Comment=XMR-Stak GUI for Conceal Mining
Path=${SCRIPT_DIR}/build/bin
Exec=sudo ${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx
Icon=${SCRIPT_DIR}/doc/_img/xmr-stak-gui-ccx.png
Terminal=false
Categories=System;
Keywords=Mining;Conceal;CCX;Crypto;
NoDisplay=false
Hidden=false
EOF

# Set correct ownership for .desktop file
# chown ${REAL_USER}:${REAL_USER} "${REAL_HOME}/.local/share/applications/xmr-stak-gui-ccx.desktop"

# Update desktop database
update-desktop-database "${REAL_HOME}/.local/share/applications"

# Final notifications
zenity --info \
    --title="Setup Complete" \
    --text="XMR-Stak GUI CCX has been added to your applications menu.\n\nIMPORTANT: You need to log out and log back in for the new group permissions to take effect."

# Optionally ask for immediate logout
zenity --question \
    --title="Log Out Now?" \
    --text="Would you like to log out now to apply the new permissions?" \
    --ok-label="Log Out Now" \
    --cancel-label="Later"

if [ $? -eq 0 ]; then
    # User clicked "Log Out Now"
    pkill -KILL -u "$REAL_USER"
fi