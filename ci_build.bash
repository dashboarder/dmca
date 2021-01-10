#!/bin/bash
set -e

# Install embedded_device_map
chmod 755 secrets/embedded_device_map

# setup env

# chmod requires parts
chmod +x tools/*

make APPLICATIONS="iBoot" TARGETS="n41 n42" BUILDS="DEVELOPMENT DEBUG" PRODUCTS="iBSS iBEC" SDKROOT="iphoneos"
