# Copyright (C) 2007-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(IMAGE_FORMAT),img3)
ALL_OBJS	+= \
	$(LOCAL_DIR)/chipid_get_raw_production_mode.o
endif


# To avoid having a bunch of little tiny files containing default
# functions, we auto-generate the trivial ones from a specification.
#
# XXX The techniques used here could be generalised to provide
# default implementations for other subsystems.

# Default functions that are independent of the build configuration
# are built into a static library shared across builds.
LIBRARY_MODULES		+=	$(LOCAL_DIR)

#
# Library sources
# 
# Adding files here is very expensive, as each file
# gets compiled for every target/product/build permutation.
#
PLATFORM_DEFAULT_SRCS	:= \
	$(LOCAL_DIR)/platform_get_default_gpio_cfg.c \
	$(LOCAL_DIR)/platform_get_memory_region.c \
	$(LOCAL_DIR)/platform_get_spi_frequency.c \
	$(LOCAL_DIR)/platform_memory_map.c \
	$(LOCAL_DIR)/platform_restore_system.c \
	$(LOCAL_DIR)/target_get_boot_battery_capacity.c \
	$(LOCAL_DIR)/target_get_precharge_gg_flag_mask.c \
	$(LOCAL_DIR)/target_get_property.c \
	$(LOCAL_DIR)/target_get_property_base.c

ALL_DEPS		+=	$(PLATFORM_DEFAULT_SRCS:%c=%d)
PLATFORM_DEFAULT_OBJS	:=	$(call TOBUILDDIR,$(PLATFORM_DEFAULT_SRCS:%c=%o))

LOCALLIBS		+=	PLATFORM_DEFAULT

PLATFORM_DEFAULT_BUILD	:=	$(call TOBUILDDIR,$(LOCAL_DIR)/PLATFORM_DEFAULT.a)

$(PLATFORM_DEFAULT_BUILD): $(PLATFORM_DEFAULT_OBJS)
