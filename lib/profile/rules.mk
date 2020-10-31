# Copyright (C) 2012 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

# Don't allow the profiling system to be built in firmware built
# by B&I. The boot profile trashes the panic buffer, which would
# not be a good thing on firmware released to engineering or production
ifneq ($(BUILDING_UNDER_XBS),)
$(error Profiling system must not be included in B&I-built iBoots)
endif

OPTIONS += WITH_PROFILE=1

ALL_OBJS += \
	$(LOCAL_DIR)/profile.o
