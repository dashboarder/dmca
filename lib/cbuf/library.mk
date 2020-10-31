# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBCBUF_DIR		:=	$(GET_LOCAL_DIR)
LIBCBUF_BUILD		:=	$(call TOLIBDIR,$(LIBCBUF_DIR)/LIBCBUF.a)
COMMONLIBS		+=	LIBCBUF

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

GLOBAL_INCLUDES		+=	$(SRCROOT)/arch/arm/include

LIBCBUF_OBJS		:=	$(LIBCBUF_DIR)/cbuf.o

LIBCBUF_OBJS		:=	$(call TOLIBOBJDIR,$(LIBCBUF_OBJS))

ALL_DEPS		+=	$(LIBCBUF_OBJS:%o=%d)

$(LIBCBUF_BUILD):	$(LIBCBUF_OBJS)

endif
