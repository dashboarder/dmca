# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# t7000fpga iboot bootloader build config
PLATFORM		:=	t7000
SUB_PLATFORM		?=	t7000
ARCH			:=	arm64
BOOT_CONFIG		:=	nand
HW_TIMER		:=	architected
AMC_REG_VERSION		?=	4
AMC_FILE_VERSION	?=	2
ADBE_VERSION		:=	2
SUPPORT_FPGA		:=	1

# code modules
MODULES_BASIC		+= \
	platform/$(PLATFORM) \
	platform/$(PLATFORM)/amc \
	platform/$(PLATFORM)/chipid \
	platform/$(PLATFORM)/miu \
	platform/$(PLATFORM)/pmgr \
	platform/$(PLATFORM)/error_handler \
	drivers/apple/a7iop \
	drivers/apple/aes \
	drivers/apple/aic \
	drivers/apple/amc \
	drivers/apple/amp_v3 \
	drivers/apple/ausb \
	drivers/apple/ccc \
	drivers/samsung/dwi \
	drivers/apple/gpio \
	drivers/apple/iic \
	drivers/apple/sep \
	drivers/samsung/uart \
	drivers/synopsys/usbotg

ifeq ($(BOOT_CONFIG), nand)
MODULES_FILESYSTEM	+= \
	drivers/apple/asp \
	drivers/apple/csi \

MODULES_FIRMWARE	+= \
	platform/$(PLATFORM)/apcie \
	drivers/apple/anc \
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

# Override platform default memory map
TZ0_SIZE		:=	12*1024*1024

include $(APPDIR)/products.mk

