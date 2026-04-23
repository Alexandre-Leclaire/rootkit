#!/bin/bash

IP_ADDR=$1
TARGET=$2
DLFILE=$3
DLPATH=$4

echo "" | scp -r ${TARGET}@${IP_ADDR}:${DLFILE} ${DLPATH}
