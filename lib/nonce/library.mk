# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBNONCE_DIR		:=	$(GET_LOCAL_DIR)
LIBNONCE_BUILD		:=	$(call TOLIBDIR,$(LIBNONCE_DIR)/LIBNONCE.a)
COMMONLIBS		+=	LIBNONCE

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBNONCE_OBJS		:=	$(LIBNONCE_DIR)/nonce.o

LIBNONCE_OBJS		:=	$(call TOLIBOBJDIR,$(LIBNONCE_OBJS))

ALL_DEPS		+=	$(LIBNONCE_OBJS:%o=%d)

$(LIBNONCE_BUILD):	$(LIBNONCE_OBJS)

endif
