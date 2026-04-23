#!/bin/bash

IP_ADDR=$1
USER=$2

if systemctl is-active --quiet ssh; then
    exit 1
else
    systemctl start ssh
    exit 0
fi
