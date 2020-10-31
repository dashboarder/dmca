# Copyright (C) 2012-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# Lower cost s5l8950x based iPhone iboot bootloader build config
PLATFORM		:=	s5l8950x
ARCH			:=	arm
NO_AMP_CALIBRATION	:=	true
AMC_REG_VERSION		:=	2

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp \
	drivers/apple/audio \
	drivers/apple/cdma \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sha2 \
	drivers/apple/shmcon \
	drivers/dialog/pmu \
	drivers/iic \
	drivers/nxp/cbtl1610 \
	drivers/samsung/dwi \
	drivers/samsung/pke \
	drivers/samsung/uart \
	drivers/samsung/usbphy \
	drivers/synopsys/usbotg \
	drivers/power/hdqgauge \
	drivers/ti/ths7383

MODULES_BOOT		+= \
	drivers/apple/adfe \
	drivers/apple/displaypipe \
	drivers/backlight/lm3534 \
	drivers/display/pinot \
	drivers/display_pmu/chestnut \
	drivers/samsung/clcd_v2 \
	drivers/samsung/mipi

# Define NOR and NAND technology submodule lists for use only within
# the context of 'boot-from-nor-template.mk' and
# 'boot-from-nand-template.mk'.

SUBMODULES_NAND		+= \
	drivers/apple/h2fmi \
	drivers/flash_nand/ppn-swiss \

include $(APPDIR)/config/boot-from-nand-template.mk

ifeq ($(PRODUCT),iBoot)
# Memory configuration (consumed by the S5L8950X platform module)
TEXT_BANK		:=	sdram
TEXT_FOOTPRINT		:=	1008*1024
endif

ifeq ($(PRODUCT),iBEC)
# Memory configuration (consumed by the S5L8950X platform module)
TEXT_BANK		:=	sdram
TEXT_FOOTPRINT		:=	1008*1024
endif

ifeq ($(PRODUCT),iBSS)
# Memory configuration (consumed by the S5L8950X platform module)
TEXT_BANK		:=	sram
TEXT_FOOTPRINT		:=	512*1024
endif

ifeq ($(PRODUCT),LLB)
# Memory configuration (consumed by the S5L8950X platform module)
TEXT_BANK		:=	sram
TEXT_FOOTPRINT		:=	512*1024
endif

include $(APPDIR)/products.mk
