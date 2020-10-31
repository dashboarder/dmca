# Copyright (C) 2009-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(AMC_REG_VERSION),)
	OPTIONS += AMC_REG_VERSION=1
else
	OPTIONS += AMC_REG_VERSION=$(AMC_REG_VERSION)
endif	

ifeq ($(AMC_FILE_VERSION),)
  ALL_OBJS += $(LOCAL_DIR)/amc.o
else
  ALL_OBJS += $(LOCAL_DIR)/amc_v2.o
endif

ifeq ($(NO_AMP_CALIBRATION),true)
  ALL_OBJS += $(LOCAL_DIR)/calibration_v1.o
endif

GLOBAL_INCLUDES	+= $(LOCAL_DIR)/include
