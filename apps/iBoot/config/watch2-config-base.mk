# Copyright (C) 2013-2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# watch2 iboot bootloader build config
PLATFORM		:=	t8002
SUB_PLATFORM		:=	t8002
TARGET			:=	watch2
ARCH			:=	arm
BOOT_CONFIG		:=	nand
DOCKCHANNELS		:=	uart
ADBE_VERSION		:=	3

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/dcs \
	platform/$(PLATFORM)/pmgr \
	drivers/apple/dcs \
	drivers/apple/a7iop \
	drivers/apple/aes_v2 \
	drivers/apple/aic \
	drivers/apple/ausb \
	drivers/apple/dockchannel \
	drivers/apple/gpio \
	drivers/apple/sep \
	drivers/nxp/cbtl1610 \
	drivers/samsung/uart \
	drivers/synopsys/usbotg \
	platform/$(PLATFORM)/miu

#DISABLED_MODULES 	+= \
	#drivers/apple/reconfig # this can probably go to MODULES_BOOT

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

#ifeq ($(CONFIG_SIM),true)
#MODULES_BASIC		+= \
#	drivers/apple/swifterpmu
#endif

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
ifneq ($(CONFIG_SIM),true)
MODULES_BOOT		+= \
	drivers/display_pmu/beryllium \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/displaypipe \
	drivers/display/summit \
	drivers/samsung/mipi
endif
endif

include $(APPDIR)/products.mk
