# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# fastsim/typhonic iboot bootloader build config

TARGET			:=	typhonic
PLATFORM		:=	t7000
SUB_PLATFORM		?=	t7000
ARCH			:=	arm64
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
	drivers/apple/gpio \
	drivers/apple/sep \
	drivers/apple/swifterpmu \
	drivers/samsung/uart \
	drivers/synopsys/usbotg

ifneq ($(BUILD),RELEASE)
MODULES_BASIC		+= \
	drivers/apple/consistent_debug
endif

MODULES_FILESYSTEM	+= \
	drivers/apple/asp \
	drivers/apple/csi

MODULES_FIRMWARE	+= \
	platform/$(PLATFORM)/apcie \
	drivers/apple/anc \
	drivers/pci \
	drivers/apple/apcie \
	drivers/apple/dart_lpae \
	drivers/nvme

LIBRARY_MODULES		+= \
	lib/libcorecrypto

MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/displaypipe

# Override platform default memory map
TZ0_SIZE		:=	12*1024*1024

include $(APPDIR)/products.mk

