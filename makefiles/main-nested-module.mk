# Copyright (C) 2008 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

#
# Handle one iteration of the nested module include logic for main.mk
#
# The caller is expected to set:
#
# MODULES_VAR
#	The name of the variable containing modules to include, and
#	which will be set by included module makefiles.
#
# MODULE_MAKEFILE
#	The name of the makefile in the module directory
#
# LIBDIR
#	If the modules being processed are library modules, the root
#	of the current library object tree.
#

# strip modules that the configuration insists we cannot have
# strip modules that we have already included
# use $(sort ...) to remove duplicates in the list
MODULES_TO_INCLUDE	:=	$(sort $(filter-out $(MODULES_ALREADY_INCLUDED),$(filter-out $(MODULES_ELIDE),$($(MODULES_VAR)))))

# clear the modules variable so that included modules can request dependencies
$(MODULES_VAR)		:=

# template for each module we are including
define include_template
  LIBOBJDIR		:=	$$(addprefix $$(LIBDIR)/,$$(MODULE))
  MODULE_RULES_FILE	:=	$$(addprefix $$(MODULE)/,$$(MODULE_MAKEFILE))
  include $$(MODULE_RULES_FILE)
endef

# expand the template and execute
$(foreach MODULE,$(MODULES_TO_INCLUDE),$(eval $(include_template)))

# remember what we have included
MODULES_ALREADY_INCLUDED+=	$(MODULES_TO_INCLUDE)

# recursively invoke ourself if there are still modules to include
ifneq ($($(MODULES_VAR)),)
include makefiles/main-nested-module.mk
endif
