# Copyright (C) 2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

SWISSPPN_DIR		:=	$(GET_LOCAL_DIR)
SWISSPPN_BUILD		:=	$(call TOLIBDIR,$(SWISSPPN_DIR)/SWISSPPN.a)
COMMONLIBS		+=	SWISSPPN

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

NAND_DIR		:=	$(patsubst %/,%,$(dir $(GET_LOCAL_DIR)))

MODULES += \
	drivers/flash_nand/OAM \
	lib/blockdev

GLOBAL_INCLUDES += \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SVFL \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/Misc \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/FPart \
	$(NAND_DIR)/ppn/WhimoryPPN/Boot \
	$(NAND_DIR)/raw/Whimory/Inc \
	$(NAND_DIR)/OAM \
	$(NAND_DIR)/OAM/iBoot

# base library files
SWISSPPN_SUBOBJS += \
	$(SWISSPPN_DIR)/ppn.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/FPart/PPNFPart.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/Misc/PPNMisc.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/Misc/VFLBuffer.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SVFL/s_vfl.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_boot.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_btoc.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_cxt.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_cxt_diff.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_cxt_load.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_external.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_gc.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_geom.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_init.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_internal.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_meta.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_read.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/s_sb.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Free.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Init.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Mem.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Repack.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Search.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Types.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Core/SFTL/L2V/L2V_Update.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Boot/WhimoryBoot.o \
	$(NAND_DIR)/ppn/WhimoryPPN/Boot/RegSwiss.o

SWISSPPN_SUBOBJS	:=	$(call TOLIBOBJDIR,$(SWISSPPN_SUBOBJS))
ALL_DEPS		+=	$(SWISSPPN_SUBOBJS:%o=%d)

SWISSPPN_OBJS		:=	$(call TOLIBOBJDIR,$(SWISSPPN_DIR)/SWISSPPN.o)


$(SWISSPPN_OBJS):  $(SWISSPPN_SUBOBJS)
	@echo LIB_LD $@
	$(_v)$(_LD) $(LIBRARY_LDFLAGS) $(SWISSPPN_SUBOBJS) -o $@  -exported_symbols_list $(SWISSPPN_DIR)/export.txt

$(SWISSPPN_BUILD):	$(SWISSPPN_OBJS)

endif
