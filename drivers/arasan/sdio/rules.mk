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

ifeq ($(APPLICATION),EmbeddedIOP)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

ALL_OBJS += \
	$(LOCAL_DIR)/sdiodrv_config.o \
	$(LOCAL_DIR)/sdiodrv_command.o \
	$(LOCAL_DIR)/sdiodrv_transfer.o \
	$(LOCAL_DIR)/sdiocommon/sdhc_debug.o \
	$(LOCAL_DIR)/sdiocommon/sdhc_registers.o \
	$(LOCAL_DIR)/sdiocommon/sdio_cmdprop.o

endif
