# Copyright (C) 2009 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += 

ALL_OBJS += \
        $(LOCAL_DIR)/controller.o \
	$(LOCAL_DIR)/device.o \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/process_edid.o
