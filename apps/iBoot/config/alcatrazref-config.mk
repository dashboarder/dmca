# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# AlkatrazRef iboot bootloader build config
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
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sep \
	drivers/apple/aes \
	drivers/dialog/pmu \
	drivers/samsung/dwi \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/power/hdqgauge

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
	drivers/apple/adbe \
	drivers/apple/dither \
	drivers/apple/dpb \
	drivers/apple/adfe \
	drivers/apple/displaypipe \

TZ0_SIZE		:=	6*1024*1024

include $(APPDIR)/products.mk

