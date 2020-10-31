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

#
# Objects that must be built for every target.
# 
# Adding files here is very expensive, as each file
# gets compiled for every target/product/build permutation.
#
#

ifneq ($(APPLICATION),SecureROM)
MODULES += \
	$(LOCAL_DIR)/pmgr
endif

ALL_OBJS += \
	$(LOCAL_DIR)/default_mib.o \
	$(LOCAL_DIR)/platform_info.o \
	$(LOCAL_DIR)/platform_dep.o \
	$(LOCAL_DIR)/target_dep.o

# Include library with platform/target/product-independent functions
LIBRARY_MODULES += $(LOCAL_DIR)

#
# The platform_info file contains symbols with useful information
#
INFO_SYMBOL_FILE	:=	$(call TOBUILDDIR,$(LOCAL_DIR)/platform_info.o)

# for platform_get_security_epoch()
ifeq ($(DEVMAP_EPOCH),)
OPTIONS			+=	PLATFORM_SECURITY_EPOCH=00
else
OPTIONS			+=	PLATFORM_SECURITY_EPOCH=$(DEVMAP_EPOCH)
endif

ifneq ($(DEVMAP_PRODUCT_ID),)
OPTIONS			+=	PLATFORM_PRODUCT_ID=`echo $(DEVMAP_PRODUCT_ID) | sed 's/../0x&,/g' | sed 's/.*/{&}/g'`
endif
