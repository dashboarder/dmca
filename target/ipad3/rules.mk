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
	DIALOG_D1974=1 \
	TARGET_HIGH_SOC_VOLTAGE=CHIPID_SOC_VOLTAGE_HIGH \
	TARGET_HIGH_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_HIGH \
	TARGET_USE_HSIC=1 \
	AMC_NUM_CHANNELS=4 \
	AMC_NUM_RANKS=1 \
	AMC_CHANNEL_WIDTH=4 \
	DP_DTPATH=\"arm-io/edp\" \
	MULTITOUCH_DTPATH=\"arm-io/spi1/multi-touch\" \
	WIFI_DTPATH=\"arm-io/usb1-complex/usb1-ehci/wlan\" \
	USBDEVICE_DTPATH=\"arm-io/usb0-complex/usb0-device\"

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/pinconfig.o

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
