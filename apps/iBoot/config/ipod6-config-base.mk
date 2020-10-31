# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# ipod6 iboot bootloader build config
PLATFORM		:=	s7002
SUB_PLATFORM		:=	s7002
TARGET			:=	ipod6
ARCH			:=	arm
HW_TIMER		:=	pmgr
BOOT_CONFIG		:=	nand
AMC_REG_VERSION		:=	2
AMP_FILE_VERSION	:=	2
#DOCKFIFOS		:=	uart bulk
DOCKFIFOS		:=	uart
ADBE_VERSION		:=	3
IBOOT_ANC_NVRAM		:=	true

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	platform/$(PLATFORM)/reconfig \
	lib/env \
	lib/blockdev \
	lib/nvram \
	drivers/apple/a7iop \
	drivers/apple/aes_s7002 \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp \
	drivers/apple/ausb \
	drivers/apple/dockfifo \
	drivers/apple/gpio \
	drivers/nxp/cbtl1610 \
	drivers/primecell/pl080dmac \
	drivers/samsung/uart \
	drivers/synopsys/usbotg

ifneq ($(BUILD),RELEASE)
MODULES_BASIC		+= \
	drivers/apple/consistent_debug
endif

ifneq ($(CONFIG_FPGA),true) 
ifneq ($(CONFIG_SIM),true)
MODULES_BASIC		+= \
	drivers/power/hdqgauge \
	drivers/idt/idtp9022 \
	drivers/dialog/pmu \
	drivers/apple/voltage_knobs
endif
endif

ifneq ($(CONFIG_SIM), true)
MODULES_BASIC		+= \
	drivers/apple/iic
endif

ifeq ($(BOOT_CONFIG), nand)
MODULES_FILESYSTEM	+= \
	drivers/apple/asp \
	drivers/apple/csi \

MODULES_FIRMWARE	+= \
	drivers/apple/anc
endif

ifeq ($(BOOT_CONFIG), nor)

MODULES_BASIC		+= \
	drivers/samsung/spi

MODULES_FILESYSTEM	+= \
	drivers/flash_nor/spi
endif

LIBRARY_MODULES		+= \
	lib/libcorecrypto

ifneq ($(CONFIG_FPGA),true) 
MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/displaypipe \

ifneq ($(CONFIG_SIM),true)
MODULES_BOOT		+= \
	drivers/display_pmu/beryllium \
	drivers/display/summit \
	drivers/samsung/mipi
endif
endif

include $(APPDIR)/products.mk

# You can override the default SRAM/SDRAM configuration here but probably shouldn't.

