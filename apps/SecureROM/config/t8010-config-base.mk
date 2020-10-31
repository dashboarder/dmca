# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

###############################################################################
# Target configuration for the t7000 SecureROM module
#

PLATFORM			:=	t8010
ARCH				:=	arm64
HW_TIMER			:= 	architected

# Memory configuration (consumed by the t8010 platform module)
ifeq ($(TEXT_AREA),ROM)
TEXT_BANK	:=	srom
TEXT_FOOTPRINT	:=	512*1024
endif

MODULES		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/apcie \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	drivers/apple/a7iop \
	drivers/apple/aes_v2 \
	drivers/apple/aic \
	drivers/apple/apcie \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/apple/dart_lpae \
	drivers/apple/gpio \
	drivers/apple/sep \
	drivers/flash_nor/spi \
	drivers/nvme \
	drivers/pci \
	drivers/samsung/spi \
	drivers/synopsys/usbotg

ifeq ($(BUILD),DEBUG)
MODULES		+= \
	drivers/samsung/uart

OPTIONS		+= \
	WITH_PLATFORM_UARTCONFIG=1
endif

LIBRARY_MODULES		+= \
		lib/libcorecrypto
