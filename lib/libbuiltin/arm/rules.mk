# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

LIBBUILTIN_OBJS += \
	$(LOCAL_DIR)/builtin_divide.o

ARCH_BUILTINOPS := \
	builtin_divide
