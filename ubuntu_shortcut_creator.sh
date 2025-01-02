#!/bin/bash
# Shortcut installer for xmr-stak-gui-ccx for Ubuntu users
# Copyright (c) 2024, Acktarius

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

# Create wrapper script
cat > "${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx-wrapper.sh" << 'EOF'
#!/bin/bash
export XAUTHORITY=$HOME/.Xauthority
export DISPLAY=:0
cd "$(dirname "$0")"
./xmr-stak-gui-ccx
EOF

chmod +x "${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx-wrapper.sh"

# Create user applications directory if it doesn't exist
mkdir -p "${REAL_HOME}/.local/share/applications"

# Create application launcher
cat > "${REAL_HOME}/.local/share/applications/xmr-stak-gui-ccx.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Xmr-Stak-gui-CCX
Comment=XMR-Stak GUI for Conceal Mining
Exec=pkexec ${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx-wrapper.sh
Icon=${SCRIPT_DIR}/doc/_img/xmr-stak-gui-ccx.png
Terminal=false
Categories=System;
Keywords=Mining;Conceal;CCX;Crypto;
EOF

# Create polkit policy file
cat > /usr/share/polkit-1/actions/org.xmrstak.guiccx.policy << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE policyconfig PUBLIC
 "-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/PolicyKit/1/policyconfig.dtd">
<policyconfig>
  <action id="org.xmrstak.guiccx">
    <description>Run XMR-Stak GUI CCX</description>
    <message>Authentication is required to run XMR-Stak GUI CCX</message>
    <defaults>
      <allow_any>auth_admin</allow_any>
      <allow_inactive>auth_admin</allow_inactive>
      <allow_active>auth_admin</allow_active>
    </defaults>
    <annotate key="org.freedesktop.policykit.exec.path">${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx-wrapper.sh</annotate>
  </action>
</policyconfig>
EOF

# Set correct permissions
chmod +x "${REAL_HOME}/.local/share/applications/xmr-stak-gui-ccx.desktop"
chown ${REAL_USER}:${REAL_USER} "${REAL_HOME}/.local/share/applications/xmr-stak-gui-ccx.desktop"

# Update desktop database for the user
update-desktop-database "${REAL_HOME}/.local/share/applications"

zenity --info \
    --title="Setup Complete" \
    --text="XMR-Stak GUI CCX has been added to your applications menu.\nYou will be prompted for authentication when launching the application."