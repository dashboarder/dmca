# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS += \
	WITH_HW_CLOCKS=1 \
	WITH_HW_PLATFORM_POWER=1 \
	WITH_HW_POWER_GATING=0

ALL_OBJS += $(LOCAL_DIR)/pmgr.o
