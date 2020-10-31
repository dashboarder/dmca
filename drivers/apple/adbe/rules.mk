# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR	:=	$(GET_LOCAL_DIR)

OPTIONS		+=	\
	DISPLAYPIPE_BASE_ADDR=ADBE_DISPLAYPIPE_BASE_ADDR \
	WITH_DBE_ADBE=1 \
	ADBE_VERSION=$(ADBE_VERSION)

ALL_OBJS	+=	\
	$(LOCAL_DIR)/adbe.o

ifeq ($(ADBE_VERSION),)
	$(error "ADBE_VERSION not set")
else
	ALL_OBJS	+=	\
		$(LOCAL_DIR)/adbe_v$(ADBE_VERSION).o
endif
	
GLOBAL_INCLUDES	+=	$(LOCAL_DIR)/include
