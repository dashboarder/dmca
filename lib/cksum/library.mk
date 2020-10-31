# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBCKSUM_DIR		:=	$(GET_LOCAL_DIR)
LIBCKSUM_BUILD		:=	$(call TOLIBDIR,$(LIBCKSUM_DIR)/LIBCKSUM.a)
COMMONLIBS		+=	LIBCKSUM

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

LIBCKSUM_OBJS		:=	\
				$(LIBCKSUM_DIR)/adler32.o \
				$(LIBCKSUM_DIR)/$(ARCH)/adler32vec.o \
				$(LIBCKSUM_DIR)/crc.o \
				$(LIBCKSUM_DIR)/crc32.o \
				$(LIBCKSUM_DIR)/debug.o \
				$(LIBCKSUM_DIR)/siphash.o

LIBCKSUM_OBJS		:=	$(call TOLIBOBJDIR,$(LIBCKSUM_OBJS))

ALL_DEPS		+=	$(LIBCKSUM_OBJS:%o=%d)

$(LIBCKSUM_BUILD):	$(LIBCKSUM_OBJS)

endif
