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

OPTIONS += \
	WITH_CONSISTENT_DBG=1

MODULES += \
	drivers/apple/consistent_debug

ALL_OBJS += \
	$(LOCAL_DIR)/consistent_debug.o

GLOBAL_INCLUDES += $(LOCAL_DIR)/include
