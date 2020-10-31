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

OPTIONS	+= \
	DIALOG_D2255=1 \
	TARGET_CHESTNUT_USE_LOW_RIPPLE_MODE=1 \
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	TARGET_PLL_VCO_CAP=0 \
	TARGET_VCO_RANGE=2 \
	TARGET_LPF_CTRL=8 \
	TARGET_ICP_CTR=15 \
	TARGET_HS_2_LP_DATA_TIME=35 \
	TARGET_LP_2_HS_DATA_TIME=60 \
	TARGET_HS_2_LP_CLOCK_TIME=67 \
	TARGET_LP_2_HS_CLOCK_TIME=159 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \
	DISPLAY_IPHONE_TUNABLES=1 \
	DCS_NUM_RANKS=1 \
	DCS_NUM_CHANNELS=4 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_CPU_RAM=7 \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	BUCK_GPU_RAM=8 \
	WITH_CPU_APSC=1 \
	MULTITOUCH_DTPATH=\"arm-io/spi2/multi-touch\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o
