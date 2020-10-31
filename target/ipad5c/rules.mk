# Copyright (C) 2013 Apple Inc. All rights reserved.
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
	DIALOG_D2207=1 \
	DIALOG_D2231=1 \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_V2 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_VMIN \
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	TARGET_VID0_CLK_CFG=0x89100000 \
	TARGET_CORE_PHY_TMR_LPCLK_CFG=0x00330087 \
	TARGET_CORE_PHY_TMR_CFG=0x256f0000 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \
	TARGET_DDR_798M=1 \
	DISPLAY_LANDSCAPE_IPAD_TUNABLES=1 \
	TARGET_DITHER_TYPE=DITHER_BLUE_NOISE \
	PMU_LCD_PWR_EN=1 \
	AMC_NUM_CHANNELS=4 \
	AMC_NUM_RANKS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_GPU=2  \
	BUCK_RAM=1  \
	MULTITOUCH_DTPATH=\"arm-io/spi1/multi-touch\" \
	WIFI_DTPATH=\"arm-io/uart2/wlan\" \
	DP_DTPATH=\"arm-io/displayport\" \
	DPPHY_DTPATH=\"arm-io/lpdp-phy\" \
	BT_DTPATH=\"arm-io/uart1/bluetooth\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o \
	    $(LOCAL_DIR)/pinconfig_proto2.o \
	    $(LOCAL_DIR)/pinconfig_proto3.o \
	    $(LOCAL_DIR)/pinconfig_evt.o \
	    $(LOCAL_DIR)/pinconfig_evt2.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
