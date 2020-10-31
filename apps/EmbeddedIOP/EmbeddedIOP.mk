# Copyright (C) 2007 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

###############################################################################
# Build rules for the EmbeddedIOP firmware
#
# Recognises values in
#
#	BUILDS
#	TARGETS
#	CONFIGS
#

VALID_BUILDS	:=	RELEASE DEBUG

SUPPRESSED_TARGETS:=	s5l8720x

###############################################################################
# No user-serviceable parts below

CONFIG_DIR	:=	$(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))/config
VALID_TARGETS	:=	$(subst -config.mk,,$(notdir $(wildcard $(CONFIG_DIR)/*-config.mk)))

BUILDS		?=	$(VALID_BUILDS)
MAKE_BUILDS	=	$(filter $(VALID_BUILDS), $(BUILDS))

TARGETS		?=	$(filter-out $(SUPPRESSED_TARGETS),$(VALID_TARGETS))
MAKE_TARGETS	=	$(filter $(VALID_TARGETS), $(TARGETS))

ERROR_TARGETS	:=	$(filter-out $(VALID_TARGETS), $(TARGETS))

ifneq ($(ERROR_TARGETS),)
$(error Unrecognized target(s) - $(ERROR_TARGETS))
endif

MAKE_PRODUCT	=	EmbeddedIOP

LIST		=	$(foreach build,  $(MAKE_BUILDS),  $(addprefix $(build)-,  $(MAKE_TARGETS)))

LIST_TEMPLATE	:=	$(addprefix %-,$(LIST))
$(STANDARD_ACTIONS):%:	$(LIST_TEMPLATE)
ACTIONS		:=	$(foreach action,$(STANDARD_ACTIONS),$(addprefix $(action)-,$(LIST)))

$(ACTIONS):	action	=	$(word 1, $(subst -, ,$@))
$(ACTIONS):	target	=	$(word 3, $(subst -, ,$@))
$(ACTIONS):	product	=	$(MAKE_PRODUCT)
$(ACTIONS):	build	=	$(word 2, $(subst -, ,$@))
$(ACTIONS):	install_name =	$(product).$(target)$(config).$(build).h
$(ACTIONS):
	@echo %%% $(action) $(target)$(config)-$(product)-$(build)
	@$(MAKE) -f makefiles/main.mk \
		SUB_TARGET=$(target) \
		CONFIG="" \
		PRODUCT=$(product) \
		BUILD=$(build) \
		INSTALL_NAME=$(install_name) \
		IMAGE_WITH_HEADER=$(SRCROOT)/apps/EmbeddedIOP/EmbeddedIOPFirmware.h \
		BIN_INCLUDES_BSS=true \
		$(action)

help:
	@echo Valid TARGETS
	@echo "    $(VALID_TARGETS)"
	@echo Valid BUILDS
	@echo "    $(VALID_BUILDS)"
	@echo ""

