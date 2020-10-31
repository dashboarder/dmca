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

# XXX Clean this kludge up when iBoot two-stage boot changes are submitted.
ifneq ($(PRODUCT),iBSS)

OPTIONS += \
	WITH_EFFACEABLE_NAND=1

MODULES += \
	lib/effaceable/iboot

# XXX Clean this up when iBoot two-stage boot changes are submitted.
GLOBAL_INCLUDES += \
	drivers/flash_nand/boot \
	$(LOCAL_DIR)/include

ALL_OBJS += \
	$(LOCAL_DIR)/effaceable_nand.o

endif

# XXX Consider relocating this directory to 'iBoot/drivers/apple/effaceable/nand'.
