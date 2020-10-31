# Copyright (C) 2010 Apple Inc. All rights reserved.
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

OPTIONS	+= \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_MED \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	TARGET_VID0_CLK_CFG=0x86100000 \
	DISPLAY_IPHONE_TUNABLES=1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	AMP_CALIBRATION_SKIP=1 \
	BUCK_CPU=0	\
	BUCK_GPU=1	\
	WITH_TARGET_AMC_PARAMS=1 \
	WITH_TARGET_AMP_PARAMS=1 \
	WIFI_DTPATH=\"arm-io/wlan\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
