# Copyright (C) 2012, 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

IMAGE_FORMAT := 4

# Hack to link against SDKROOT or PLATFORMROOT until somebody figures
# out why libImg4Decode is bouncing between locations.
IMG4_IN_SDKROOT		:= $(realpath $(SDKROOT)/usr/local/lib/libImg4Decode_os.a)
IMG4_IN_PLATFORMROOT	:= $(realpath $(PLATFORMROOT)/usr/local/lib/libImg4Decode_os.a)
ifneq ($(IMG4_IN_SDKROOT),)
EXTERNAL_STATICLIBS	+= $(IMG4_IN_SDKROOT)
EXTERNAL_INCLUDES	+= $(SDKROOT)/usr/local/include/
else
 ifneq ($(IMG4_IN_PLATFORMROOT),)
EXTERNAL_STATICLIBS	+= $(IMG4_IN_PLATFORMROOT)
EXTERNAL_INCLUDES	+= $(PLATFORMROOT)/usr/local/include/
 else
$(error libImg4Decode.a not found in SDKROOT or PLATFORMROOT)
 endif
endif

OPTIONS	+= WITH_IMAGE4=1

MODULES += \
	drivers/sha1 \
	lib/cksum \
	lib/image \
	lib/pki

ALL_OBJS += \
	$(LOCAL_DIR)/image4_partial.o \
	$(LOCAL_DIR)/image4_wrapper.o
