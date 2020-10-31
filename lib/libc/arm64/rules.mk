# Copyright (C) 2011-2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

LIBC_OBJS += \
	$(LOCAL_DIR)/bcopy.o \
	$(LOCAL_DIR)/bzero.o

ARCH_STROPS := \
	bcopy \
	bzero \
	memset \
	memmove \
	memcpy
