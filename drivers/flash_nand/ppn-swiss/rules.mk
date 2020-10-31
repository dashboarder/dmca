# Copyright (c) 2007-2009 Apple Inc. All rights reserved.
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)
NAND_DIR := $(patsubst %/,%,$(dir $(GET_LOCAL_DIR)))

OPTIONS += \
	WITH_PPN=1 \
	WITH_PPN_SYSCFG=1

GLOBAL_INCLUDES += \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/FPart \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/Misc \
	$(NAND_DIR)/ppn/WhimoryPPN/Boot

ALL_OBJS += \
	$(LOCAL_DIR)/ppn_syscfg.o

#
# If a NAND FTL has not been configured, we need some of the raw NAND bits.
#
ifeq ($(WITH_NAND_FTL),)
ALL_OBJS += \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/FPart/PPNFPart.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/Misc/PPNMisc.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Boot/WhimoryBoot.o
else
LIBRARY_MODULES += $(LOCAL_DIR)
endif
