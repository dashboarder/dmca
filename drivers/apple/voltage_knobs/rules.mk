# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ifneq ($(BUILD),RELEASE)
ALL_OBJS +=			\
    $(LOCAL_DIR)/knobs.o	\
    $(LOCAL_DIR)/menu_commands.o

ifeq ($(PLATFORM),t7000)
ALL_OBJS += $(LOCAL_DIR)/knobs_t7000.o
else ifeq ($(PLATFORM),s7002)
ALL_OBJS += $(LOCAL_DIR)/knobs_s7002.o
else ifeq ($(PLATFORM),s8000)
ALL_OBJS += $(LOCAL_DIR)/knobs_s8000.o
else
$(error unknown platform)
endif

OPTIONS += \
    WITH_VOLTAGE_KNOBS=1
endif

