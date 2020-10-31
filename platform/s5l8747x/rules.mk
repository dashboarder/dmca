# Copyright (C) 2011-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

# modules implied by this platform
MODULES		+= \
	platform/defaults \
	platform/generic \
	arch/$(ARCH)

##############################################################################
# Code running on the main CPU
#
ifeq ($(PLATFORM_VARIANT),)

ARM_ARCH := armv7
ARM_CPU := cortex-a5

# Devmap chip ID based on sub-platform identifier
ifeq ($(SUB_PLATFORM),s5l8747x)
DEVMAP_CHIP_ID := 8747
endif

MAX_DFU_SIZE	:=	98304

ifeq ($(TEXT_BANK),srom)
TEXT_BASE := 0x00000000
endif
ifeq ($(TEXT_BANK),sram)
TEXT_BASE := 0x22000000
endif
ifeq ($(TEXT_BANK),sdram)
TEXT_BASE := 0x0FF00000
endif

OPTIONS += \
	WITH_CLASSIC_SUSPEND_TO_RAM=1 \
	WITH_LEGACY_PANIC_LOGS=1 \
	WITH_NON_COHERENT_DMA=1 \
	WITH_ROM_TRAMPOLINE=1 \
	WITH_CONJOINED_USB_PHYS=1 \
	PLATFORM_ENTROPY_RATIO=60 \
	PLATFORM_IRQ_COUNT=192 \
	TEXT_BASE="$(TEXT_BASE)" \
	TEXT_FOOTPRINT="$(TEXT_FOOTPRINT)"

GLOBAL_LDFLAGS += -seg1addr $(TEXT_BASE)

ifeq ($(APPLICATION),SecureROM)
# This selects the start of the DATA segment and the end of the INSECURE_MEMORY area
DATA_BASE := 0x22019000

OPTIONS += \
	DATA_BASE="$(DATA_BASE)"

GLOBAL_LDFLAGS += -segaddr __DATA $(DATA_BASE)

# Simulation trace uses STATESAVE2 and STATESAVE3
# We have to duplicate the literal values here as debug.h is 
# not necessarily going have hwregbase.h in scope all the time.
OPTIONS	+= \
	SIMULATION_TRACE_IP_REGISTER=0x3970004c \
	SIMULATION_TRACE_PARAMETER_REGISTER=0x39700050
endif

# SecureROM turns off the timers on G1, so turn them back on as early
# as possible to get accurate measurements of boot time.
# By default only do this in development builds though.
ifeq ($(APPLICATION),iBoot)
ifeq ($(BUILD),DEVELOPMENT)
OPTIONS += \
	PLATFORM_START_FUNCTION=_platform_start
endif
endif

ALL_OBJS += \
	 $(LOCAL_DIR)/asm.o \
	 $(LOCAL_DIR)/init.o

MODULES_ELIDE += lib/libcorecrypto

endif

