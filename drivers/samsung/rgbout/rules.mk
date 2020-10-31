# Copyright (C) 2012 Apple Inc. All rights reserved.
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
	DISPLAYPIPE_BASE_ADDR=RGBOUT_DISPLAYPIPE_BASE_ADDR \
	WITH_DBE_RGBOUT=1

ALL_OBJS	+=	\
	$(LOCAL_DIR)/rgbout.o
	
GLOBAL_INCLUDES	+=	$(LOCAL_DIR)/include
