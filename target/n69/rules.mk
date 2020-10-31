# Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
	TARGET_BOOT_SOC_VOLTAGE=3 \
	TARGET_PLL_VCO_CAP=0 \
	TARGET_VCO_RANGE=3 \
	TARGET_LPF_CTRL=8 \
	TARGET_ICP_CTR=6 \
 	TARGET_HS_2_LP_DATA_TIME=35 \
 	TARGET_LP_2_HS_DATA_TIME=54 \
 	TARGET_HS_2_LP_CLOCK_TIME=57 \
 	TARGET_LP_2_HS_CLOCK_TIME=119 \
	TARGET_PHY_STOP_WAIT_TIME=0x28 \
	DISPLAY_IPHONE_TUNABLES=1 \
	DPB_DETECT_VIB_CONTROL_TUNABLE=0xa4830001 \
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
	MULTITOUCH_DTPATH=\"arm-io/spi2/multi-touch\" \
	WIFI_DTPATH=\"arm-io/uart4/wlan\" \
	BT_DTPATH=\"arm-io/uart1/bluetooth\"

# XXX - Needs added for Agile clocking	
# WITH_HW_AGC_MIPI_V2=1 \
# TARGET_AGILE_LINECOUNT=0x9 \
# TARGET_AGILE_SEQ1=0x2010002 \
# TARGET_AGILE_SEQ2=0x6010231 \
# TARGET_AGILE_CTRL=0xC1804 \

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o

ALL_OBJS += $(LOCAL_DIR)/pinconfig.o
