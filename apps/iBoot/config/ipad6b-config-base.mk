# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# iPad6,[3,4] iBoot bootloader build config

# Switch to s8001 once support for Elba is in
#
PLATFORM		:=	s8000
SUB_PLATFORM		?=	s8001
ARCH			:=	arm64
BOOT_CONFIG		:=	nand
HW_TIMER		:=	architected
ADBE_VERSION		:=	2
LPDP_VERSION		:=	2

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/dcs \
	platform/$(PLATFORM)/error_handler \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	drivers/apple/a7iop \
	drivers/apple/aes_v2 \
	drivers/apple/aic \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/apple/dcs \
	drivers/apple/dwi \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sep \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	drivers/dialog/charger \
	drivers/dialog/pmu \
	drivers/nxp/cbtl1610 \
	drivers/power/hdqgauge \
	drivers/apple/voltage_knobs

ifneq ($(BUILD),RELEASE)
MODULES_BASIC		+= \
	drivers/apple/consistent_debug
endif

ifeq ($(BOOT_CONFIG), nand)
MODULES_FIRMWARE	+= \
	platform/$(PLATFORM)/apcie \
	drivers/pci \
	drivers/apple/apcie \
	drivers/apple/dart_lpae \
	drivers/nvme
endif

ifeq ($(BOOT_CONFIG), nor)
MODULES_BASIC		+= \
	drivers/samsung/spi

MODULES_FILESYSTEM	+= \
	drivers/flash_nor/spi
endif

LIBRARY_MODULES		+= \
	lib/libcorecrypto

MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/dither \
	drivers/apple/dpb_v2 \
	drivers/apple/wpc \
	drivers/apple/prc \
	drivers/apple/displaypipe \
	drivers/apple/reconfig \
	drivers/displayport \
	drivers/apple/lpdp_phy \
	drivers/analogix/displayport \
	drivers/display/edp

# Disabling dpb for rdar://problem/20014397
#	drivers/apple/dpb

# Override default ASP size.
ASP_SIZE		:=	18*1024*1024

# override default platform display region size config
DISPLAY_SIZE		:=	64*1024*1024

include $(APPDIR)/products.mk

# You can override the default SRAM/SDRAM configuration here but you
# probably shouldn't.

# POR for J127 and J128 is 2GB only. Hence limiting it since the default
# for Elba is 4GB.
SDRAM_LEN		:= 2*1024*1024*1024

ifeq ($(TEXT_BANK),sdram)
TEXT_BASE		:= 0x870000000
TEXT_FOOTPRINT		:= 1024*1024
endif
