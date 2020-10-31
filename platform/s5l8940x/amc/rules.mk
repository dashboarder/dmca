# Copyright (C) 2009-2011 Apple Inc. All rights reserved.
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
	WITH_HW_AMC=1

ifneq ($(SUB_PLATFORM), s5l8947x)
ALL_OBJS += \
	$(LOCAL_DIR)/amc_s5l8940x.o
else
ALL_OBJS += \
	$(LOCAL_DIR)/amc_s5l8947x.o
endif
