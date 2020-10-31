# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# iPod7,1 iBoot bootloader build config
PLATFORM		:=	t7000
SUB_PLATFORM		?=	t7000
ARCH			:=	arm64
BOOT_CONFIG		:=	nand
HW_TIMER		:=	architected
AMC_REG_VERSION		?=	4
AMC_FILE_VERSION	?=	2
ADBE_VERSION		:=	2

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	platform/$(PLATFORM)/error_handler \
	drivers/apple/a7iop \
	drivers/apple/aes \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp_v3 \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/apple/voltage_knobs \
	drivers/samsung/dwi \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sep \
	drivers/dialog/pmu \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/nxp/cbtl1610

ifneq ($(BUILD),RELEASE)
MODULES_BASIC		+= \
	drivers/apple/consistent_debug
endif

ifeq ($(BOOT_CONFIG), nand)
MODULES_FILESYSTEM	+= \
	drivers/apple/asp \
	drivers/apple/csi \

MODULES_FIRMWARE	+= \
	drivers/apple/anc
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
	platform/$(PLATFORM)/apcie \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/dither \
	drivers/apple/dpb \
	drivers/apple/displaypipe \
	drivers/backlight/lm3534 \
	drivers/display/pinot \
	drivers/display_pmu/chestnut \
	drivers/synopsys/mipi

OPTIONS += \
	DISPLAY_D410_TUNABLES=1

# Override default platform memory map
ASP_SIZE		:=	10*1024*1024
DISPLAY_SIZE		:=	34*1024*1024
TZ0_SIZE		:=	8*1024*1024

include $(APPDIR)/products.mk

