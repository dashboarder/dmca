# Copyright (C) 2008 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

################################################################################
# Makefile to build libraries common to multiple build targets in the iBoot tree
#
# The list of libraries to build is derived from the contents of the file named in LIBLIST, which
# encodes the library module path, library uniquing tag and build options for the library in the
# format:
#
# <module path>~<uniquing tag>[~<build option>[~<build option>[~...]]]
#
# It is assumed that the contents of the LIBLIST file have been uniqued and thus
# every library target is unique.
#

LIBRARY_SPECS	:=	$(shell touch $(LIBLIST) && cat $(LIBLIST))

BUILD_ACTIONS	:=	$(addprefix build~,$(LIBRARY_SPECS))
CLEAN_ACTIONS	:=	$(addprefix clean~,$(LIBRARY_SPECS))

ACTIONS		:=	$(BUILD_ACTIONS) $(CLEAN_ACTIONS)

$(ACTIONS):	$(MAKEFILE_LIST) $(LIBLIST)
$(ACTIONS):	spec	=	$(subst ~, ,$@)
$(ACTIONS):	action	=	$(word 1,$(spec))
$(ACTIONS):	module	=	$(word 2,$(spec))
$(ACTIONS):	tag	=	$(word 3,$(spec))
$(ACTIONS):	options	=	$(wordlist 4,$(words $(spec)),$(spec))
$(ACTIONS):
	@echo "%%%% $(action) $(module) as $(tag)"
	$(_v)$(MAKE) -f makefiles/lib.mk \
		MODULE=$(module) \
		TAG=$(tag) \
		OPTIONS="$(options)" \
		$(action)

build:		$(BUILD_ACTIONS)

clean:		$(CLEAN_ACTIONS)
