# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

TEST_NAME := fs

include $(LOCAL_DIR)/tests-common.mk

TEST_SUPPORT_OBJS += \
	tests/unittest-main.o \
	$(LOCAL_DIR)/tests.o

HELPER			:=	test-hfs-helper.sh
HELPER_INSTALL		:=	$(call TOTESTDIR,$(HELPER))

$(HELPER_INSTALL):	$(LOCAL_DIR)/$(HELPER)
	$(_v)cp $< $@

EXTRA_TEST_PRODUCTS	+=	$(HELPER_INSTALL)

TEST_RUN_COMMAND	:= ./$(HELPER)
