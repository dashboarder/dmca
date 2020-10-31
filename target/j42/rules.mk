# Copyright (C) 2014 Apple Inc. All rights reserved.
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
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	\
	TARGET_SPARE0_CLK_CFG=0x81000001 \
	TARGET_VID0_CLK_CFG=0x81100000 \
	PLL2_T=1 \
	PLL2_P=2 \
	PLL2_M=99 \
	PLL2_S=15 \
	\
	WITH_HW_HOOVER=1 \
	DISPLAY_APPLE_TV_TUNABLES=1 \
	TARGET_DITHER_TYPE=DITHER_BLUE_NOISE \
	\
	TARGET_CORE_PHY_TMR_LPCLK_CFG=0x00330087 \
	TARGET_CORE_PHY_TMR_CFG=0x256f0000 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	BT_DTPATH=\"arm-io/uart1/bluetooth\" \
	DP_DTPATH=\"arm-io/displayport\" \
	DPPHY_DTPATH=\"arm-io/lpdp-phy\" \
	WIFI_DTPATH=\"arm-io/uart2/wlan\" \
	ETHERNET_DTPATH=\"arm-io/usb-complex/usb-ehci2/lan0\"

ifeq ($(SUB_TARGET),j42d)
OPTIONS += \
	WITH_TARGET_AMC_PARAMS=1
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ifeq ($(PRODUCT),iBoot)
OPTIONS += \
	WITH_HW_DISPLAY_DISPLAYPORT=1
endif

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
