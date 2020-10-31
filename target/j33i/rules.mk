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
	TARGET_CPU_SOURCE=2 \
	TARGET_CPU_850M=1 \
	TARGET_DDR_533M=1 \
	TARGET_MIPI_DSI_SOURCE=0 \
	WITH_HW_BS=1 \
	TARGET_MANAGED2H_SOURCE=1 \
	TARGET_MANAGED2L_SOURCE=1 \
	TARGET_MANAGED2H_DIV=2 \
	TARGET_MANAGED2L_DIV=2 \
	TARGET_PCLK1_SOURCE=2 \
	TARGET_PCLK1_DIV=10 \
	TARGET_GFX_SOURCE=3 \
	TARGET_GFX_SLC_SOURCE=3 \
	TARGET_IOP_SOURCE=0 \
	TARGET_LPERFS_SOURCE=0 \
	TARGET_HPERFNRT_SOURCE=2 \
	TARGET_USE_PREDIV4=1 \
	TARGET_PREDIV4_DIV=2 \
	TARGET_PREDIV4_SOURCE=3 \
	TARGET_USE_HSIC=1 \
	TARGET_USB_DEVICE_SELF_POWERED=1 \
	AMC_NUM_CHANNELS=1 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	BT_DTPATH=\"arm-io/uart2/bluetooth\" \
	ETHERNET_DTPATH=\"arm-io/ethernet\" \
	WIFI_DTPATH=\"arm-io/usb0-complex/usb0-ehci/wlan\"

ifeq ($(PRODUCT),iBoot)
OPTIONS += \
	WITH_HW_DISPLAY_HDMI=1
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
