# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

TEST_NAME := fs-fuzz

include $(LOCAL_DIR)/tests-common.mk

TEST_SUPPORT_OBJS += \
	tests/fuzz-main.o \
	$(LOCAL_DIR)/fuzz.o

TEST_CFLAGS += \
	-DNO_MOCK_ASSERTS=1

TEST_BUILD_ONLY := YES
