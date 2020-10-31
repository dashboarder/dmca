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

TEST_NAME := arm64-mmu

TEST_OBJS := \
	$(LOCAL_DIR)/mmu.o \

TEST_SUPPORT_OBJS := \
	$(LOCAL_DIR)/mmu_test.o

TEST_CFLAGS := \
	-DCPU_CACHELINE_SIZE=64 \
	-DARCH_ARMv8=1 \
	-DTEST=1 \
	-Iarch/arm64/include/ \
	-Iarch/arm/include

TEST_INCLUDES := \
	arch/arm64/include/ \
	arch/arm/include
