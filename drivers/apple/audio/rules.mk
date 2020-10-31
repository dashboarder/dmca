# Copyright (C) 2009-2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR	:=	$(GET_LOCAL_DIR)

ifeq ($(PLATFORM_VARIANT),Audio)
ALL_OBJS	+=	${LOCAL_DIR}/audio.o
OPTIONS += \
	WITH_HW_TIMER=1
else
ALL_OBJS	+=	${LOCAL_DIR}/debug.o
endif

GLOBAL_INCLUDES	+=	${LOCAL_DIR}/include
