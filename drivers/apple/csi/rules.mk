# Copyright (C) 2011-2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS += \
	WITH_CSI=1

# The additional paths below are used to find the ans iop firmware 
# file, "AppleCSI.*.h" and a protocol definition file, 
# "AppleCSIProtocol.h" which defines communications between coproc firmware
# and the csi driver.  The $(DSTROOT) path is useful only during development.
GLOBAL_INCLUDES	+=	$(LOCAL_DIR)/include \
					$(DSTROOT)/usr/local/standalone/firmware \
					$(SDKROOT)/../../../usr/local/standalone/firmware

ALL_OBJS += $(LOCAL_DIR)/csi.o \
			$(LOCAL_DIR)/debug.o \
			$(LOCAL_DIR)/endpoints/builtin.o \
			$(LOCAL_DIR)/endpoints/management_ep.o \
			$(LOCAL_DIR)/endpoints/syslog_ep.o \
			$(LOCAL_DIR)/endpoints/console_ep.o \
			$(LOCAL_DIR)/endpoints/crashlog_ep.o \
			$(LOCAL_DIR)/firmware/csi_firmware.o \
			$(LOCAL_DIR)/firmware/fw_ans.o \
