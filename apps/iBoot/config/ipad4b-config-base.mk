# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# iPad4,x iboot bootloader build config
PLATFORM		:=	s5l8960x
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
	platform/$(PLATFORM)/error_handler \
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
	drivers/nxp/cbtl1610 \
	drivers/samsung/dwi \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/power/hdqgauge 

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
	drivers/apple/adbe \
	drivers/apple/adfe \
	drivers/apple/dither \
	drivers/apple/dpb	\
	drivers/apple/displaypipe \
	drivers/display/edp \
	drivers/displayport \
	drivers/samsung/displayport

# override platform default memory map
ASP_SIZE		:=	10*1024*1024
DISPLAY_SIZE		:=	64*1024*1024
ifeq ($(TARGET_HAS_MESA),1)
	TZ0_SIZE	:=	6*1024*1024
endif

include $(APPDIR)/products.mk

