# Copyright (C) 2013-2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

ifneq ($(filter $(DOCKCHANNELS), uart),)
 OPTIONS += \
		WITH_HW_DOCKCHANNEL_UART=1 \

 ALL_OBJS += \
		$(LOCAL_DIR)/dockchannel_uart.o
endif

ifneq ($(DOCKCHANNELS),)
 ALL_OBJS += \

		GLOBAL_INCLUDES += $(LOCAL_DIR)/include
endif