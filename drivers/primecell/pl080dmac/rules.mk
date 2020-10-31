# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES	+=	$(LOCAL_DIR)/include

OPTIONS += \
	WITH_HW_DMA=1

ALL_OBJS += \
	$(LOCAL_DIR)/pl080dmac.o
