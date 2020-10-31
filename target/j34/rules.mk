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
	DIALOG_D2089=1 \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_MED \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	TARGET_SPARE0_CLK_CFG=0x81000001 \
	TARGET_VID0_CLK_CFG=0x81000000 \
	WITH_HW_HOOVER=1 \
	DISPLAY_APPLE_TV_TUNABLES=1 \
	TARGET_DITHER_TYPE=DITHER_NONE \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	WITH_TARGET_AMC_PARAMS=1 \
	TARGET_USE_HSIC=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	TARGET_USB_DEVICE_SELF_POWERED=1 \
	TARGET_ADBE0_VBLANK_POSITION=0xF0000 \
	BUCK_CPU=0  \
	BUCK_GPU=1 \
	WITH_CPU_APSC=1 \
	BT_DTPATH=\"arm-io/uart4/bluetooth\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci0/wlan\" \
	PINTO_DTPATH=\"arm-io/usb-complex/usb-ehci0/lan1\" \
	ETHERNET_DTPATH=\"arm-io/usb-complex/usb-ehci1/lan0\" \
	WITH_TARGET_USB_CONFIG=1

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ifeq ($(PRODUCT),iBoot)
OPTIONS += \
	DP_DTPATH=\"arm-io/displayport\" \
	WITH_HW_DISPLAY_DISPLAYPORT=1
endif

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
