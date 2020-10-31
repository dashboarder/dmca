# Copyright (C) 2007-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

###############################################################################
# Build rules for the SecureROM module.
#
# Recognises values in
#
#	BUILDS
#	TARGETS
#	CONFIGS
#

VALID_BUILDS	:=	ROMRELEASE DEBUG
VALID_TARGETS	:=	t8002 t8010
VALID_CONFIGS	:=	si fpga

# support for temporarily turning off targets
SUPPRESSED_TARGETS :=

###############################################################################
# No user-serviceable parts below

# DEVICEMAP macros
include makefiles/device_map.mk

BUILDS		?=	$(VALID_BUILDS)
MAKE_BUILDS	=	$(filter $(VALID_BUILDS), $(BUILDS))

CONFIGS		?=	$(VALID_CONFIGS)
MAKE_CONFIGS	=	$(filter $(VALID_CONFIGS), $(CONFIGS))

# Consult the device map database for the set of platforms associated with
# at least one target.
MAP_TARGETS	:=	$(DEVICEMAP_PLATFORMS)

TARGETS		?=	$(filter-out $(SUPPRESSED_TARGETS),$(MAP_TARGETS))
MAKE_TARGETS	=	$(filter $(VALID_TARGETS), $(TARGETS))

MAKE_PRODUCT	=	SecureROM

TC_LIST		=	$(foreach target, $(MAKE_TARGETS), $(addprefix $(target)-, $(MAKE_CONFIGS)))
LIST		=	$(foreach build,  $(MAKE_BUILDS),  $(addprefix $(build)-,  $(TC_LIST)))

LIST_TEMPLATE	:=	$(addprefix %-,$(LIST))
$(STANDARD_ACTIONS):%:	$(LIST_TEMPLATE)
ACTIONS		:=	$(foreach action,$(STANDARD_ACTIONS),$(addprefix $(action)-,$(LIST)))

ROM_IMAGE_si	:= true
ROM_IMAGE_fpga	:= true

build:		$(BUILD)
install:	$(INSTALL)
clean:		$(CLEAN)

$(ACTIONS):	action	=	$(word 1, $(subst -, ,$@))
$(ACTIONS):	target	=	$(word 3, $(subst -, ,$@))
$(ACTIONS):	config	=	$(word 4, $(subst -, ,$@))
$(ACTIONS):	product	=	$(MAKE_PRODUCT)
$(ACTIONS):	build	=	$(word 2, $(subst -, ,$@))
$(ACTIONS):	install_name =	$(product).$(target)$(config).$(build)
$(ACTIONS):
	@echo %%% $(action) $(target)$(config)-$(product)-$(build)
	@$(MAKE) -f makefiles/main.mk \
		SUB_TARGET=$(target) \
		CONFIG=$(config) \
		PRODUCT=$(product) \
		BUILD=$(build) \
		ROM_IMAGE=$(ROM_IMAGE_$(config)) \
		ROM_IMAGE_SIZE=64k \
		INSTALL_NAME=$(install_name) \
		DEVMAP_EPOCH=$(call DEVICEMAP_EPOCH_FOR_PLATFORM,$(target)) \
		IMAGE_FORMAT=$(or $(call DEVICEMAP_IMGFMT_FOR_PLATFORM,$(target)), $(IMAGE_FORMAT)) \
		CRYPTO_HASH=$(call DEVICEMAP_CRYPTO_HASH_FOR_PLATFORM,$(target)) \
		$(action)

help:
	@echo Valid TARGETS
	@echo "    $(VALID_TARGETS)"
	@echo Valid CONFIGS
	@echo "    $(VALID_CONFIGS)"
	@echo Valid BUILDS
	@echo "    $(VALID_BUILDS)"
	@echo ""
