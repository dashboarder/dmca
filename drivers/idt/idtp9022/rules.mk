# Copyright (C) 2013 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS += \
	WITH_HW_CHARGER_IDTP9022=1 \
	WITH_HW_CHARGER=1

MODULES += \
	lib/power

ALL_OBJS += \
	$(LOCAL_DIR)/idtp9022.o
