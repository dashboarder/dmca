# Copyright (C) 2010-2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# K93a iboot bootloader build config
TARGET			:=	k93a
PLATFORM		:=	s5l8940x
SUB_PLATFORM		:=	s5l8942x
ARCH			:=	arm
NO_AMP_CALIBRATION	:=	true

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amg \
	drivers/apple/audio \
	drivers/apple/cdma \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sha2 \
	drivers/dialog/pmu \
	drivers/iic \
	drivers/samsung/pke \
	drivers/samsung/uart \
	drivers/samsung/usbphy \
	drivers/synopsys/usbotg \
	drivers/power/hdqgauge

MODULES_BOOT		+= \
	drivers/apple/adfe \
	drivers/apple/displaypipe \
	drivers/display/pinot \
	drivers/samsung/clcd_v2 \
	drivers/samsung/mipi

# Define NOR and NAND technology submodule lists for use only within
# the context of 'boot-from-nor-template.mk' and
# 'boot-from-nand-template.mk'.

SUBMODULES_NOR		+= 

SUBMODULES_NAND		+= \
	drivers/apple/h2fmi \
	drivers/flash_nand/ppn-swiss

include $(APPDIR)/config/boot-from-nand-template.mk

ifeq ($(PRODUCT),iBoot)
# Memory configuration (consumed by the S5L8940X platform module)
TEXT_BANK		:=	sdram
TEXT_FOOTPRINT		:=	1008*1024
endif

ifeq ($(PRODUCT),iBEC)
# Memory configuration (consumed by the S5L8940X platform module)
TEXT_BANK		:=	sdram
TEXT_FOOTPRINT		:=	1008*1024
endif

ifeq ($(PRODUCT),iBSS)
# Memory configuration (consumed by the S5L8940X platform module)
TEXT_BANK		:=	sram
TEXT_FOOTPRINT		:=	256*1024
endif

ifeq ($(PRODUCT),LLB)
# Memory configuration (consumed by the S5L8940X platform module)
TEXT_BANK		:=	sram
TEXT_FOOTPRINT		:=	256*1024
endif

include $(APPDIR)/products.mk
