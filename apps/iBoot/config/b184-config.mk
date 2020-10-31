# Copyright (C) 2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# B184 iboot bootloader build config
TARGET			:=	b184
TARGET_HAS_BASEBAND	:=	0


PLATFORM		:=	s5l8960x
ARCH			:=	arm64
HW_TIMER		:=	architected
BOOT_CONFIG		:=	nand
AMC_REG_VERSION		:=	3
AMP_FILE_VERSION	:=	2

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
	drivers/apple/amp \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sep \
	drivers/dialog/pmu \
	drivers/nxp/cbtl1610 \
	drivers/samsung/dwi \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/ti/ths7383

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

ifeq ($(PRODUCT),iBoot)
# Memory configuration (consumed by the S5L8960X platform module)
TEXT_BANK		:=	sdram
TEXT_FOOTPRINT		:=	1024*1024
endif

ifeq ($(PRODUCT),iBEC)
# Memory configuration (consumed by the S5L8960X platform module)
TEXT_BANK		:=	sdram
TEXT_FOOTPRINT		:=	1024*1024
endif

ifeq ($(PRODUCT),iBSS)
# Memory configuration (consumed by the S5L8960X platform module)
TEXT_BANK		:=	sram
TEXT_FOOTPRINT		:=	256*1024
endif

ifeq ($(PRODUCT),LLB)
# Memory configuration (consumed by the S5L8960X platform module)
TEXT_BANK		:=	sram
TEXT_FOOTPRINT		:=	256*1024
endif

# override default platform display region size config
# TODO: We don't have a display. Can we reclaim this memory? (s5l8960x's memmap assumes one)
#DISPLAY_SIZE		:=	64*1024*1024
TZ0_SIZE	:=	6*1024*1024


include $(APPDIR)/products.mk

