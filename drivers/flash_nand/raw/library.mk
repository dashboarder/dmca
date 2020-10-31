# Copyright (C) 2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

WHIMORY_DIR		:=	$(GET_LOCAL_DIR)
WHIMORY_BUILD		:=	$(call TOLIBDIR,$(WHIMORY_DIR)/WHIMORY.a)
COMMONLIBS		+=	WHIMORY

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

NAND_DIR		:=	$(patsubst %/,%,$(dir $(GET_LOCAL_DIR)))

MODULES += \
	drivers/flash_nand/OAM \
	lib/blockdev

GLOBAL_INCLUDES += \
	$(WHIMORY_DIR)/Whimory/Core/VFL \
	$(WHIMORY_DIR)/Whimory/Core/FTL \
	$(WHIMORY_DIR)/Whimory/Core/FPart \
	$(WHIMORY_DIR)/Whimory/Exam \
	$(WHIMORY_DIR)/Whimory/Inc \
	$(NAND_DIR)/OAM \
	$(NAND_DIR)/OAM/iBoot

# base library files
WHIMORY_SUBOBJS += \
	$(WHIMORY_DIR)/raw_nand.o \
	$(WHIMORY_DIR)/Whimory/Core/FTL/FTLInterface.o \
	$(WHIMORY_DIR)/Whimory/Core/FPart/FPart.o \
	$(WHIMORY_DIR)/Whimory/Core/FTL/yaFTL.o \
	$(WHIMORY_DIR)/Whimory/Core/FTL/yaFTL_BTOC.o \
	$(WHIMORY_DIR)/Whimory/Core/VFL/VFLBuffer.o \
	$(WHIMORY_DIR)/Whimory/Core/VFL/VFLInterface.o \
	$(WHIMORY_DIR)/Whimory/Core/VFL/VSVFLInterface.o \
	$(WHIMORY_DIR)/Whimory/Exam/WMRExam.o \
	$(WHIMORY_DIR)/Whimory/Test/FILTest.o

WHIMORY_SUBOBJS		:=	$(call TOLIBOBJDIR,$(WHIMORY_SUBOBJS))
ALL_DEPS		+=	$(WHIMORY_SUBOBJS:%o=%d)

WHIMORY_OBJS		:=	$(call TOLIBOBJDIR,$(WHIMORY_DIR)/WHIMORY.o)

$(WHIMORY_OBJS):	$(WHIMORY_SUBOBJS)
	@echo LIB_LD $@
	$(_v)$(_LD) $(LIBRARY_LDFLAGS) $(WHIMORY_SUBOBJS) -o $@ -exported_symbols_list $(WHIMORY_DIR)/export.txt

$(WHIMORY_BUILD):	$(WHIMORY_OBJS)

endif
