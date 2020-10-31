# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

TEST_OBJS := \
	lib/blockdev/blockdev.o \
	lib/cksum/adler32.o \
	lib/env/env.o \
	lib/libc/log2.o \
	lib/nvram/nvram.o

TEST_SUPPORT_OBJS := \
	tests/mocks/sys/security.o \
	tests/mocks/syscfg.o \
	lib/devicetree/devicetree.o 

TEST_CFLAGS := \
	-DCPU_CACHELINE_SIZE=32
