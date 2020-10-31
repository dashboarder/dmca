# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

#
# Flash Memory Interface driver
#
LOCAL_DIR		:= $(GET_LOCAL_DIR)

OPTIONS			+=	WITH_FUNCTION_FMI=1
IOP_FUNCTIONS		+=	FMI

ALL_OBJS		+=	$(LOCAL_DIR)/iop_fmi.o

INSTALL_HEADERS		+=	$(LOCAL_DIR)/iop_fmi_protocol.h

# XXX this is probably excessive
IOP_HEAP_REQUIRED	:=	$(call ADD,$(IOP_HEAP_REQUIRED),8192)

