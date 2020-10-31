# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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
	DIALOG_D2186=1 \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_V2 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_VMIN \
	TARGET_VID0_CLK_CFG=0x86100000 \
	TARGET_CORE_PHY_TMR_LPCLK_CFG=0x00330087 \
	TARGET_CORE_PHY_TMR_CFG=0x256f0000 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \
	DISPLAY_IPHONE_TUNABLES=1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	TARGET_HS_2_LP_DATA_TIME=0 \
	TARGET_LP_2_HS_DATA_TIME=0 \
	TARGET_HS_2_LP_CLOCK_TIME=0 \
	TARGET_LP_2_HS_CLOCK_TIME=0

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
