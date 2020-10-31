# Copyright (C) 2007-2010 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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

################################################################################
# Per-CPU configuration 
################################################################################

#
# Apple Swift
#
ifeq ($(ARM_CPU),apple-swift)
ARM_ARCH ?= armv7
ARM_ARCH_OPTIONS := ARCH_ARMv7=1 WITH_VFP=1
OPTIONS += \
	CPU_APPLE_SWIFT=1 \
	WITH_MMU=1 \
	WITH_MMU_SECURITY_EXTENSIONS=1 \
	WITH_CACHE=1 \
	CPU_CACHELINE_SIZE=64 \
	CPU_CACHELINE_SHIFT=6 \
	CPU_CACHEINDEX_SHIFT=8 \
	CPU_CACHESET_SHIFT=3 \
	WITH_BRANCH_PREDICTION=1 \
	WITH_UNALIGNED_MEM=1 \
	WITH_L1_PARITY=1 \
	PAGE_SIZE=4096
ALL_OBJS += \
	$(LOCAL_DIR)/cache_v7.o \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/fp.o
endif

#
# ARM Cortex A8
#
ifeq ($(ARM_CPU),cortex-a8)
ARM_ARCH ?= armv7
ARM_ARCH_OPTIONS := ARCH_ARMv7=1 WITH_VFP=1
OPTIONS += \
	CPU_ARM_CORTEX_A8=1 \
	WITH_MMU=1 \
	WITH_MMU_SECURITY_EXTENSIONS=1 \
	WITH_CACHE=1 \
	CPU_CACHELINE_SIZE=64 \
	CPU_CACHELINE_SHIFT=6 \
	CPU_CACHEINDEX_SHIFT=7 \
	CPU_CACHESET_SHIFT=2 \
	WITH_BRANCH_PREDICTION=1 \
	WITH_UNALIGNED_MEM=1 \
	WITH_L1_PARITY=1 \
	WITH_CACHE_DEBUG=1 \
	WITH_ARCHITECTED_L2=1 \
	WITH_AUX_L2_ENABLE_BIT=1 \
	PAGE_SIZE=4096
ALL_OBJS += \
	$(LOCAL_DIR)/cache_v7.o \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/fp.o \
	$(LOCAL_DIR)/cachedebug_a8.o

# Turned off to save space, enable as needed
ifeq ($(APPLICATION),XXX_iBoot)
ALL_OBJS += \
	$(LOCAL_DIR)/cortex_hang.o \
	$(LOCAL_DIR)/cortex_debug.o
endif
endif

#
# ARM Cortex A9
#
ifeq ($(ARM_CPU),cortex-a9)
ARM_ARCH ?= armv7
ARM_ARCH_OPTIONS := ARCH_ARMv7=1 WITH_VFP=1
OPTIONS += \
	CPU_ARM_CORTEX_A9=1 \
	WITH_MMU=1 \
	WITH_MMU_SECURITY_EXTENSIONS=1 \
	WITH_CACHE=1 \
	CPU_CACHELINE_SIZE=32 \
	CPU_CACHELINE_SHIFT=5 \
	CPU_CACHEINDEX_SHIFT=8 \
	CPU_CACHESET_SHIFT=2 \
	WITH_BRANCH_PREDICTION=1 \
	WITH_UNALIGNED_MEM=1 \
	WITH_L1_PARITY=1 \
	SET_AUX_SMP_BIT=1 \
	PAGE_SIZE=4096
ALL_OBJS += \
	$(LOCAL_DIR)/cache_v7.o \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/fp.o
endif

#
# ARM Cortex A5 "Sparrow"
#
ifeq ($(ARM_CPU),cortex-a5)
ARM_ARCH ?= armv7
ARM_ARCH_OPTIONS := ARCH_ARMv7=1 WITH_VFP=1
OPTIONS += \
	CPU_ARM_CORTEX_A5=1 \
	FP_REGISTER_COUNT=32 \
	WITH_MMU=1 \
	WITH_MMU_SECURITY_EXTENSIONS=1 \
	WITH_CACHE=1 \
	CPU_CACHELINE_SIZE=32 \
	CPU_CACHELINE_SHIFT=5 \
	CPU_CACHEINDEX_SHIFT=8 \
	CPU_CACHESET_SHIFT=2 \
	WITH_BRANCH_PREDICTION=1 \
	WITH_UNALIGNED_MEM=1 \
	SET_AUX_SMP_BIT=1 \
	PAGE_SIZE=4096
ALL_OBJS += \
	$(LOCAL_DIR)/cache_v7.o \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/fp.o
endif

#
# ARM Cortex A7 "Kingfisher"
#
ifeq ($(ARM_CPU),cortex-a7)
ARM_ARCH ?= armv7k
ARM_ARCH_OPTIONS := ARCH_ARMv7=1 WITH_VFP=1 WITH_VFP_ALWAYS_ON=1
OPTIONS += \
	CPU_ARM_CORTEX_A7=1 \
	VFP_REV=4 \
	FP_REGISTER_COUNT=32 \
	WITH_MMU=1 \
	WITH_MMU_SECURITY_EXTENSIONS=1 \
	WITH_CACHE=1 \
	WITH_EARLY_ICACHE=1 \
	CPU_CACHELINE_SIZE=64 \
	CPU_CACHELINE_SHIFT=6 \
	CPU_CACHEINDEX_SHIFT=7 \
	CPU_CACHESET_SHIFT=2 \
	WITH_BRANCH_PREDICTION=1 \
	WITH_UNALIGNED_MEM=1 \
	WITH_ARCHITECTED_L2=1 \
	SET_AUX_SMP_BIT=1 \
	PAGE_SIZE=4096
ALL_OBJS += \
	$(LOCAL_DIR)/cache_v7.o \
	$(LOCAL_DIR)/mmu.o \
	$(LOCAL_DIR)/fp.o
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
LIBRARY_OPTIONS		+= ARCH=arm ARM_ARCH=$(ARM_ARCH)

# Always build libbuiltin, to partially override the gcc static library.
LIBRARY_MODULES		+= lib/libbuiltin

# set up to build with thumb
WITH_THUMB ?= true
ifeq ($(WITH_THUMB),true)
THUMBFLAGS		:= -mthumb
LIBRARY_TAG		+= thumb
else
THUMBFLAGS		:= -mno-thumb
LIBRARY_TAG		+= arm
endif

GLOBAL_ALLFLAGS		+= $(THUMBFLAGS)
GLOBAL_INCLUDES		+= $(LOCAL_DIR)/include

# Defaults for stack sizes
# These are override in platform/.../rules.mk
ifeq ($(IRQ_STACK_SIZE),)
  IRQ_STACK_SIZE	?= 2048
endif
ifeq ($(FIQ_STACK_SIZE),)
  FIQ_STACK_SIZE	?= 1024
endif
ifeq ($(EXCEPTION_STACK_SIZE),)
  EXCEPTION_STACK_SIZE	?= 2048
endif
ifeq ($(BOOTSTRAP_STACK_SIZE),)
  BOOTSTRAP_STACK_SIZE	?= 2048
endif
OPTIONS += \
	IRQ_STACK_SIZE=$(IRQ_STACK_SIZE) \
	FIQ_STACK_SIZE=$(FIQ_STACK_SIZE) \
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
	$(LOCAL_DIR)/exceptions.o \
	$(LOCAL_DIR)/handlers.o \
	$(LOCAL_DIR)/task.o

LINKER_EXPORTS := $(LOCAL_DIR)/link.exp

# if someone requests DCC, add it in
ifeq ($(WITH_DCC),true)
OPTIONS		+=	ARM_DCC_SYNCHRONOUS=1
include $(LOCAL_DIR)/dcc/rules.mk
endif

