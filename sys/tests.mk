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

TEST_NAME := iboot-sys

TEST_OBJS := \
	$(LOCAL_DIR)/hash.o

TEST_SUPPORT_OBJS := \
	tests/unittest-main.o \
	$(LOCAL_DIR)/tests-sha384.o \
	$(LOCAL_DIR)/tests-list.o

HOST_SDKROOT_PATH := $(shell xcodebuild -version -sdk macosx.internal Path)

TEST_CFLAGS := \
	-DWITH_SHA2_384=1 \
	-I$(HOST_SDKROOT_PATH)/usr/local/include

