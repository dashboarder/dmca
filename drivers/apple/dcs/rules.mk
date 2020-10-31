# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

# Set ENV_IBOOT for calibration code that is shared with SEG
OPTIONS += ENV_IBOOT

ALL_OBJS += \
	$(LOCAL_DIR)/dcs.o 	\
	$(LOCAL_DIR)/dcs_init_lib.o

DCS_CAL_LIB_V1_PLATFORMS := s8000 s8003

ifneq ($(filter $(SUB_PLATFORM),$(DCS_CAL_LIB_V1_PLATFORMS)),)
  ALL_OBJS += $(LOCAL_DIR)/dcs_calibration.o
else
  ALL_OBJS += $(LOCAL_DIR)/dcs_calibration_v2.o
endif

GLOBAL_INCLUDES	+= $(LOCAL_DIR)/include
