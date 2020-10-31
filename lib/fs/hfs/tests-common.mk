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

TEST_OBJS := \
	lib/libc/log2.o \
	lib/blockdev/blockdev.o \
	lib/blockdev/mem_blockdev.o \
	lib/fs/fs.o \
	lib/fs/debug.o \
	lib/fs/hfs/cache.o \
	lib/fs/hfs/hfs.o \
	lib/fs/hfs/hfs_fs.o

TEST_SUPPORT_OBJS := \
	lib/env/env.o \
	tests/mocks/blockdev.o \
	tests/mocks/sys/security.o

TEST_CFLAGS := \
	-DCPU_CACHELINE_SIZE=32 \
	-DHFS_UNITTEST=1
