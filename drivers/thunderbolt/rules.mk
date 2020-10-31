# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

# For now thunderbolt implies thunderboot. May not
# always be the case
OPTIONS += WITH_THUNDERBOLT=1 \
	   WITH_THUNDERBOOT=1

ALL_OBJS	+= \
	$(LOCAL_DIR)/tbt_cp_crc.o	\
	$(LOCAL_DIR)/tbt_control_port.o	\
	$(LOCAL_DIR)/tbt_xdomain.o	\
	$(LOCAL_DIR)/thunderboot.o	\
	$(LOCAL_DIR)/uuid.o		\
	$(LOCAL_DIR)/ipipe.o

ifeq ($(APPLICATION),SecureROM)
OPTIONS += WITH_TBT_BOOT=1
endif
ifneq ($(filter $(PRODUCT),LLB,iBSS),)
OPTIONS += WITH_TBT_BOOT=1
endif

# SecureROM and iBSS-DFU support
ifneq ($(filter $(OPTIONS), WITH_DFU_MODE=1), )

OPTIONS += \
	WITH_TBT_DFU=1 \
	WITH_TBT_MODE_DFU=1

endif

# Recovery mode support
ifneq ($(filter $(OPTIONS), WITH_RECOVERY_MODE=1), )

OPTIONS += \
	WITH_TBT_MODE_RECOVERY=1

ifneq ($(BUILD),RELEASE)
OPTIONS += \
	WITH_TBT_UPLOAD=0
# XXX: should be a "1" above
endif

MODULES += \
	lib/cbuf \
	lib/cksum

ALL_OBJS += \
	$(LOCAL_DIR)/thunderboot_debug.o

endif
