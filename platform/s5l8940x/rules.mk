# Copyright (C) 2009-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR	:= $(GET_LOCAL_DIR)

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

ARM_CPU		:= cortex-a9

MAX_DFU_SIZE	:= 180224

ifeq ($(TEXT_BANK),srom)
TEXT_BASE	:= 0x00000000
endif
ifeq ($(TEXT_BANK),sram)
TEXT_BASE	:= 0x34000000
endif
ifeq ($(TEXT_BANK),sdram)
TEXT_BASE	:= 0x9FF00000
endif

# Platform target can override any of these sizes by specifying in target config file (apps/iBoot/$target-config.mk)
ifeq ($(DISPLAY_SIZE),)
 DISPLAY_SIZE       := 15*1024*1024
endif

OPTIONS		+= \
	AIC_CPU_ID=0 \
	WITH_CLASSIC_SUSPEND_TO_RAM=1 \
	WITH_LEGACY_PANIC_LOGS=1 \
	WITH_NO_RANDOM_HEAP_COOKIE=1 \
	WITH_NO_RANDOM_STACK_COOKIE=1 \
	WITH_NON_COHERENT_DMA=1 \
	WITH_ROM_TRAMPOLINE=1 \
	PLATFORM_ENTROPY_RATIO=200 \
	PLATFORM_IRQ_COUNT=192 \
	TEXT_BASE="$(TEXT_BASE)" \
	DISPLAY_SIZE="($(DISPLAY_SIZE)ULL)" \
	TEXT_FOOTPRINT="$(TEXT_FOOTPRINT)" \
	DISPLAY_BASE_NONALIGNED

#	L2_CACHE_SIZE=1048576 \
#

GLOBAL_LDFLAGS	+= \
	-seg1addr $(TEXT_BASE)

ifeq ($(APPLICATION),SecureROM)
# This selects the start of the DATA segment and the end of the INSECURE_MEMORY area
DATA_BASE	:= 0x3402C000

OPTIONS		+= \
	DATA_BASE="$(DATA_BASE)"

GLOBAL_LDFLAGS	+= \
	-segaddr __DATA $(DATA_BASE)
endif

ALL_OBJS	+= $(LOCAL_DIR)/init.o \
		   $(LOCAL_DIR)/pinconfig_$(SUB_PLATFORM).o

endif

##############################################################################
# Code running on the IOP
#
ifeq ($(PLATFORM_VARIANT),IOP)

OPTIONS		+= \
	PLATFORM_VARIANT_IOP=1 \
	AIC_CPU_ID=2 \
	WITH_PLATFORM_HALT=1 \
	SUPPORT_SLEEP=1 \
	WITH_IOP_POWER_GATING=1 \
	DEEP_IDLE_THRESHOLD_US=10000

ARM_CPU		:=	cortex-a5

TEXT_BASE	:=	0x0

OPTIONS		+= \
	TEXT_BASE="$(TEXT_BASE)"

ALL_OBJS	+= $(LOCAL_DIR)/iop_init.o
endif

##############################################################################
# Code running on the audio engine
#
ifeq ($(PLATFORM_VARIANT),Audio)

OPTIONS		+= \
	PLATFORM_VARIANT_AUDIO=1 \
	WITH_PLATFORM_HALT=1 \
	SUPPORT_SLEEP=1

ARM_CPU		:=	cortex-a5

TEXT_BASE	:=	0x0

OPTIONS		+= \
	TEXT_BASE="$(TEXT_BASE)" \
	TEXT_FOOTPRINT="$(TEXT_FOOTPRINT)"

ALL_OBJS	+= $(LOCAL_DIR)/iop_init.o

endif
