#!/bin/bash

IP_ADDR=$1
TARGET=$2
UPFILE=$3
UPPATH=$4

echo "" | scp -r ${UPFILE} ${TARGET}@${IP_ADDR}:${DLPATH}
