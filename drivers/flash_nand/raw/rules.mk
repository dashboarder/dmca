# Copyright (c) 2007-2009 Apple Inc. All rights reserved.
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
	WITH_RAW_NAND=1

MODULES += \
	drivers/flash_nand

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/Whimory/Core/FPart \
	$(LOCAL_DIR)/Whimory/Core/VFL \
	$(LOCAL_DIR)/Whimory/Exam

#
# If a NAND FTL has not been configured, we need some of the raw NAND bits.
#
ifeq ($(WITH_NAND_FTL),)
ALL_OBJS += \
	$(LOCAL_DIR)/Whimory/Core/FPart/FPart.o \
	$(LOCAL_DIR)/Whimory/Core/VFL/VFLBuffer.o \
	$(LOCAL_DIR)/Whimory/Exam/WMRExam.o
else
LIBRARY_MODULES += $(LOCAL_DIR)
endif
