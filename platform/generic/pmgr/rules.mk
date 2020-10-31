# Copyright (C) 2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

# All H8+ chip must use pmgr_binning
PMGR_BINNING=0

ifeq ($(SUB_PLATFORM),s8000)
PMGR_BINNING=1
endif

ifeq ($(SUB_PLATFORM),s8001)
PMGR_BINNING=1
endif

ifeq ($(SUB_PLATFORM),s8003)
PMGR_BINNING=1
endif

ifeq ($(PMGR_BINNING),1)
ALL_OBJS += \
	$(LOCAL_DIR)/pmgr_binning.o

ifneq ($(BUILD),RELEASE)
ifeq ($(PRODUCT),iBoot)
ALL_OBJS +=                     \
    $(LOCAL_DIR)/pmgr_binning_menu.o
endif
ifeq ($(PRODUCT),iBEC)
ALL_OBJS +=                     \
    $(LOCAL_DIR)/pmgr_binning_menu.o
endif
endif
endif

