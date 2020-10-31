#
# Copyright (c) 2011 Apple Inc. All rights reserved.
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
	$(LOCAL_DIR)/include

ALL_OBJS += \
	$(LOCAL_DIR)/effaceable_storage_core.o \
	$(LOCAL_DIR)/effaceable_nand_core.o \
	$(LOCAL_DIR)/effaceable_nor_core.o
