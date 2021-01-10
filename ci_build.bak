#!/bin/bash
set -e

# Install embedded_device_map
chmod 755 secrets/embedded_device_map
cp secrets/embedded_device_map /usr/bin/

mkdir -p /usr/local/standalone/firmware
cp secrets/device_map.db /usr/local/standalone/firmware/

# setup env


# chmod requires parts
chmod +x tools/*

make APPLICATIONS="iBoot" TARGETS="n41 n42" BUILDS="DEVELOPMENT DEBUG" PRODUCTS="iBSS iBEC" SDK_PLATFORM="iOS 9.3"
