# Copyright (C) 2007-2009, 2012, 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR	:=	$(GET_LOCAL_DIR)

#
# Global headers that are independing of product, target, etc.
#
GLOBAL_HEADERS += \
	lib/mib/mib_nodes.h


ALL_OBJS	+=	$(LOCAL_DIR)/debugcmds.o \
			$(LOCAL_DIR)/main.o \
			$(LOCAL_DIR)/clock_management.o \
			$(LOCAL_DIR)/clock_stepping.o \
			$(LOCAL_DIR)/qwi.o

OPTIONS		+=	WITH_INTERRUPTS=1 \
			QWI_MAX_CHANNELS=8 \
			WITH_APPLICATION_PUTCHAR=1

WITH_NO_SECURITY	:= true

# basic heap allocation
export IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),28672)

ifeq ($(BUILD),DEBUG)
##############################################################################
# Debug build
#

# enable DCC console
WITH_DCC	:=	true
OPTIONS		+=	DEBUG_LEVEL=20 \
			DCC_TX_BUFFER_SIZE=4096 \
			APPLICATION_CONSOLE_BUFFER=4096 \
			WITH_MENU=1

#OPTIONS		+=	ARM_DCC_SYNCHRONOUS=1

# console 8K
# menu 5.5K
# commands ...
export IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),32768)

else
##############################################################################
# Release build
#

endif

##############################################################################
# Build configuration

MODULES		+=	lib/heap \
			sys

LIBRARY_MODULES	+=	lib/libc \
			lib/mib

GLOBAL_INCLUDES +=	$(LOCAL_DIR)

# defeat the default heap initialisation
OPTIONS		+=	HEAP_SIZE=0

# configure the message channel
export IOP_MESSAGE_CHANNEL_SIZE	=	8

# and the panic log buffer
export IOP_PANIC_LOG_SIZE =		32768
OPTIONS		+=	IOP_PANIC_LOG_SIZE=$(IOP_PANIC_LOG_SIZE) \
			WITH_PANIC_HOOKS=1

# basic protocol header
INSTALL_HEADERS	:=	$(LOCAL_DIR)/EmbeddedIOPProtocol.h \
			$(LOCAL_DIR)/qwi_protocol.h
