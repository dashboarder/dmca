# Copyright (C) 2009-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR	:=	$(GET_LOCAL_DIR)

OPTIONS += \
	WITH_HW_AIC=1

ifneq ($(PLATFORM_VARIANT),Audio)
OPTIONS += \
	WITH_AIC_INTERRUPTS=1
endif

ifeq ($(HW_TIMER),)

OPTIONS += \
	WITH_HW_TIMER=1

ifneq ($(PLATFORM_VARIANT),Audio)
OPTIONS += \
	WITH_AIC_TIMERS=1
endif

endif

ALL_OBJS	+=	$(LOCAL_DIR)/aic.o

GLOBAL_INCLUDES	+=	$(LOCAL_DIR)/include
