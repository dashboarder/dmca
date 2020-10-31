# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBLZSS_DIR		:=	$(GET_LOCAL_DIR)
LIBLZSS_BUILD	:=	$(call TOLIBDIR,$(LIBLZSS_DIR)/LIBLZSS.a)
COMMONLIBS		+=	LIBLZSS

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBLZSS_OBJS		:=	\
				$(LIBLZSS_DIR)/lzss.o \
				$(LIBLZSS_DIR)/$(ARCH)/lzssdec.o

LIBLZSS_OBJS		:=	$(call TOLIBOBJDIR,$(LIBLZSS_OBJS))

ALL_DEPS		+=	$(LIBLZSS_OBJS:%o=%d)

$(LIBLZSS_BUILD):	$(LIBLZSS_OBJS)

endif
