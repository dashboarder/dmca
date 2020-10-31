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

TEST_NAME := syscfg

TEST_OBJS := \
	$(LOCAL_DIR)/syscfg.o \
	lib/blockdev/blockdev.o \
	lib/blockdev/mem_blockdev.o \
	lib/libc/log2.o

TEST_SUPPORT_OBJS := \
	tests/unittest-main.o \
	$(LOCAL_DIR)/tests.o

TEST_CFLAGS := \
	-DCPU_CACHELINE_SIZE=32
