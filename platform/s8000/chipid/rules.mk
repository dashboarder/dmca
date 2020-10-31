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

OPTIONS += \
	WITH_HW_PLATFORM_CHIPID=1

ALL_OBJS += \
	$(LOCAL_DIR)/chipid.o \
	$(LOCAL_DIR)/operating_point.o \
	$(LOCAL_DIR)/dvfmperf.o \
	$(LOCAL_DIR)/tunables_pmgr_product.o

ifeq ($(SUB_PLATFORM),s8000)
ALL_OBJS += \
	$(LOCAL_DIR)/tunables_pmgr_s8000.o \
	$(LOCAL_DIR)/pmgr_binning_s8000.o
endif

ifeq ($(SUB_PLATFORM),s8001)
ALL_OBJS += \
	$(LOCAL_DIR)/tunables_pmgr_s8001.o \
	$(LOCAL_DIR)/pmgr_binning_s8001.o

	ifeq ($(TARGET),ipad6b)
	ALL_OBJS += \
		$(LOCAL_DIR)/tunables_pmgr_s8001_ipad6b.o
	endif
endif

ifeq ($(SUB_PLATFORM),s8003)
ALL_OBJS += \
	$(LOCAL_DIR)/tunables_pmgr_s8003.o \
	$(LOCAL_DIR)/pmgr_binning_s8003.o
endif
