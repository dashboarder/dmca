# Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

#
# SDIO driver
#
LOCAL_DIR		:=	$(GET_LOCAL_DIR)

OPTIONS			+=	WITH_FUNCTION_SDIO=1
IOP_FUNCTIONS		+=	SDIO

ALL_OBJS		+=	$(LOCAL_DIR)/iop_sdio.o $(LOCAL_DIR)/iop_sdio_wrapper.o

INSTALL_HEADERS		+=	$(LOCAL_DIR)/iop_sdio_protocol.h

# XXX tune this to suit SDIO requirements
IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),8192)

