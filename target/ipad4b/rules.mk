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
	TARGET_HAS_BASEBAND=$(TARGET_HAS_BASEBAND) \
	TARGET_SPARE0_CLK_CFG=0x83000006 \
	TARGET_VID0_CLK_CFG=0x81000000 \
	DISPLAY_LANDSCAPE_IPAD_TUNABLES=1 \
	TARGET_ENABLE_CM=1 \
	TARGET_HPD_TO_BL_TIMEOUT=92000 \
	TARGET_DDR_798M=1 \
	AMC_NUM_CHANNELS=2 \
	AMC_NUM_RANKS=1 \
	TARGET_DWI_FREQUENCY=24000000 \
	TARGET_DWI_TRANSFER_GAP_US=2.5 \
	TARGET_ADBE0_VBLANK_POSITION=0xF0000 \
	BUCK_CPU=0  \
	BUCK_GPU=1 \
	WITH_CPU_APSC=1 \
	BT_DTPATH=\"arm-io/uart1/bluetooth\" \
	DP_DTPATH=\"arm-io/displayport\" \
	MULTITOUCH_DTPATH=\"arm-io/spi1/multi-touch\" \
	WIFI_DTPATH=\"arm-io/usb-complex/usb-ehci1/wlan\" \
	WITH_TARGET_USB_CONFIG=1

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/$(PINCONFIG_OBJ)

ifneq ($(PRODUCT),LLB)
ALL_OBJS += \
	$(LOCAL_DIR)/properties.o
endif
