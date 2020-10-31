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
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_V2 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_VMIN \
	TARGET_VID0_CLK_CFG=0x85100000 \
	DISPLAY_FPGA_TUNABLES=1 \
	AMC_NUM_RANKS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	WITH_TARGET_AMC_PARAMS=1 \
	WITH_TARGET_AMP_PARAMS=1

ifeq ($(SUB_PLATFORM),t7000)
OPTIONS	+= AMC_NUM_CHANNELS=2
else
OPTIONS	+= AMC_NUM_CHANNELS=4
OPTIONS += BUCK_RAM=2
endif

OPTIONS += \
	SUPPORT_FPGA=1

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS +=	$(LOCAL_DIR)/init.o \
		$(LOCAL_DIR)/pinconfig_$(SUB_TARGET).o
