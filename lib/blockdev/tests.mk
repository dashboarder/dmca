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

TEST_NAME := blockdev

TEST_OBJS := \
	lib/libc/log2.o \
	$(LOCAL_DIR)/blockdev.o \
	$(LOCAL_DIR)/mem_blockdev.o \
	$(LOCAL_DIR)/subdev.o

TEST_SUPPORT_OBJS := \
	tests/unittest-main.o \
	$(LOCAL_DIR)/tests.o

TEST_CFLAGS := \
	-DCPU_CACHELINE_SIZE=32 \
	-DHOST_TEST=1
