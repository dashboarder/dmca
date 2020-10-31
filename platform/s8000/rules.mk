# Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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

# Devmap chip ID, CPU, and memory default configs based on sub-platform identifier
ifeq ($(SUB_PLATFORM),s8000)
 ARM_CPU		:= apple-twister
 DEVMAP_CHIP_ID		:= 8000
 DISPLAY_SIZE_DEFAULT	:= 17*1024*1024
 MAX_DFU_SIZE		:= 512*1024
 SDRAM_LEN_DEFAULT	:= 2*1024*1024*1024
 SDRAM_TEXT_BASE	:= 0x870000000
 SDRAM_TEXT_FOOTPRINT	:= 1024*1024
 SRAM_TEXT_BASE		:= 0x180380000
 SRAM_TEXT_FOOTPRINT	:= 416*1024
 SROM_DATA_BASE		:= 0x180080000
 SPDS_CHIP_REV		:= c0
else ifeq ($(SUB_PLATFORM),s8001)
 ARM_CPU		:= apple-twister
 DEVMAP_CHIP_ID		:= 8001
 DISPLAY_SIZE_DEFAULT	:= 64*1024*1024
 MAX_DFU_SIZE		:= 256*1024
 SDRAM_LEN_DEFAULT	:= 4*1024*1024*1024
 SDRAM_TEXT_BASE	:= 0x8F0000000
 SDRAM_TEXT_FOOTPRINT	:= 1024*1024
 SRAM_TEXT_BASE		:= 0x180000000
 SRAM_TEXT_FOOTPRINT	:= 304*1024
 SROM_DATA_BASE		:= 0x180044000
 SPDS_CHIP_REV		:= b0
else ifeq ($(SUB_PLATFORM),s8003)
 ARM_CPU		:= apple-twister
 DEVMAP_CHIP_ID		:= 8003
 DISPLAY_SIZE_DEFAULT	:= 17*1024*1024
 MAX_DFU_SIZE		:= 512*1024
 SDRAM_LEN_DEFAULT	:= 2*1024*1024*1024
 SDRAM_TEXT_BASE	:= 0x870000000
 SDRAM_TEXT_FOOTPRINT	:= 1024*1024
 SRAM_TEXT_BASE		:= 0x180380000
 SRAM_TEXT_FOOTPRINT	:= 416*1024
 SROM_DATA_BASE		:= 0x180080000
 SPDS_CHIP_REV		:= a1
else
  $(error "Unrecognized SUB_PLATFORM \"$(SUB_PLATFORM)\"")
endif

# SROM is always defined here
ifeq ($(TEXT_BANK),srom)
 TEXT_BASE		:= 0x100000000
endif

# Overriding  SRAM_LEN is not allowed.
ifeq ($(TEXT_BANK),sram)
 # SRAM TEXT_BASE is fixed by SecureROM.
 TEXT_BASE		:= $(SRAM_TEXT_BASE)

 # iBSS/LLB memory configuration
 ifeq ($(TEXT_FOOTPRINT),)
  TEXT_FOOTPRINT 	:= $(SRAM_TEXT_FOOTPRINT)
 endif
endif

# Platform target can override SDRAM config by specifying this in target config file (apps/iBoot/$target-config.mk)
ifeq ($(SDRAM_LEN),)
 SDRAM_LEN		:= $(SDRAM_LEN_DEFAULT)

 # iBoot/iBEC memory configuration
 ifeq ($(TEXT_BANK),sdram)
  # SDRAM TEXT_BASE is now set such that you don't have to do lots of math to
  # calulate the correct value if you change any of the other region sizes:
  #	TEXT_BASE = SDRAM_BASE + SDRAM_LEN - 256MB
  TEXT_BASE		:= $(SDRAM_TEXT_BASE)
  # Platform target can override SRAM config by specifying this in target config file (apps/iBoot/$target-config.mk)
  ifeq ($(TEXT_FOOTPRINT),)
   TEXT_FOOTPRINT	:= $(SDRAM_TEXT_FOOTPRINT)
  endif
 endif
endif

# Platform target can override any of these sizes by specifying in target config file (apps/iBoot/$target-config.mk)
ifeq ($(TZ0_SIZE),)
 TZ0_SIZE	:= 12*1024*1024
endif
ifeq ($(DISPLAY_SIZE),)
 DISPLAY_SIZE	:= $(DISPLAY_SIZE_DEFAULT)
endif

ifeq ($(TEXT_FOOTPRINT),)
 $(error TEXT_FOOTPRINT has not been set)
endif

# platform target can override ASP size by specifying this in target config file (apps/iBoot/$target-config.mk)
ifeq ($(ASP_SIZE),)
 ASP_SIZE      := 12*1024*1024
endif

OPTIONS		+= \
	AIC_CPU_ID=0 \
	ANC_PPNNPL_DS_DRIVE_STRENGTH=6 \
	ANC_PPNNPL_INPUT_SELECT_SCHMITT=1 \
	ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_HOLD_TIME=2 \
	ANC_LINK_CMD_ADDR_PULSE_TIMING_CA_SETUP_TIME=3 \
	ANC_LINK_SDR_DATA_CAPTURE_DELAY=1 \
	ANC_LINK_SDR_CLE_ALE_SETUP_TIME=0 \
	ANC_BOOT_CONTROLLERS=2 \
	ANC_TOGGLE_SUPPORTED=1 \
	PLATFORM_ENTROPY_RATIO=200 \
	PLATFORM_IRQ_COUNT=288 \
	PLATFORM_SPDS_CHIP_REV=$(SUB_PLATFORM)/$(SPDS_CHIP_REV) \
	PLATFORM_START_FUNCTION=_platform_start \
	SDRAM_LEN="$(SDRAM_LEN)ULL" \
	TZ0_SIZE="$(TZ0_SIZE)ULL" \
	ASP_SIZE="$(ASP_SIZE)" \
	TEXT_BASE="$(TEXT_BASE)" \
	TEXT_FOOTPRINT="$(TEXT_FOOTPRINT)"

#<rdar://problem/20487512> maui/elba: increase device memory carveout to 3 framebuffers + 50%
# only for maui and malta	
ifeq ($(SUB_PLATFORM),s8000)
 OPTIONS += DISPLAY_SIZE="((((($(DISPLAY_SIZE)ULL)*150)/100)))"
 OPTIONS += TARGET_DDR_800M=1
 OPTIONS += TARGET_FB_MULT=45
 OPTIONS += LPDP_LINK_CAL_TABLE_VERSION=2
else ifeq ($(SUB_PLATFORM),s8001)
 OPTIONS += DISPLAY_SIZE="($(DISPLAY_SIZE)ULL)"
 OPTIONS += LPDP_LINK_CAL_TABLE_VERSION=3
else ifeq ($(SUB_PLATFORM),s8003)
 OPTIONS += DISPLAY_SIZE="((((($(DISPLAY_SIZE)ULL)*150)/100)))"
 OPTIONS += TARGET_DDR_800M=1
 OPTIONS += TARGET_FB_MULT=45
 OPTIONS += LPDP_LINK_CAL_TABLE_VERSION=3
else
 OPTIONS += DISPLAY_SIZE="($(DISPLAY_SIZE)ULL)"
endif

GLOBAL_LDFLAGS	+= \
	-seg1addr $(TEXT_BASE)

ifeq ($(APPLICATION),SecureROM)
 # XXX reconsider this for future platforms to allow DFU space to increase
 DATA_BASE	:= $(SROM_DATA_BASE)
 GLOBAL_LDFLAGS	+= \
	-segaddr __DATA $(DATA_BASE)
 OPTIONS	+= \
	WITH_ROM_TRAMPOLINE=1 \
	DATA_BASE="$(DATA_BASE)"

 ifeq ($(CONFIGS),fpga)
  OPTIONS		+= \
	SUPPORT_FPGA=1
  endif

else	# ifeq ($(APPLICATION),SecureROM)
 OPTIONS		+= \
	WITH_MONITOR=1
endif

ALL_OBJS	+= \
	$(LOCAL_DIR)/asm.o \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/reconfig.o \
	$(LOCAL_DIR)/trampoline.o
