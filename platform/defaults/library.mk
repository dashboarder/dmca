# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

LIBPD_DIR 		:= 	$(GET_LOCAL_DIR)
LIBPD_BUILD		:=	$(call TOLIBDIR,$(LIBPD_DIR)/LIBPD.a)
COMMONLIBS		+=	LIBPD

# Only process the remainder of the makefile if we are building libraries
#
ifeq ($(MAKEPHASE),libraries)

# Build definitions for the target/platform independent parts of the platform defaults code

SPECIFICATION		:=	$(LIBPD_DIR)/platform.defaults
TEMPLATE		:=	$(LIBPD_DIR)/template.c
LISTER			:=	$(LIBPD_DIR)/lister.awk
FORMATTER		:=	$(LIBPD_DIR)/formatter.awk

PLATFORM_DEFAULTS	:=	$(shell awk -v TAG=INDEP -f $(LISTER) $(SPECIFICATION))

ifneq ($(PLATFORM_DEFAULTS),)

GENERATED_SRC_DIR 	:=	$(call TOLIBOBJDIR,$(LIBPD_DIR))
GENERATED_NAMES		:=	$(foreach tf,$(PLATFORM_DEFAULTS),$(word 1,$(subst :, ,$(tf))))
GENERATED_SRCS		:=	$(addprefix $(GENERATED_SRC_DIR)/,$(addsuffix .c,$(GENERATED_NAMES)))
GENERATED_OBJS		:=	$(addprefix $(GENERATED_SRC_DIR)/,$(addsuffix .o,$(GENERATED_NAMES)))

$(GENERATED_SRCS):	$(SPECIFICATION) $(TEMPLATE) $(LISTER) $(FORMATTER)
$(GENERATED_SRCS):	func = $(notdir $(@:.c=))
$(GENERATED_SRCS):
	@echo GEN  $@
	@mkdir -p $(dir $@)
	@awk -v TEMPLATE=$(TEMPLATE) -v FUNCTION=$(func) -f $(FORMATTER) < $(SPECIFICATION) > $@

LIBPD_OBJS		:=	$(GENERATED_OBJS)

$(LIBPD_BUILD):		$(LIBPD_OBJS)

ALL_DEPS		+=	$(LIBPD_OBJS:%o=%d)

endif
endif
