# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

################################################################################
# Makefile to build global headers common to multiple build targets in the iBoot tree
#
# The list of libraries to build is derived from the contents of the file named
# in HDRLIST, which encodes the global and local path to the header in the format:
#
# <global header path>@<module header path>
#

ifeq ($(MAKEPHASE),headers)

HEADER_SPECS	:=	$(shell touch $(HDRLIST) && cat $(HDRLIST))
HEADER_LIST	:=	$(subst ~,/,$(HEADER_SPECS))

BUILD_ACTIONS	:=	$(addprefix build@,$(HEADER_SPECS))
CLEAN_ACTIONS	:=	$(addprefix clean@,$(HEADER_SPECS))

ACTIONS		:=	$(BUILD_ACTIONS) $(CLEAN_ACTIONS)

$(ACTIONS):	$(MAKEFILE_LIST) $(HDRLIST)
$(ACTIONS):	spec	=	$(subst @, ,$@)
$(ACTIONS):	action	=	$(word 1,$(spec))
$(ACTIONS):	header	=	$(word 3,$(spec))
$(ACTIONS):	module	=	$(dir $(header))
$(ACTIONS):
		@echo "%%%% $(action) $(header)"
		$(_v)$(MAKE) -f $(module)header.mk \
			HEADER=$(header) \
			$(action)

build:		$(BUILD_ACTIONS)

clean:
		$(_v)rm -f $(HEADER_LIST)

endif

