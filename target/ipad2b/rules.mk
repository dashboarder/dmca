# Copyright (C) 2010-2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)
TARGET_DIR := $(LOCAL_DIR)

OPTIONS += \
	DIALOG_D1946=1 \
	TARGET_HIGH_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_HIGH \
	TARGET_HIGH_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_HIGH \
	TARGET_CPU_VOLTAGE_MASK=5 \
	TARGET_EMA_CTL_CPU_SEL=2 \
	TARGET_CPU_SOURCE=1 \
	TARGET_MIPI_DSI_SOURCE=1 \
	TARGET_MANAGED2H_SOURCE=1 \
	TARGET_MANAGED2L_SOURCE=2 \
	TARGET_MANAGED2H_DIV=2 \
	TARGET_MANAGED2L_DIV=3 \
	TARGET_PCLK1_SOURCE=1 \
	TARGET_PCLK1_DIV=4 \
	TARGET_GFX_SOURCE=2 \
	TARGET_GFX_SLC_SOURCE=2 \
	TARGET_SPI2_SOURCE=0 \
	TARGET_IOP_SOURCE=3 \
	TARGET_LPERFS_SOURCE=3 \
	TARGET_HPERFNRT_SOURCE=0 \
	TARGET_USE_HSIC=1 \
	TARGET_CLCD_VSPP=0 \
	TARGET_DSIM_CONFIG=0x00800000 \
	TARGET_DSIM_SUPPRESS=0x1 \
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	MULTITOUCH_DTPATH=\"arm-io/spi1/multi-touch\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci/wlan\" \
	BT_DTPATH=\"arm-io/uart1/bluetooth\" \
	WITH_TARGET_USB_CONFIG=1

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
