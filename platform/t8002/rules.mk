# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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

ARM_CPU		:= cortex-a7

# Devmap chip ID
DEVMAP_CHIP_ID := 8002

SPDS_CHIP_REV		:= b0

IRQ_STACK_SIZE		:= 4096

# srom is always defined in here
ifeq ($(TEXT_BANK),srom)
 TEXT_BASE	:= 0x00000000
endif

# Overriding SRAM_LEN is not allowed.
ifeq ($(TEXT_BANK),sram)
 # SRAM TEXT_BASE is fixed by SecureROM.
 TEXT_BASE	:= 0x48818000

 # iBSS/LLB memory configuration
 ifeq ($(TEXT_FOOTPRINT),)
  TEXT_FOOTPRINT:= 1024*1024
 endif
endif

# platform target can override sdram config by specifying this in target config file (apps/iBoot/$target-config.mk)
ifeq ($(SDRAM_LEN),)
 SDRAM_LEN	:= 512*1024*1024

 # iBoot/iBEC memory configuration
 ifeq ($(TEXT_BANK),sdram)
  # SDRAM TEXT_BASE is now set such that you don't have to do lots of math to
  # calulate the correct value if you change any of the other region sizes:
  #	TEXT_BASE = SDRAM_BASE + SDRAM_LEN - 64MB
  TEXT_BASE	:= 0x9C000000

  ifeq ($(TEXT_FOOTPRINT),)
   TEXT_FOOTPRINT	:= 1024*1024
  endif
 endif
endif

ifeq ($(TEXT_BANK),srom)
  MAX_DFU_SIZE	:= 128*1024
endif
ifeq ($(TEXT_BANK),sram)
  MAX_DFU_SIZE	:= 1056*1024
endif

# Platform target can override any of these sizes by specifying in target config file (apps/iBoot/$target-config.mk)
ifeq ($(TZ0_SIZE),)
 TZ0_SIZE	:= 8*1024*1024
endif
ifeq ($(DISPLAY_SIZE),)
 DISPLAY_SIZE	:= 4*1024*1024
endif

OPTIONS		+= \
	AIC_CPU_ID=0 \
	L2_CACHE_SIZE=512*1024 \
	L2_CACHE_SETS=10 \
	WITH_NON_COHERENT_DMA=1 \
	ANC_PPNNPL_DS_DRIVE_STRENGTH=6 \
	ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_HOLD_TIME=1 \
	ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_SETUP_TIME=1 \
	ANC_LINK_SDR_REN_HOLD_TIME=0 \
	ANC_LINK_SDR_REN_SETUP_TIME=1 \
	ANC_LINK_SDR_WEN_HOLD_TIME=0 \
	ANC_LINK_SDR_WEN_SETUP_TIME=1 \
	ANC_LINK_SDR_DATA_CAPTURE_DELAY=0 \
	ANC_LINK_SDR_CLE_ALE_SETUP_TIME=1 \
	ANC_BOOT_CONTROLLERS=1 \
	ANC_TOGGLE_SUPPORTED=0 \
	DISPLAY_SIZE="$(DISPLAY_SIZE)ULL" \
	PLATFORM_ENTROPY_RATIO=200 \
	PLATFORM_IRQ_COUNT=256 \
	PLATFORM_SPDS_CHIP_REV=$(PLATFORM)/$(SPDS_CHIP_REV) \
	SDRAM_LEN="$(SDRAM_LEN)ULL" \
	TZ0_SIZE="$(TZ0_SIZE)ULL" \
	TEXT_BASE="$(TEXT_BASE)" \
	TEXT_FOOTPRINT="$(TEXT_FOOTPRINT)" \
	WITH_DPA=1 \
	WITH_DMA_CLEAR=1

# This platform contains corecrypto, needs large idle task stack for DFU stages.
ifeq ($(PRODUCT),iBSS)
  OPTIONS	+= \
	IDLE_TASK_SIZE=0x3000 \
	PLATFORM_START_FUNCTION=_platform_start
endif

ifeq ($(PRODUCT),LLB)
  OPTIONS	+= \
	IDLE_TASK_SIZE=0x3000 \
	PLATFORM_START_FUNCTION=_platform_start
endif

ifeq ($(APPLICATION),SecureROM)
 DATA_BASE	:= 0x48800000
 OPTIONS		+= \
	IDLE_TASK_SIZE=0x3000 \
	WITH_ROM_TRAMPOLINE=1 \
	DATA_BASE="$(DATA_BASE)"

 GLOBAL_LDFLAGS	+= \
	-segaddr __DATA $(DATA_BASE)
 ifeq ($(CONFIGS),fpga)
  OPTIONS		+= \
	SUPPORT_FPGA=1
 endif
endif

GLOBAL_LDFLAGS	+= \
	-seg1addr $(TEXT_BASE)


ALL_OBJS	+= \
	$(LOCAL_DIR)/asm.o \
	$(LOCAL_DIR)/trampoline.o \
	$(LOCAL_DIR)/init.o
