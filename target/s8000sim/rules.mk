# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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

# TBD - BUCK_*

OPTIONS	+= \
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	DCS_NUM_RANKS=1 \
	AMP_CALIBRATION_SKIP=1 \
	BUCK_CPU=0	\
	BUCK_CPU_RAM=7 \
	BUCK_SOC=2  \
	BUCK_GPU=1	\
	BUCK_GPU_RAM=8 \
	WITH_CPU_APSC=1 \
	WIFI_DTPATH=\"arm-io/wlan\"

ifeq ($(SUB_PLATFORM),s8001)
OPTIONS += \
	DCS_NUM_CHANNELS=8 \
	DISPLAY_LANDSCAPE_IPAD_TUNABLES=1
else
OPTIONS += \
	DCS_NUM_CHANNELS=4
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o

ifeq ($(SUB_PLATFORM),s8003)
ALL_OBJS += $(LOCAL_DIR)/pinconfig_s8000sim.o
else
ALL_OBJS += $(LOCAL_DIR)/pinconfig_$(SUB_TARGET).o
endif
