# Copyright (C) 2010-2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# N94 iboot bootloader build config
PLATFORM		:=	s5l8940x
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
	drivers/flash_nand/ppn-swiss \

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

###########
# N94 Memory Map Explanation
############

# frame buffer size = hight (960) * stride (2560) = 0x258000
# iBoot and OS memory map use same frame buffer start address = 0x9f8f4000

# iBoot:
#
# REGION                       START                   END             SIZE                    NOTES
# Panic                        0x9fffc000              0x9fffffff      0x00004000
# iBoot                        0x9ff00000              0x9fffbfff      0x0000c000
# Display                      0x9f000000              0x9fefffff      0x00f00000              used for frame buffer, compressed and uncompressed artwork
#      [frame buffer           0x9f8f4000              0x9fb4bfff      0x00258000              frame buffer region = 1 x frame buffer size]
#      [flat image             0x9f25a000              0x9f7fffff      0x005a6000              uncompressed image]
#      [scratch                0x9f000000              0x9f258fff      0x00259000              compressed image; size = frame buffer size + 0x1000]
# Heap etc                     0x80000000              0x9f0effff      0x1f0f0000

# OS: [Display region is resized; iBoot region is removed]
#
# REGION                       START                   END             SIZE                    NOTES
# Panic                        0x9fffc000              0x9fffffff      0x00004000
# Display                      0x9f8f4000              0x9fffbfff      0x00708000              display size = 3 x frame buffer size
# Kernel                       0x80000000              0x9f8f3fff      0x1f8f4000

include $(APPDIR)/products.mk
