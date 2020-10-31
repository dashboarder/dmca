# Copyright (C) 2011-2014 Apple Inc. All rights reserved.
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

ARM_CPU			:= apple-cyclone

# Devmap chip ID based on sub-platform identifier
ifeq ($(SUB_PLATFORM),s5l8960x)
 DEVMAP_CHIP_ID		:= 8960
endif

MAX_DFU_SIZE		:= 524288

# SROM is always defined here
ifeq ($(TEXT_BANK),srom)
 TEXT_BASE		:= 0x100000000
endif

# Overriding SRAM_LEN is not allowed.
ifeq ($(TEXT_BANK),sram)
 # SRAM TEXT_BASE is fixed by SecureROM.
 TEXT_BASE		:= 0x180380000

 # iBSS/LLB memory configuration
 ifeq ($(TEXT_FOOTPRINT),)
  TEXT_FOOTPRINT	:= 384*1024
 endif
endif

# Platform target can override SDRAM config by specifying this in target config file (apps/iBoot/$target-config.mk)
ifeq ($(SDRAM_LEN),)
 SDRAM_LEN		:= 1*1024*1024*1024

 # iBoot/iBEC memory configuration
 ifeq ($(TEXT_BANK),sdram)
  # SDRAM TEXT_BASE is now set such that you don't have to do lots of math to
  # calulate the correct value if you change any of the other region sizes:
  #	TEXT_BASE = SDRAM_BASE + SDRAM_LEN - 256MB
  TEXT_BASE		:= 0x830000000
  ifeq ($(TEXT_FOOTPRINT),)
   TEXT_FOOTPRINT	:= 1024*1024
  endif
 endif
endif

# Platform target can override any of these sizes by specifying in target config file (apps/iBoot/$target-config.mk)
ifeq ($(ASP_SIZE),)
 ASP_SIZE		:= 8*1024*1024
endif
ifeq ($(TZ0_SIZE),)
 TZ0_SIZE		:= 4*1024*1024
endif
ifeq ($(DISPLAY_SIZE),)
 DISPLAY_SIZE		:= 16*1024*1024
endif

ifeq ($(TEXT_BASE),)
 $(error TEXT_BASE has not been set)
endif
ifeq ($(TEXT_FOOTPRINT),)
 $(error TEXT_FOOTPRINT has not been set)
endif

OPTIONS		+= \
	AIC_CPU_ID=0 \
	WITH_CLASSIC_SUSPEND_TO_RAM=1 \
	ANC_PPNNPL_DS_DRIVE_STRENGTH=7 \
	ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_HOLD_TIME=2 \
	ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_SETUP_TIME=3 \
	ANC_LINK_SDR_REN_HOLD_TIME=2 \
	ANC_LINK_SDR_REN_SETUP_TIME=3 \
	ANC_LINK_SDR_WEN_HOLD_TIME=2 \
	ANC_LINK_SDR_WEN_SETUP_TIME=3 \
	ANC_LINK_SDR_DATA_CAPTURE_DELAY=1 \
	ANC_LINK_SDR_CLE_ALE_SETUP_TIME=0 \
	ANC_BOOT_CONTROLLERS=2 \
	ANC_TOGGLE_SUPPORTED=1 \
	DISPLAY_SIZE="($(DISPLAY_SIZE)ULL)" \
	PLATFORM_ENTROPY_RATIO=200 \
	PLATFORM_IRQ_COUNT=256 \
	PLATFORM_START_FUNCTION=_platform_start \
	SDRAM_LEN="$(SDRAM_LEN)ULL" \
	ASP_SIZE="$(ASP_SIZE)ULL" \
	TZ0_SIZE="$(TZ0_SIZE)ULL" \
	TEXT_BASE="$(TEXT_BASE)" \
	TEXT_FOOTPRINT="$(TEXT_FOOTPRINT)" \
	WITH_NO_RANDOM_HEAP_COOKIE=1 \
	WITH_NO_RANDOM_STACK_COOKIE=1

ifeq ($(APPLICATION),SecureROM)

OPTIONS		+= \
	WITH_ROM_TRAMPOLINE=1

ifeq ($(CONFIGS),fpga)
	SUPPORT_FPGA=1
endif

else

OPTIONS		+= \
	WITH_MONITOR=1

endif

GLOBAL_LDFLAGS	+= \
	-seg1addr $(TEXT_BASE)

ifeq ($(APPLICATION),SecureROM)
DATA_BASE	:= 0x180080000
OPTIONS		+= \
	DATA_BASE="$(DATA_BASE)"

GLOBAL_LDFLAGS	+= \
	-segaddr __DATA $(DATA_BASE)
endif

ALL_OBJS	+= \
	$(LOCAL_DIR)/asm.o \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/trampoline.o
