# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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
	WITH_HW_DCS=1

ALL_OBJS += \
	$(LOCAL_DIR)/dcs.o 			\
	$(LOCAL_DIR)/dcs_init_lib_t8002.o
