#!/bin/bash
# shorcut installer for xmr-stak-gui-ccx for Ubuntu users
# this file is subject to Licence
# Copyright (c) 2024, Acktarius
#
# make sure ./ubuntu_shortcut_installer.sh is an executable file
# otherwise, run: sudo chmod 755 shortcut_installer.sh
# run with command: ./shortcut_installer.sh
#
#
#variables
#user=$(id -nu 1000)
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
#path=$(pwd)
#Functions
shortcutCreator() {
cat << EOF > /usr/share/applications/xmr-stak-gui-ccx.desktop
[Desktop Entry]
Version=3.0
Name=Xmr-Stak-gui-CCX
Path=${SCRIPT_DIR}/build/bin
Exec=${SCRIPT_DIR}/build/bin/xmr-stak-gui-ccx
Terminal=false
Type=Application
Icon=${SCRIPT_DIR}/doc/_img/xmr-stak-gui-ccx.png
Hidden=false
NoDisplay=false
Terminal=false
Categories=Office
X-GNOME-Autostart-enabled=true
Comment=Xmr-Stak-gui-CCX
EOF
echo "shortcut created, you may have to log out and log back in"
}
already() {
read -p  "shortcut already in place, do you want to replace it (y/N)" ans
	case $ans in
		y | Y | yes)
		rm -f /usr/share/applications/xmr-stak-gui-ccx.desktop
		shortcutCreator
		;;
		*)
		echo "nothing done"
		;;
	esac
}
#check and install
##not already install
if [[ ! -f /usr/share/applications/xmr-stak-gui-ccx.desktop ]]; then 
shortcutCreator
else
already
fi