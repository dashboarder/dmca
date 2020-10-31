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

OPTIONS	+= \
	DIALOG_D2045=1 \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_MED \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	TARGET_VID0_CLK_CFG=0x86100000 \
	DISPLAY_IPHONE_TUNABLES=1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	TARGET_DSIM_CONFIG=0x00400000 \
	TARGET_DSIM_DPHYCTL=0x30 \
	TARGET_DSIM_UP_CODE=0x1 \
	TARGET_DSIM_DOWN_CODE=0x1 \
	TARGET_DSIM_SUPPRESS=0x1 \
	BUCK_CPU=0  \
	BUCK_GPU=1 \
	WITH_CPU_APSC=1

# this board may have no PMU installed, so AMP should not panic because of that
OPTIONS	+= \
	AMP_NO_PMU_PANIC=1

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
