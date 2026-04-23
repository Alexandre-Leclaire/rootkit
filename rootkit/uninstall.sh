#!/bin/sh

set -e

MODULE_NAME="rootkit"

echo "[+] Removing module..."
sudo modprobe -r ${MODULE_NAME}

echo "[+] Cleaning up persistence files..."
sudo rm -f /etc/modules-load.d/${MODULE_NAME}.conf
sudo rm -f /etc/modprobe.d/${MODULE_NAME}.conf
sudo rm -f /lib/modules/$(uname -r)/extra/${MODULE_NAME}.ko

echo "[+] Running depmod..."
sudo depmod

echo "[+] Done. Module removed and persistence cleaned."
