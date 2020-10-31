# Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	drivers/flash_nand/raw/Whimory/Inc \
	lib \
	$(LOCAL_DIR)

MODULES	+= \
	drivers/flash_nand/OAM

ifeq ($(APPLICATION),EmbeddedIOP)

# we need the CDMA
MODULES	+= \
	drivers/apple/cdma

ALL_OBJS += \
	$(LOCAL_DIR)/H2fmi_iop.o \
	$(LOCAL_DIR)/H2fmi_boot.o \
	$(LOCAL_DIR)/H2fmi_debug.o \
	$(LOCAL_DIR)/H2fmi_erase.o \
	$(LOCAL_DIR)/H2fmi_misc.o \
	$(LOCAL_DIR)/H2fmi_read.o \
	$(LOCAL_DIR)/H2fmi_write.o \
	$(LOCAL_DIR)/H2fmi_dma_iboot.o \
	$(LOCAL_DIR)/fmiss.o \
	$(LOCAL_DIR)/fmiss_raw.o

#ppn modules
ALL_OBJS += \
	$(LOCAL_DIR)/H2fmi_ppn.o \
	$(LOCAL_DIR)/H2fmi_ppn_iop.o \
	$(LOCAL_DIR)/fmiss_ppn.o

# Regular support for the FMI
else

MODULES += \
	drivers/flash_nand/id

OPTIONS += \
	WITH_HW_FLASH_NAND=1 \
	WITH_FMI=1

ALL_OBJS += \
	$(LOCAL_DIR)/H2FIL.o \
	$(LOCAL_DIR)/H2fmi.o \
	$(LOCAL_DIR)/H2fmi_boot.o \
	$(LOCAL_DIR)/H2fmi_debug.o \
	$(LOCAL_DIR)/H2fmi_erase.o \
	$(LOCAL_DIR)/H2fmi_misc.o \
	$(LOCAL_DIR)/H2fmi_read.o \
	$(LOCAL_DIR)/H2fmi_write.o \
	$(LOCAL_DIR)/H2fmi_dma_iboot.o \
	$(LOCAL_DIR)/H2fmi_test.o \
	$(LOCAL_DIR)/H2fmi_timing.o \
	$(LOCAL_DIR)/fmiss.o \
	$(LOCAL_DIR)/fmiss_raw.o

#ppn modules
ALL_OBJS += \
	$(LOCAL_DIR)/H2fmi_ppn.o \
	$(LOCAL_DIR)/H2fmi_ppn_fil.o \
	$(LOCAL_DIR)/fmiss_ppn.o

ALL_OBJS += \
	$(LOCAL_DIR)/debug.o

endif
