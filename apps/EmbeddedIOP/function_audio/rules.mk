# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

#
# Audio driver
#
LOCAL_DIR		:= $(GET_LOCAL_DIR)

OPTIONS			+=	WITH_FUNCTION_AUDIO=1
IOP_FUNCTIONS		+=	AUDIO

ALL_OBJS		+=	$(LOCAL_DIR)/iop_audio.o \
				$(LOCAL_DIR)/libstd_stub.o
				
INSTALL_HEADERS		+=	$(LOCAL_DIR)/iop_audio_protocol.h

# We need 64 k for stack
# and 372064 for heap
# 192512 of which will be in SRAM
export IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),245088)

GLOBAL_INCLUDES		+=	$(SDKROOT)/usr/local/standalone/firmware \
				$(SDKROOT)/usr/local/standalone/firmware/include \
#				$(SDKROOT)/../../../usr/local/standalone/firmware/Accelerate.framework/Frameworks/vecLib.framework/Headers

PREBUILT_STATICLIBS	+=	$(SDKROOT)/usr/local/standalone/firmware/libm.a \
				$(SDKROOT)/usr/local/standalone/firmware/libvDSP.a \
				$(SDKROOT)/usr/local/standalone/firmware/libAudioCodecsA5.a
