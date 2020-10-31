# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBHEAP_DIR		:=	$(GET_LOCAL_DIR)
LIBHEAP_BUILD		:=	$(call TOLIBDIR,$(LIBHEAP_DIR)/LIBHEAP.a)
COMMONLIBS		+=	LIBHEAP

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBHEAP_OBJS		:=	$(LIBHEAP_DIR)/heap.o \
				$(LIBHEAP_DIR)/libc_stub.o

LIBHEAP_OBJS		:=	$(call TOLIBOBJDIR,$(LIBHEAP_OBJS))

ALL_DEPS		+=	$(LIBHEAP_OBJS:%o=%d)

$(LIBHEAP_BUILD):	$(LIBHEAP_OBJS)

endif
