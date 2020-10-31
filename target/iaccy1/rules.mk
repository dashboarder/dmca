# Copyright (C) 2007 Apple Inc. All rights reserved.
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
	TARGET_DREX_CLK_RATIO=2 \
	TARGET_DREX_TIMING_AREF=0x0000005D \
	TARGET_DREX_TIMING_ROW=0x34488611 \
	TARGET_DREX_TIMING_DATA=0x36330306 \
	TARGET_DREX_TIMING_POWER=0x50380335 \
	TARGET_DREX_QOS_TIDEMARK=9 \
	TARGET_DREX_QOS_ACCESS=12 \
	TARGET_USB_DEVICE_SELF_POWERED=1 \
	WITH_BRIEF_BOOT_BANNER=1

ifeq ($(APPLICATION),iBoot)
OPTIONS += \
	   DISABLE_CLEAR_MEMORY=1
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o			\
	    $(LOCAL_DIR)/display_get_info.o
