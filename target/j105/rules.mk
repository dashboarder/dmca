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
	DIALOG_D2255=1

# XXX darrin -- These aren't correct.
# However, they aren't used unless doing clpc,
# which we don't expect to do on this platform
OPTIONS +=\
	BUCK_CPU=0  \
	BUCK_CPU_RAM=7 \
	BUCK_SOC=2  \
	BUCK_GPU=1  \
	BUCK_GPU_RAM=8 \

# XXX darrin -- These haven't been reviewed yet
OPTIONS+= \
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0

# VID0 is 74.25MHz pixel clock from SPARE0 (PLL2) = (24/2•99)/(15+1)
OPTIONS+= \
	TARGET_SPARE0_CLK_CFG=0x82000001 \
	TARGET_VID0_CLK_CFG=0x81100000 \
	PLL2_T=1 \
	PLL2_P=2 \
	PLL2_M=99 \
	PLL2_S=15 \

OPTIONS+= \
	DISPLAY_APPLE_TV_TUNABLES=1 \
	TARGET_DITHER_TYPE=DITHER_BLUE_NOISE \
	WITH_HW_DISPLAY_DISPLAYPORT=1 \
	DP_DTPATH=\"arm-io/displayport0\" \
	DPPHY_DTPATH=\"arm-io/lpdp-phy0\" \

xOPTIONS+= \
	\
	TARGET_CORE_PHY_TMR_LPCLK_CFG=0x00330087 \
	TARGET_CORE_PHY_TMR_CFG=0x256f0000 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \

OPTIONS+= \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \

OPTIONS+= \
	BT_DTPATH=\"arm-io/uart4/bluetooth\" \
	WIFI_DTPATH=\"arm-io/uart3/wlan\" \
	ETHERNET_DTPATH=\"arm-io/apcie/pci-bridge2/lan0\"

OPTIONS += \
	DCS_NUM_RANKS=1 \
	DCS_NUM_CHANNELS=8

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
