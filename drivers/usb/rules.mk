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

# Base USB Device stack
ALL_OBJS += \
	$(LOCAL_DIR)/usb.o \
	$(LOCAL_DIR)/usb_controller.o \
	$(LOCAL_DIR)/usb_core.o

ifneq ($(filter $(DOCKFIFOS), bulk),)
ALL_OBJS += \
	$(LOCAL_DIR)/usb_dockfifo_controller.o
endif

# SecureROM and LLB-DFU support
ifneq ($(filter $(OPTIONS), WITH_DFU_MODE=1), )

OPTIONS += \
	WITH_USB_DFU=1 \
	WITH_USB_MODE_DFU=1

ALL_OBJS += \
	$(LOCAL_DIR)/usb_dfu.o

endif

# Recovery mode support
ifneq ($(filter $(OPTIONS), WITH_RECOVERY_MODE=1), )

OPTIONS += \
	WITH_USB_MODE_RECOVERY=1

ifeq ($(filter RELEASE ROMRELEASE,$(BUILD)),)
OPTIONS += \
	WITH_BULK_UPLOAD=1
endif

MODULES += \
	lib/cbuf \
	lib/cksum

ALL_OBJS += \
	$(LOCAL_DIR)/usb_debug.o \
	$(LOCAL_DIR)/usb_serial.o \
	$(LOCAL_DIR)/usb_transfer.o

endif
