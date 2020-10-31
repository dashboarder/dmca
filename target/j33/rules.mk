# Copyright (C) 2009-2010 Apple Inc. All rights reserved.
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
	DIALOG_D1815=1 \
	WITH_SWI_BACKLIGHT=1 \
	TARGET_HIGH_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_MED \
	TARGET_HIGH_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_MED \
	TARGET_EMA_CTL_CPU_SEL=2 \
	TARGET_CPU_SOURCE=2 \
	TARGET_CPU_850M=1 \
	TARGET_DDR_256M=1 \
	TARGET_MIPI_DSI_SOURCE=1 \
	WITH_HW_BS=1 \
	TARGET_MANAGED2H_SOURCE=1 \
	TARGET_MANAGED2L_SOURCE=1 \
	TARGET_MANAGED2H_DIV=2 \
	TARGET_MANAGED2L_DIV=2 \
	TARGET_PCLK1_SOURCE=2 \
	TARGET_PCLK1_DIV=6 \
	TARGET_GFX_SOURCE=2 \
	TARGET_GFX_SLC_SOURCE=2 \
	TARGET_IOP_SOURCE=3 \
	TARGET_LPERFS_SOURCE=3 \
	TARGET_HPERFNRT_SOURCE=0 \
	TARGET_DSIM_DPHYCTL=0x30 \
	TARGET_USE_HSIC=1 \
	TARGET_USB_DEVICE_SELF_POWERED=1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	BT_DTPATH=\"arm-io/uart1/bluetooth\" \
	ETHERNET_DTPATH=\"arm-io/usb-complex/usb-ehci/lan0\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci/wlan\" \
	WITH_TARGET_AMC_PARAMS=1 \
	WITH_TARGET_AMG_PARAMS=1

ifeq ($(PRODUCT),iBoot)
OPTIONS += \
	DP_DTPATH=\"arm-io/displayport\" \
	WITH_HW_DISPLAY_DISPLAYPORT=1
endif
	

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/pinconfig.o
