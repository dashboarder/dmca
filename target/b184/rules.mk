# Copyright (C) 2012 Apple Inc. All rights reserved.
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
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	TARGET_SPARE0_CLK_CFG=0x83000006 \
	TARGET_VID0_CLK_CFG=0x81000000 \
	TARGET_DDR_798M=1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	WITH_TARGET_AMC_PARAMS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	BUCK_CPU=0  \
	BUCK_GPU=1 \
	WITH_CPU_APSC=1 \
	BT_DTPATH=\"arm-io/uart1/bluetooth\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci0/wlan0\" \
	WITH_TARGET_USB_CONFIG=1 \
	ETHERNET_DTPATH=\"arm-io/usb-complex/usb-ehci0/lan0\"

# TODO: Wifi and BT config are almost certainly wrong here.

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
