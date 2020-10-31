# Copyright (C) 2010-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS += WITH_HW_AMP=1

# Set ENV_IBOOT for calibration code that is shared with SEG
OPTIONS += ENV_IBOOT

ALL_OBJS += \
	$(LOCAL_DIR)/amp_v3.o \
	$(LOCAL_DIR)/iboot/amp_v3_shim.o \
	$(LOCAL_DIR)/amp_v3_calibration.o

GLOBAL_INCLUDES	+= $(LOCAL_DIR)/include
