# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# M7REF iboot bootloader build config

PLATFORM		:=	s7002
SUB_PLATFORM		:=	s7002
TARGET			:=	m7ref
ARCH			:=	arm
HW_TIMER		:=	pmgr
BOOT_CONFIG		:=	nand
AMC_REG_VERSION		:=	2
AMP_FILE_VERSION	:=	2

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	platform/$(PLATFORM)/reconfig \
	drivers/apple/a7iop \
	drivers/apple/aes_s7002 \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp \
	drivers/apple/ausb \
	drivers/apple/dockfifo \
	drivers/apple/gpio \
	drivers/primecell/pl080dmac \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/apple/iic \
	drivers/dialog/pmu \
	drivers/apple/voltage_knobs

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

include $(APPDIR)/products.mk
