# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS	+= \
	WITH_PKI=1 \
	PKI_CHECK_ANCHOR_BY_SHA1=0 \
	PKI_APPLE_ROOT_CA=1 \
	PKI_CHECK_KEY_IDS=1

LIBRARY_MODULES += \
	lib/pki

GLOBAL_INCLUDES += $(LOCAL_DIR)

ALL_OBJS += \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/chain-validation.o
