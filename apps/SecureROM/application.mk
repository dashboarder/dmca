# Copyright (C) 2007-2010, 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

#
# Global headers that are independing of product, target, etc.
#
GLOBAL_HEADERS += \
	lib/mib/mib_nodes.h

OPTIONS += \
	READ_ONLY=1

ifeq ($(CONFIG),si)
TEXT_AREA ?= ROM
endif

ifeq ($(CONFIG),fpga)
TEXT_AREA ?= ROM
OPTIONS += \
	SUPPORT_FPGA=1
endif

ifeq ($(BUILD),ROMRELEASE)
OPTIONS += \
	DEBUG_LEVEL=DEBUG_SILENT \
	NO_PANIC_STRINGS=1
endif

OPTIONS	+= \
	IMAGE_MAX_COUNT=1

ALL_OBJS += \
	$(LOCAL_DIR)/main.o

MODULES	+= \
	lib/heap \
	lib/random \
	platform/defaults \
	platform/generic \
	sys

ifeq ($(IMAGE_FORMAT),)
$(error "IMAGE_FORMAT not set")
endif
ifeq ($(IMAGE_FORMAT),im4p)
MODULES += \
        lib/image/image4
OPTIONS += \
        WITH_UNTRUSTED_EXECUTION_ALLOWED=1
else
$(error "IMAGE_FORMAT not supported")
endif


LIBRARY_MODULES += \
	lib/libc \
	lib/mib

OPTIONS += \
        WITH_DFU_MODE=1
