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

TEST_NAME := cksum

TEST_OBJS := \
	$(LOCAL_DIR)/adler32.o \
	$(LOCAL_DIR)/crc.o \
	$(LOCAL_DIR)/crc32.o \
	$(LOCAL_DIR)/siphash.o

TEST_SUPPORT_OBJS := \
	tests/unittest-main.o \
	$(LOCAL_DIR)/tests.o

ifeq ($(ARM_ARCH),armv7)
TEST_OBJS += \
	$(LOCAL_DIR)/arm/adler32vec.o
endif

ifeq ($(ARM_ARCH),armv7k)
TEST_OBJS += \
	$(LOCAL_DIR)/arm/adler32vec.o
endif

ifeq ($(ARM_ARCH),arm64)
TEST_OBJS += \
	$(LOCAL_DIR)/arm64/adler32vec.o
endif

