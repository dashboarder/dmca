# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# CAPRIREF iboot bootloader build config

TARGET			:=	capriref
PLATFORM		:=	t7000
SUB_PLATFORM		?=	t7001
ARCH			:=	arm64
BOOT_CONFIG		:=	nand
HW_TIMER		:=	architected
AMC_REG_VERSION		?=	5
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
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp_v3 \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sep \
	drivers/apple/aes \
	drivers/dialog/pmu \
	drivers/samsung/uart \
	drivers/apple/voltage_knobs \
	drivers/samsung/dwi \
	drivers/synopsys/usbotg

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
	platform/$(PLATFORM)/apcie

# override default platform display region size config
DISPLAY_SIZE		:=	64*1024*1024
TZ0_SIZE		:=	12*1024*1024

include $(APPDIR)/products.mk

