# Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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

# BUCK_CPU, BUCK_GPU and BUCK_RAM should be reviewed
# if any changed happen in drivers/dialog/pmu/power.c:power_get_buck_value()

OPTIONS	+= \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_V2 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_VMIN \
	TARGET_VID0_CLK_CFG=0x85100000 \
	AMC_NUM_RANKS=1 \
	AMP_CALIBRATION_SKIP=1 \
	BUCK_CPU=0	\
	BUCK_SOC=2  \
	BUCK_GPU=1	\
	WITH_TARGET_AMC_PARAMS=1 \
	WITH_TARGET_AMP_PARAMS=1 \
	WIFI_DTPATH=\"arm-io/wlan\"

ifeq ($(SUB_PLATFORM),t7000)
OPTIONS	+= AMC_NUM_CHANNELS=2
OPTIONS	+= DISPLAY_IPHONE_TUNABLES=1
OPTIONS	+= DISPLAY_D403_TUNABLES=1
else
OPTIONS	+= AMC_NUM_CHANNELS=4
OPTIONS	+= BUCK_RAM=1
OPTIONS	+= DISPLAY_LANDSCAPE_IPAD_TUNABLES=1
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig_$(SUB_TARGET).o
