# Copyright (C) 2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

WHIMORYPPN_DIR		:=	$(GET_LOCAL_DIR)
WHIMORYPPN_BUILD	:=	$(call TOLIBDIR,$(WHIMORYPPN_DIR)/WHIMORYPPN.a)
COMMONLIBS		+=	WHIMORYPPN

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

NAND_DIR		:=	$(patsubst %/,%,$(dir $(GET_LOCAL_DIR)))

MODULES += \
	drivers/flash_nand/OAM \
	lib/blockdev

GLOBAL_INCLUDES += \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/VFL \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/FTL \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/FPart \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/Misc \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Boot \
	$(NAND_DIR)/raw/Whimory/Inc \
	$(NAND_DIR)/OAM \
	$(NAND_DIR)/OAM/iBoot

$(info WHIMORYPPN_DIR is $(WHIMORYPPN_DIR))

# base library files
WHIMORYPPN_SUBOBJS += \
	$(WHIMORYPPN_DIR)/ppn.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/FPart/PPNFPart.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/Misc/PPNMisc.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/Misc/VFLBuffer.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/FTL/yaFTL.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/FTL/yaFTL_BTOC.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Core/VFL/PPNVFLInterface.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Boot/WhimoryBoot.o \
	$(WHIMORYPPN_DIR)/WhimoryPPN/Boot/RegVanilla.o

WHIMORYPPN_SUBOBJS	:=	$(call TOLIBOBJDIR,$(WHIMORYPPN_SUBOBJS))
ALL_DEPS		+=	$(WHIMORYPPN_SUBOBJS:%o=%d)

WHIMORYPPN_OBJS		:=	$(call TOLIBOBJDIR,$(WHIMORYPPN_DIR)/WHIMORYPPN.o)


$(WHIMORYPPN_OBJS):  $(WHIMORYPPN_SUBOBJS)
	@echo LIB_LD $@
	$(_v)$(_LD) $(LIBRARY_LDFLAGS) $(WHIMORYPPN_SUBOBJS) -o $@  -exported_symbols_list $(WHIMORYPPN_DIR)/export.txt

$(WHIMORYPPN_BUILD):	$(WHIMORYPPN_OBJS)

endif
