# Copyright (C) 2011 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# iaccy1 iboot bootloader build config
PLATFORM		:=	s5l8747x
ARCH			:=	arm

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	platform/$(PLATFORM)/clocks \
	drivers/primecell/pl192vic \
	drivers/samsung/aes \
	drivers/samsung/drex \
	drivers/samsung/gpio \
	drivers/samsung/pke \
	drivers/samsung/sha1 \
	drivers/samsung/timer \
	drivers/samsung/uart \
	drivers/samsung/usbphy \
	drivers/synopsys/usbotg

# We don't have flash, so there's no syscfg
MODULES_ELIDE		+= \
	$(MODULES_SYSCFG) \
	lib/paint

# The full console menu is too big, so use the simple
# menu everywhere (and not just on release builds).
# Ditto for printfs
OPTIONS			+= \
	WITH_SIMPLE_MENU=1 \
	OBFUSCATED_LOGGING=1

ifeq ($(PRODUCT),iBoot)
# Memory configuration (consumed by the S5L8747X platform module)
TEXT_FOOTPRINT		:=	1008*1024
endif

ifeq ($(PRODUCT),iBEC)
# Memory configuration (consumed by the S5L8747X platform module)
TEXT_FOOTPRINT		:=	1008*1024
endif

ifeq ($(PRODUCT),iBSS)
# Memory configuration (consumed by the S5L8747X platform module)
TEXT_FOOTPRINT		:=	128*1024

RECOVERY_MODE_IBSS	:=	true
endif

ifeq ($(PRODUCT),LLB)
# Memory configuration (consumed by the S5L8747X platform module)
TEXT_FOOTPRINT		:=	128*1024
endif

ifeq ($(BUILD),DEBUG)
ABORT := true
endif

include $(APPDIR)/products.mk
