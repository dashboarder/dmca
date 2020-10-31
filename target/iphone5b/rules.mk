# Copyright (C) 2011-2012 Apple Inc. All rights reserved.
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
	DIALOG_D1972=1 \
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=CHIPID_RAM_VOLTAGE_LOW \
	TARGET_BOOT_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_MED \
	TARGET_RAM_VOLTAGE_OFFSET=17 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=5 \
	TARGET_USE_HSIC=1 \
	TARGET_DSIM_CONFIG=0x00400000 \
	TARGET_DSIM_DPHYCTL=0x30 \
	TARGET_DSIM_UP_CODE=0x5 \
	TARGET_DSIM_DOWN_CODE=0x1 \
	TARGET_DSIM_SUPPRESS=0x1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	MULTITOUCH_DTPATH=\"arm-io/spi1/multi-touch\" \
	BASEBAND_DTPATH=\"baseband\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci/wlan\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
