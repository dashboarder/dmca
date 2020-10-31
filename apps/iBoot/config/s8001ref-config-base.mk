# Copyright (C) 2012-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# s8001ref iBoot bootloader build config

PLATFORM     := s8000
SUB_PLATFORM := s8001
ARCH         := arm64
BOOT_CONFIG  := nand
HW_TIMER     := architected

# code modules
MODULES_BASIC += \
    platform/$(PLATFORM) \
    platform/$(PLATFORM)/chipid \
    platform/$(PLATFORM)/dcs \
    platform/$(PLATFORM)/error_handler \
    platform/$(PLATFORM)/miu \
    platform/$(PLATFORM)/pmgr \
    drivers/apple/a7iop \
    drivers/apple/aes_v2 \
    drivers/apple/aic \
    drivers/apple/ausb \
    drivers/apple/ccc \
    drivers/apple/dcs \
    drivers/apple/dwi \
    drivers/apple/gpio \
    drivers/apple/iic \
    drivers/apple/reconfig \
    drivers/apple/sep \
    drivers/samsung/uart \
    drivers/synopsys/usbotg \
    drivers/dialog/pmu \
    drivers/apple/voltage_knobs

ifneq ($(BUILD),RELEASE)
MODULES_BASIC += \
    drivers/apple/consistent_debug
endif

ifeq ($(BOOT_CONFIG), nand)
MODULES_FIRMWARE += \
    platform/$(PLATFORM)/apcie \
    drivers/pci \
    drivers/apple/apcie \
    drivers/apple/dart_lpae \
    drivers/nvme
endif

ifeq ($(BOOT_CONFIG), nor)
MODULES_BASIC += \
    drivers/samsung/spi

MODULES_FILESYSTEM += \
    drivers/flash_nor/spi
endif

LIBRARY_MODULES += \
    lib/libcorecrypto
