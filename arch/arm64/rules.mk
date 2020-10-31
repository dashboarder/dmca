# Copyright (C) 2011-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

#
# ARM CPU config 
#

# sanity check
ifeq ($(ARM_CPU),)
$(error ARM_CPU is not set)
endif

ifneq ($(ARM_ARCH),)
$(info ARM_ARCH does not need to be set at the platform level)
endif

ARCH_ARMV8_CPUS := apple-cyclone apple-typhoon apple-typhoon-capri apple-twister apple-hurricane-zephyr

################################################################################
# Per-CPU configuration 
################################################################################

#
# Apple Cyclone/Typhoon
#
ifneq (,$(filter $(ARCH_ARMV8_CPUS),$(ARM_CPU)))
ARM_ARCH ?= arm64
ARM_ARCH_OPTIONS := ARCH_ARMv8=1 WITH_VFP=1 WITH_VFP_ALWAYS_ON=1
OPTIONS += \
	CPU_APPLE_CYCLONE=1 \
	L1_CACHELINE_SIZE=64 \
	L1_CACHELINE_SHIFT=6 \
	CPU_CACHELINE_SIZE=L1_CACHELINE_SIZE \
	CPU_CACHELINE_SHIFT=L1_CACHELINE_SHIFT \
	WITH_HW_TIMER=1

ALL_OBJS += \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/timer.o

ifneq (,$(filter apple-cyclone apple-typhoon,$(ARM_CPU)))
WITH_EL3 := 1
OPTIONS += \
	L1_CACHEINDEX_SHIFT=9 \
	L1_CACHEWAY_SHIFT=1 \
	L2_CACHE_SIZE=1048576 \
	L2_CACHEINDEX_SHIFT=11 \
	L2_CACHEWAY_SHIFT=3 \
	L2_CACHEWAY_COUNT=8 \
	L2_CACHELINE_SHIFT=6 \
	PAGE_GRANULE_SHIFT=12 \
	PAGE_SIZE=4096 \
	WITH_EL3=1 \
	WITH_MCC_AS_RAM=1

#4k-algined linker pages
GLOBAL_LDFLAGS		+=	-Wl,-segalign,0x1000

else ifneq (,$(filter apple-typhoon-capri,$(ARM_CPU)))
WITH_EL3 := 1
OPTIONS += \
	L1_CACHEINDEX_SHIFT=9 \
	L1_CACHEWAY_SHIFT=1 \
	L2_CACHE_SIZE=2097152 \
	L2_CACHEINDEX_SHIFT=12 \
	L2_CACHEWAY_SHIFT=3 \
	L2_CACHEWAY_COUNT=8 \
	L2_CACHELINE_SHIFT=6 \
	PAGE_GRANULE_SHIFT=12 \
	PAGE_SIZE=4096 \
	WITH_EL3=1 \
	WITH_MCC_AS_RAM=1

#4k-algined linker pages
GLOBAL_LDFLAGS		+=	-Wl,-segalign,0x1000

else ifneq (,$(filter apple-twister,$(ARM_CPU)))
WITH_EL3 := 1
OPTIONS += \
	L1_CACHEINDEX_SHIFT=8 \
	L1_CACHEWAY_SHIFT=2 \
	L2_CACHE_SIZE=3145728 \
	L2_CACHEINDEX_SHIFT=12 \
	L2_CACHEWAY_SHIFT=4 \
	L2_CACHEWAY_COUNT=12 \
	L2_CACHELINE_SHIFT=6 \
	PAGE_GRANULE_SHIFT=14 \
	PAGE_SIZE=16384 \
	WITH_EL3=1 \
	WITH_MCC_AS_RAM=1

#16k-algined linker pages
GLOBAL_LDFLAGS		+=	-Wl,-segalign,0x4000

else ifneq (,$(filter apple-hurricane-zephyr,$(ARM_CPU)))
OPTIONS += \
	L1_CACHEINDEX_SHIFT=8 \
	L1_CACHEWAY_SHIFT=2 \
	L2_CACHE_SIZE=3145728 \
	L2_CACHEINDEX_SHIFT=11 \
	L2_CACHEWAY_SHIFT=4 \
	L2_CACHEWAY_COUNT=12 \
	L2_CACHELINE_SHIFT=7 \
	PAGE_GRANULE_SHIFT=14 \
	PAGE_SIZE=16384 \
	WITH_L2_AS_RAM=1

#16k-algined linker pages
GLOBAL_LDFLAGS		+=	-Wl,-segalign,0x4000

endif

endif

################################################################################
# General and conditional configuration
################################################################################

# sanity check
ifeq ($(ARM_ARCH),)
$(error ARM_ARCH not set, no valid CPU selected)
endif

# build for the proper cpu
OPTIONS			+= ARCH_ARM=1
GLOBAL_ALLFLAGS		+= -arch $(ARM_ARCH) $(addprefix -D,$(ARM_ARCH_OPTIONS))
GLOBAL_LDFLAGS		+= -arch $(ARM_ARCH)

# Tell the build system how libraries should be tagged
LIBRARY_TAG		+= $(ARM_ARCH)
LIBRARY_OPTIONS		+= ARCH=arm64 ARM_ARCH=$(ARM_ARCH)

GLOBAL_INCLUDES		+= arch/arm/include
GLOBAL_INCLUDES		+= $(LOCAL_DIR)/include

# Defaults for stack sizes (stacks laid out from top to bottom as listed)
INTERRUPT_STACK_SIZE	?= 8192
BOOTSTRAP_STACK_SIZE	?= 4096
EXCEPTION_STACK_SIZE	?= 4096
OPTIONS += \
	INTERRUPT_STACK_SIZE=$(INTERRUPT_STACK_SIZE) \
	EXCEPTION_STACK_SIZE=$(EXCEPTION_STACK_SIZE) \
	BOOTSTRAP_STACK_SIZE=$(BOOTSTRAP_STACK_SIZE)

# Force start.o and asm.o to link first
ALL_OBJS := \
	$(LOCAL_DIR)/start.o \
	$(LOCAL_DIR)/asm.o \
	$(ALL_OBJS)

# Code common to all cores
ALL_OBJS += \
	$(LOCAL_DIR)/cpu.o \
	$(LOCAL_DIR)/context.o \
	$(LOCAL_DIR)/entropy.o \
	$(LOCAL_DIR)/fp.o \
	$(LOCAL_DIR)/handlers.o \
	$(LOCAL_DIR)/task.o

ifeq ($(WITH_EL3),1)
ALL_OBJS += \
	$(LOCAL_DIR)/exceptions_el3.o
else
ALL_OBJS += \
	$(LOCAL_DIR)/exceptions_el1.o
endif

LINKER_EXPORTS := $(LOCAL_DIR)/link.exp
