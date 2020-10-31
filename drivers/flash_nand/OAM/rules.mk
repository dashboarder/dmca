# Copyright (c) 2008-2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/$(APPLICATION)

ALL_OBJS += \
	$(LOCAL_DIR)/$(APPLICATION)/WMROAM.o \
	$(LOCAL_DIR)/$(APPLICATION)/WMRBuf.o

MODULES += \
	lib/heap
