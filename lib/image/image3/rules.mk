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

OPTIONS	+= WITH_IMAGE3=1

MODULES += \
	lib/cksum \
	drivers/sha1 \
	lib/image

ALL_OBJS += \
	$(LOCAL_DIR)/Image3.o \
	$(LOCAL_DIR)/image3_wrapper.o
