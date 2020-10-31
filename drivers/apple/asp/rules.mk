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

GLOBAL_INCLUDES += $(LOCAL_DIR)/include \
				   $(DSTROOT)/usr/local/standalone/firmware \
				   $(SDKROOT)/../../../usr/local/standalone/firmware

ALL_OBJS +=			\
	$(LOCAL_DIR)/asp.o \
    $(LOCAL_DIR)/debug.o \
    $(LOCAL_DIR)/common_util.o

OPTIONS += \
	WITH_HW_ASP=1 \
	WITH_NAND_BOOT=1
