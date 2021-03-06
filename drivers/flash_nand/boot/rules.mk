#
# Copyright (c) 2008-2011 Apple Inc. All rights reserved.
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
	WITH_NAND_BOOT=1

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULES += \
	lib/effaceable/iboot/nand

ALL_OBJS += \
	$(LOCAL_DIR)/nand_export.o \
	$(LOCAL_DIR)/nand_part_core.o \
	$(LOCAL_DIR)/nand_boot.o

ifeq ($(PRODUCT),LLB)
OPTIONS += \
	WITH_NAND_PART_BUILD_TINY=1
endif