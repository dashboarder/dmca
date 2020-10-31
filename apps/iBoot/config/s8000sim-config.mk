# Copyright (C) 2012-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# fastsim/s8000sim iboot bootloader build config

PLATFORM		:=	s8000
SUB_PLATFORM		?=	s8000
ARCH			:=	arm64
HW_TIMER		:=	architected
ADBE_VERSION		:=	2

OPTIONS += \
	DISPLAY_IPHONE_TUNABLES=1 \
	TARGET_DISPLAY_D520=1 \
	DISPLAY_D520_TUNABLES=1

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
	drivers/apple/gpio \
	drivers/apple/sep \
	drivers/apple/swifterpmu \
	drivers/samsung/uart \
	drivers/synopsys/usbotg

ifneq ($(BUILD),RELEASE)
MODULES_BASIC		+= \
	drivers/apple/consistent_debug
endif

MODULES_FIRMWARE	+= \
	platform/$(PLATFORM)/apcie \
	drivers/pci \
	drivers/apple/apcie \
	drivers/apple/dart_lpae \
	drivers/nvme

LIBRARY_MODULES		+= \
	lib/libcorecrypto

MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/displaypipe \
	drivers/apple/reconfig

include $(APPDIR)/products.mk

# You can override the default SRAM/SDRAM configuration here but you probably shouldn't.

