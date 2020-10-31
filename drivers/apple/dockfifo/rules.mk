# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

ifneq ($(filter $(DOCKFIFOS), uart),)
  OPTIONS += \
	  WITH_HW_DOCKFIFO_UART=1 \

  ALL_OBJS += \
	  $(LOCAL_DIR)/dockfifo_uart.o
endif

ifneq ($(filter $(DOCKFIFOS), bulk),)
  OPTIONS += \
	  WITH_HW_DOCKFIFO_BULK=1 \

OPTIONS += \
	WITH_HW_USB=1

  ALL_OBJS += \
	  $(LOCAL_DIR)/cobs.o \
	  $(LOCAL_DIR)/dockfifo_bulk.o
endif

ifneq ($(DOCKFIFOS),)
  ALL_OBJS += \

  GLOBAL_INCLUDES +=      $(LOCAL_DIR)/include
endif