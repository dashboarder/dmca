# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LIBTICKET_DIR		:=	$(GET_LOCAL_DIR)
LIBTICKET_BUILD		:=	$(call TOLIBDIR,$(LIBTICKET_DIR)/LIBTICKET.a)
COMMONLIBS		+=	LIBTICKET

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

GLOBAL_INCLUDES		+=	$(SRCROOT)/lib/pki

LIBTICKET_OBJS		:=	$(LIBTICKET_DIR)/ticket.o \
				$(LIBTICKET_DIR)/DERApTicket.o

LIBTICKET_OBJS		:=	$(call TOLIBOBJDIR,$(LIBTICKET_OBJS))

ALL_DEPS		+=	$(LIBTICKET_OBJS:%o=%d)

$(LIBTICKET_BUILD):	$(LIBTICKET_OBJS)

endif
