# Copyright (C) 2007-2008 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
	WITH_HW_PKE=1

MODULES += \
	lib/pki

ALL_OBJS += \
	$(LOCAL_DIR)/pke.o \
	$(LOCAL_DIR)/AppleS5L8900XPKE-hardware.o
