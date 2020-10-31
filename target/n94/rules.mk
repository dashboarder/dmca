# Copyright (C) 2010-2011 Apple Inc. All rights reserved.
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
	DIALOG_D1881=1 \
	TARGET_HIGH_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_MED \
	TARGET_HIGH_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_MED \
	TARGET_CPU_SOURCE=2 \
	TARGET_MIPI_DSI_SOURCE=1 \
	TARGET_MANAGED2H_SOURCE=0 \
	TARGET_MANAGED2L_SOURCE=0 \
	TARGET_MANAGED2H_DIV=1 \
	TARGET_MANAGED2L_DIV=2 \
	TARGET_PCLK1_SOURCE=2 \
	TARGET_PCLK1_DIV=6 \
	TARGET_GFX_SOURCE=2 \
	TARGET_GFX_SLC_SOURCE=3 \
	TARGET_IOP_SOURCE=3 \
	TARGET_LPERFS_SOURCE=3 \
	TARGET_HPERFNRT_SOURCE=0 \
	TARGET_USE_HSIC=1 \
	TARGET_DSIM_DPHYCTL=0x30 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	MULTITOUCH_DTPATH=\"arm-io/spi1/multi-touch\" \
	BASEBAND_DTPATH=\"baseband\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci/wlan\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
