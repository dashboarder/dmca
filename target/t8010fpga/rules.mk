# Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	DISPLAY_FPGA_TUNABLES=1 \
	DCS_RUN_AT_50MHZ=1 \
	DCS_NUM_CHANNELS=4 \
	DCS_NUM_RANKS=1 \
	AMP_CALIBRATION_SKIP=1 \
	DCS_FIXUP_PARAMS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_CPU_RAM=7 \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	BUCK_GPU_RAM=8 \
	WITH_CPU_APSC=1

OPTIONS += \
	SUPPORT_FPGA=1

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o
ALL_OBJS += $(LOCAL_DIR)/pinconfig_$(SUB_TARGET).o
