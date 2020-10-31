# Copyright (C) 2010 Apple, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

#
# L2V Test Benchmark
#
LOCAL_DIR		:= $(GET_LOCAL_DIR)
NAND_DIR		:= $(dir $(GET_LOCAL_DIR))
L2V_DIR			:= $(NAND_DIR)/raw/Whimory/Core/FTL/L2V

MODULES			+= drivers/flash_nand/OAM 

GLOBAL_INCLUDES		+= $(L2V_DIR)

OPTIONS += ENABLE_L2V_TREE=1

ALL_OBJS		+=\
			$(LOCAL_DIR)/benchmark.o \
		        $(LOCAL_DIR)/L2V_Test.o \
			$(L2V_DIR)/L2V_Forget.o \
			$(L2V_DIR)/L2V_Free.o \
		        $(L2V_DIR)/L2V_Funcs.o \
		        $(L2V_DIR)/L2V_Init.o \
		        $(L2V_DIR)/L2V_Mem.o \
		        $(L2V_DIR)/L2V_Print.o \
		        $(L2V_DIR)/L2V_Repack.o \
		        $(L2V_DIR)/L2V_Search.o \
		        $(L2V_DIR)/L2V_Types.o \
		        $(L2V_DIR)/L2V_Update.o \
		        $(L2V_DIR)/L2V_Valid.o

IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),1048576)

