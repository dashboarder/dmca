# Copyright (C) 2011 Apple Inc. All rights reserved.
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

OPTIONS += \
	DIALOG_D2018=1 \
	TARGET_BOOT_CPU_VOLTAGE=0 \
	TARGET_BOOT_RAM_VOLTAGE=CHIPID_RAM_VOLTAGE_LOW \
	TARGET_BOOT_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_MED \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=5 \
	TARGET_USE_HSIC=1 \
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	AMC_NUM_CHANNELS=4 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	DP_DTPATH=\"arm-io/edp\" \
	MULTITOUCH_DTPATH=\"arm-io/spi3/multi-touch\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci0/wlan\" \
	USBDEVICE_DTPATH=\"arm-io/usb0-complex/usb0-device\" \
        DPTX_PLL_CTL_VALUE=0x14 \
        DPTX_ANALOG_CTL_2_VALUE=0x0d \
        DPTX_ANALOG_CTL_3_VALUE=0x60 \
        DPTX_FINE_PRE_EMPHASIS_VOLTAGE_VALUE=2


GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
