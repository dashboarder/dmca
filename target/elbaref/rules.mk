# Copyright (C) 2015 Apple Inc. All rights reserved.
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
	DIALOG_D2257=1 \
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	TARGET_PLL_VCO_CAP=0 \
	TARGET_VCO_RANGE=2 \
	TARGET_LPF_CTRL=8 \
	TARGET_ICP_CTR=15 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \
	DISPLAY_LANDSCAPE_IPAD_TUNABLES=1 \
	PMU_LCD_PWR_EN=1 \
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	TARGET_CORE_PHY_TMR_LPCLK_CFG=0x00330087 \
	TARGET_CORE_PHY_TMR_CFG=0x256f0000 \
	DCS_NUM_RANKS=1 \
	DCS_NUM_CHANNELS=8 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_CPU_RAM=7 \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	BUCK_GPU_RAM=8 \
	WITH_CPU_APSC=1 \
	MULTITOUCH_DTPATH=\"arm-io/spi3/multi-touch\" \
	WIFI_DTPATH=\"arm-io/uart2/wlan\" \
	DP_DTPATH=\"arm-io/displayport0\" \
	DPPHY_DTPATH=\"arm-io/lpdp-phy0\" \
	BT_DTPATH=\"arm-io/uart3/bluetooth\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o $(LOCAL_DIR)/pinconfig.o
