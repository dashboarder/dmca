# Copyright (C) 2011-2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(PRODUCT),LLB)
OPTIONS += WITH_ANC_FIRMWARE=1
ALL_OBJS +=			\
	$(LOCAL_DIR)/anc_bootrom.o \
    $(LOCAL_DIR)/anc_llb.o     \
    $(LOCAL_DIR)/util_boot.o
endif

ifeq ($(IBOOT_ANC_NVRAM), true)
ifeq ($(PRODUCT),iBoot)

# Some targets like M7 can boot into chargetrap faster
# by skipping ANS in iBoot and instead relying on ANC
# for firmware and NVRAM access. These targets set
# IBOOT_ANC_NVRAM to true and ANC support is only conditionally
# active in iBoot.

OPTIONS += WITH_ANC_FIRMWARE=1
ALL_OBJS +=			\
	$(LOCAL_DIR)/anc_bootrom.o \
    $(LOCAL_DIR)/anc_llb.o     \
    $(LOCAL_DIR)/util_boot.o
endif
endif


ifeq ($(APPLICATION),SecureROM)
OPTIONS += WITH_ANC_BOOT=1
ALL_OBJS +=			\
	$(LOCAL_DIR)/anc_bootrom.o \
    $(LOCAL_DIR)/util_boot.o
endif

