# Copyright (C) 2013-2014, 2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# iPhone8,x iBoot bootloader build config
PLATFORM		:=	s8000
ARCH			:=	arm64
BOOT_CONFIG		:=	nand
HW_TIMER		:=	architected
ADBE_VERSION		:=	2
TARGET			:=	iphone8

# code modules
MODULES_BASIC		+= \
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
	drivers/apple/sep \
	drivers/dialog/pmu \
	drivers/nxp/cbtl1610 \
	drivers/power/hdqgauge \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/ti/sn2400 \
	drivers/apple/voltage_knobs \

ifneq ($(BUILD),RELEASE)
MODULES_BASIC		+= \
	drivers/apple/consistent_debug
endif

ifeq ($(BOOT_CONFIG), nand)
MODULES_FIRMWARE	+= \
	platform/$(PLATFORM)/apcie \
	drivers/pci \
	drivers/apple/apcie \
	drivers/apple/dart_lpae \
	drivers/nvme
endif

ifeq ($(BOOT_CONFIG), nor)
MODULES_BASIC		+= \
	drivers/samsung/spi

MODULES_FILESYSTEM	+= \
	drivers/flash_nor/spi
endif

LIBRARY_MODULES		+= \
	lib/libcorecrypto

MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/dither \
	drivers/apple/dpb \
	drivers/apple/displaypipe \
	drivers/apple/reconfig \
	drivers/backlight/lm3534 \
	drivers/display/pinot \
	drivers/display_pmu/chestnut \
	drivers/synopsys/mipi

# Override platform default memory map
ASP_SIZE		:=	12*1024*1024

include $(APPDIR)/products.mk

