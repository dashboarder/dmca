# Copyright (C) 2010-2011 Apple Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

#
# Audio driver
#
LOCAL_DIR		:= $(GET_LOCAL_DIR)

OPTIONS			+=	WITH_FUNCTION_AUDIODSP=1 \
				USE_SIDETONE=1

IOP_FUNCTIONS	+=	AUDIODSP

ALL_OBJS		+=	$(LOCAL_DIR)/iop_audiodsp.o \
					$(LOCAL_DIR)/loopback_process.o \
					$(LOCAL_DIR)/loopback_device.o \
					$(LOCAL_DIR)/timestamper.o \
					$(LOCAL_DIR)/debug_tap.o \
					$(LOCAL_DIR)/AudioUnits/iop_au_interface.o \
					$(LOCAL_DIR)/AudioUnits/AUSidetone.o \
					$(LOCAL_DIR)/AudioUnits/AUNull.o \

INSTALL_HEADERS	+=	$(LOCAL_DIR)/iop_audiodsp_protocol.h

# Assuming 8 k for each task, with 2 task.
# Assuming 16 k heap for au process
# 32 k heap total
export IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),32768)

GLOBAL_INCLUDES		+=	$(SDKROOT)/usr/local/standalone/firmware \
						$(SDKROOT)/usr/local/standalone/firmware/include \
						$(LOCAL_DIR)/AudioUnits \
						$(LOCAL_DIR)/AE2_I2S \
						$(LOCAL_DIR)/AE2_MCA \
						$(LOCAL_DIR)/AE2_DMA \
						$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework/Headers \
						$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework/PrivateHeaders
						
PREBUILT_STATICLIBS	+=	$(SDKROOT)/usr/local/standalone/firmware/libm.a \
						$(SDKROOT)/usr/local/standalone/firmware/libvDSP.a

