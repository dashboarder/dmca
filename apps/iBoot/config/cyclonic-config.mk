# Copyright (C) 2011-2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# swifter/fastsim iboot bootloader build config
PLATFORM		:=	s5l8960x
SUB_PLATFORM		:=	s5l8960x
ARCH			:=	arm64
HW_TIMER		:=	architected
BOOT_CONFIG		:=	nand
AMC_REG_VERSION		:=	3
AMP_FILE_VERSION	:=	2
ADBE_VERSION		:=	1

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	drivers/apple/a7iop \
	drivers/apple/aes \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp \
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
	drivers/apple/csi \

MODULES_FIRMWARE    += \
	drivers/apple/anc

LIBRARY_MODULES		+= \
	lib/libcorecrypto

MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe \
	drivers/apple/displaypipe

include $(APPDIR)/products.mk

# You can override the default memory configuration here but you probably shouldn't.

