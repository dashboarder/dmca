# Copyright (C) 2007-2014 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

###############################################################################
# Build rules for the iBoot application.
#
# Recognises values in
#
#	TARGETS
#	PRODUCTS
#	BUILDS
#

#
# These are the config, product and build kinds that the application
# code recognises.
#
VALID_PRODUCTS	:=	LLB iBoot iBSS iBEC
VALID_BUILDS	:=	RELEASE DEVELOPMENT DEBUG SECRET

# support for temporarily turning off targets
SUPPRESSED_TARGETS :=

###############################################################################
###############################################################################
# No user-serviceable parts below
###############################################################################
###############################################################################

#
# Directory containing configuration files.
#
# Must do this before including any other makefile.
#
CONFIG_DIR	:=	$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))/config

# DEVICEMAP macros
include makefiles/device_map.mk

#
# Consult the configuration directory for the set of targets we know
# how to build, and the device map database for the set of targets
# that the current train wants to build.
#
VALID_TARGETS	:=	$(sort $(subst -config.mk,,$(notdir $(wildcard $(CONFIG_DIR)/*-config.mk))))
MAP_TARGETS_ALL	:=	$(sort $(DEVICEMAP_TARGETS))

# Filter MAP_TARGETS for targets with at least one PRODUCT in their manifest.
MAP_TARGETS_WITH_PRODUCTS := \
			$(sort $(call DEVICEMAP_TARGETS_FOR_PRODUCTS,$(VALID_PRODUCTS)))
MAP_TARGETS	:=	$(filter $(MAP_TARGETS_ALL),$(MAP_TARGETS_WITH_PRODUCTS))
FILTERED_OUT_TARGETS	:= \
			$(filter-out $(MAP_TARGETS_WITH_PRODUCTS),$(MAP_TARGETS_ALL))

#
# Generate and validate the list of targets that we are going to
# build.
#
TARGETS		?=	$(filter-out $(SUPPRESSED_TARGETS),$(MAP_TARGETS))
ERROR_TARGETS	:=	$(filter-out $(VALID_TARGETS), $(TARGETS))
ifneq ($(ERROR_TARGETS),)
$(warning CONFIG_DIR	$(CONFIG_DIR))
$(warning MAP_TARGETS   $(MAP_TARGETS))
$(warning VALID_TARGETS $(VALID_TARGETS))
$(error Unrecognized in TARGETS - $(ERROR_TARGETS))
endif

#
# If TARGETS contains a target not in the device map, we have to deal
# with not being able to look up PRODUCTS in the map for those
# targets.  Since the PRODUCTS handling below allows either an
# explicit list, OR a lookup in the database, we can't handle the case
# where some TARGETS are in the map and some aren't.
#
NONMAP_TARGETS	:=	$(filter-out $(MAP_TARGETS), $(TARGETS))
ifneq ($(NONMAP_TARGETS),)
ifneq ($(NONMAP_TARGETS), $(TARGETS))
$(error Some TARGETS not listed in device map (must be all or none) - $(NONMAP_TARGETS))
endif
endif
PRODUCTS	?=	$(VALID_PRODUCTS)

#
# Validate or fetch the list of PRODUCTS that we are going to build.
#
# In the macro form of PRODUCTS, the <target> argument may also be
# a partially-constructed <target>-<param>... word.
#
# If we invoke DEVICEMAP_PRODUCTS_FOR_TARGET we will get back product
# types that we don't know about, so we have to filter them out at
# this point.
#
# Note that following this it is necessary to use $(call PRODUCTS)
# to get the PRODUCTS list.
#
ERROR_PRODUCTS	=	$(filter-out $(VALID_PRODUCTS),$(PRODUCTS))
ifneq ($(ERROR_PRODUCTS),)
$(error Unrecognized in PRODUCTS - $(ERROR_PRODUCTS))
endif
PRODUCTS	?=	$(filter $(VALID_PRODUCTS),$(call DEVICEMAP_PRODUCTS_FOR_TARGET,$(word 1, $(subst -, ,$(1)))))

#
# Validate the list of build types we're going to build.
#
BUILDS		?=	$(filter-out SECRET, $(VALID_BUILDS))
ERROR_BUILDS	=	$(filter-out $(VALID_BUILDS), $(BUILDS))
ifneq ($(ERROR_BUILDS),)
$(error Unrecognized in BUILDS - $(ERROR_BUILDS))
endif

#
# Generate a list of <target>-<product>-<build> words.
#
# These words are used to define the things to be built, and are
# decomposed to extract the build configuration in the ACTIONS rule
# below.
#
LIST		:=	$(call DEVICE_MAP_TPB_FILTERED,$(TARGETS),$(PRODUCTS),$(BUILDS))

#
# Generate rules for the standard actions that depend on targets
# defining the things to be built.
#
# For each <action>, generate a dependency on
# <action>-<target>-<product>-<build> for every valid
# permutation of <action>, <target>, <product> and <build>.
#
LIST_TEMPLATE	:=	$(addprefix %-,$(LIST))
$(STANDARD_ACTIONS):%:	$(LIST_TEMPLATE)

#
# Generate a list of action words corresponding to the depended
# targets that was produced for the STANDARD_ACTIONS.
#
ACTIONS		:=	$(foreach action,$(STANDARD_ACTIONS),$(addprefix $(action)-,$(LIST)))

#
# Configuration options for built products - could probably move this to products.mk
#
DFU_IMAGE_LLB		:= false
DFU_IMAGE_iBoot		:= false
DFU_IMAGE_iBEC		:= true
DFU_IMAGE_iBSS		:= true

# Look up per-target parameters once so that we don't have to go to the database
# every time we invoke make below
TGT_DICT		:= $(call DEVICEMAP_TGT_DICT)

# Look up per-target/product parameters once as well
TGT_PRODUCT_DICT	:= $(call DEVICEMAP_TGT_PRODUCT_DICT, $(VALID_PRODUCTS))

#
# Template rule to build things.  
#
$(ACTIONS):	action	=	$(word 1, $(subst -, ,$@))
$(ACTIONS):	target	=	$(word 2, $(subst -, ,$@))
$(ACTIONS):	product	=	$(word 3, $(subst -, ,$@))
$(ACTIONS):	build	=	$(word 4, $(subst -, ,$@))
$(ACTIONS):	install_name =	$(product).$(target).$(build)
$(ACTIONS):
	@echo %%% $(action) $(install_name)
	@$(MAKE) -f makefiles/main.mk \
		SUB_TARGET=$(target) \
		CONFIG="" \
		PRODUCT=$(product) \
		BUILD=$(build) \
		APPLICATION_OPTIONS=$(APPLICATION_OPTIONS) \
		INSTALL_NAME=$(install_name) \
		DFU_IMAGE=$(DFU_IMAGE_$(product)) \
		IMAGE_TYPE=$(call DEVICEMAP_TGT_PRODUCT_DICT_PAYLOADTYPE, $(TGT_PRODUCT_DICT), $(target),$(product)) \
		DEVMAP_EPOCH=$(call DEVICEMAP_TGT_DICT_EPOCH, $(TGT_DICT), $(target)) \
		DEVMAP_PRODUCT_ID=$(call DEVICEMAP_TGT_DICT_PRODUCTID, $(TGT_DICT), $(target)) \
		IMAGE_FORMAT=$(or $(call DEVICEMAP_TGT_DICT_IMGFMT, $(TGT_DICT), $(target), $(IMAGE_FORMAT))) \
		CRYPTO_HASH=$(call DEVICEMAP_TGT_DICT_CRYPTOHASH, $(TGT_DICT), $(target)) \
		$(action)

help:
	@echo Valid TARGETS
	@echo "    $(VALID_TARGETS)"
	@echo Valid PRODUCTS
	@echo "    $(VALID_PRODUCTS)"
	@echo Valid BUILDS
	@echo "    $(VALID_BUILDS)"
	@echo Elided TARGETS with no matching PRODUCTS
	@echo "    $(FILTERED_OUT_TARGETS)"
	@echo ""
	@echo Default products:
	@echo $(LIST)
