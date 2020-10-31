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
# CE-ATA driver
#
LOCAL_DIR		:= $(GET_LOCAL_DIR)

OPTIONS			+=	WITH_FUNCTION_CEATA=1
IOP_FUNCTIONS		+=	CEATA

ALL_OBJS		+=	$(LOCAL_DIR)/iop_ceata.o

export FMI_PROTOCOL_HEADER:=	$(LOCAL_DIR)/iop_ceata_protocol.h

export WITH_FUNCTION_CEATA=	1

export IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),8192)

