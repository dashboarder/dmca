# Copyright (C) 2007-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

###############################################################################
# This is the top-level makefile for everything built out of the iBoot source 
# tree.

# Find out what OS we're building on
export BUILD_OS		:=	$(shell uname -s| tr '[:upper:]' '[:lower:]')
$(info %%% building on OS $(BUILD_OS))

# Find the root of the source tree from the build system's perspective.
# could be $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))) with a newer make
# Note that buildit sets SRCROOT to the destination for installsrc, so it can be
# empty which breaks the VALID_APPLICATIONS probe.
ORIGINAL_SRCROOT	:=	$(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
export SRCROOT		?=	${ORIGINAL_SRCROOT}

# Applications listed in the APPLICATIONS argument are processed one at a 
# time, and the per-application makefile in ./makefiles is invoked.
VALID_APPLICATIONS	:=	$(filter-out .%,$(notdir $(shell find $(ORIGINAL_SRCROOT)/apps -mindepth 1 -maxdepth 1 -type d)))

# Valid applications that should not be built by default
SUPPRESSED_APPLICATIONS	:=

# Options that must not be set if building default or multiple APPLICATIONS
SPECIFIC_OPTIONS	:=	TARGETS CONFIGS

# Default SDK platform
ifeq ($(BUILD_OS),darwin)
export SDK_PLATFORM	?=	iphoneos.internal
 ifeq ($(SDKROOT),)
  SDKROOT_PATH		:=	$(shell xcodebuild -version -sdk $(SDK_PLATFORM) Path)
  export SDKROOT	:=	$(SDKROOT_PATH)
 else
  # SDKROOT may be a canonical name (iphoneos.internal). Normalize into a path.
  SDKROOT_PATH		:=	$(shell xcodebuild -version -sdk $(SDKROOT) Path)
  export SDKROOT	:=	$(SDKROOT_PATH)
  export SDK_PLATFORM	:=	$(shell xcodebuild -version -sdk $(SDKROOT) | head -1 | grep -o '(.*)' | tr -d \(\))
 endif
 
 export SDK_PRODUCT_NAME	:= $(shell xcodebuild -version -sdk $(SDKROOT) ProductName)

 ifeq ($(SDKVERSION),)
  export SDKVERSION	:=	$(shell xcodebuild -version -sdk $(SDK_PLATFORM) SDKVersion | head -1)
 endif
 ifeq ($(PLATFORMROOT),)
  export PLATFORMROOT	:=	$(shell xcodebuild -version -sdk $(SDK_PLATFORM) PlatformPath)
 endif

 export HOST_PLATFORM	?=	macosx
 ifeq ($(HOST_SDKROOT),)
  export HOST_SDKROOT	:=	$(shell xcodebuild -version -sdk $(HOST_PLATFORM) Path)
 endif
endif

export OBJROOT		?=	.
export SYMROOT		?=	.
export INSTALL_DIR	?=	$(DSTROOT)/usr/local/standalone/firmware/

###############################################################################
# No user-serviceable parts below

# Check for spaces in critical paths - we can't handle that
SPACE_CHECK		=	$(filter ALL,$(and $(findstring $(subst ~, ,~),$(1)),$(error $(2))))
SDK_ADVICE		=	A path to a component of the SDK contains one or more spaces; this is not supported.  Please relocate your SDK to a path without spaces
$(call SPACE_CHECK,$(SDKROOT_PATH),$(SDK_ADVICE) (SDKROOT=$(SDKROOT_PATH)))
$(call SPACE_CHECK,$(PLATFORMROOT),$(SDK_ADVICE) (PLATFORMROOT=$(PLATFORMROOT)))
$(call SPACE_CHECK,$(HOST_SDKROOT),$(SDK_ADVICE) (HOST_SDKROOT=$(HOST_SDKROOT)))
DIR_ADVICE		=	A path supplied in a variable contains one or more spaces; this is not supported.
$(call SPACE_CHECK,$(DSTROOT),$(DIR_ADVICE) (DSTROOT=$(DSTROOT)))
$(call SPACE_CHECK,$(OBJROOT),$(DIR_ADVICE) (OBJROOT=$(OBJROOT)))
$(call SPACE_CHECK,$(SYMROOT),$(DIR_ADVICE) (SYMROOT=$(SYMROOT)))
$(call SPACE_CHECK,$(SRCROOT),The iBoot sources are in a folder with one or more spaces in the path; this is not supported (SRCROOT=$(SRCROOT)))

# Check for the existence of required directories
DIRECTORY_CHECK		=	$(filter ALL,$(or $(findstring Directory,$(shell stat -Lf %HT $(1))),$(error $(2))))
DIR_MISSING_ADVICE	=	A path to a component of the SDK specifies a directory that does not exist
ifeq ($(BUILD_OS),darwin)
 $(call DIRECTORY_CHECK,$(SDKROOT_PATH),$(DIR_MISSING_ADVICE) (SDKROOT=$(SDKROOT_PATH)))
 $(call DIRECTORY_CHECK,$(PLATFORMROOT),$(DIR_MISSING_ADVICE) (PLATFORMROOT=$(PLATFORMROOT)))
 $(call DIRECTORY_CHECK,$(HOST_SDKROOT),$(DIR_MISSING_ADVICE) (HOST_SDKROOT=$(HOST_SDKROOT)))
endif

# Force the tools to emit consistent punctuation in diagnostics
export LC_ALL=C

# Work out what we are building
ifneq ($(APPS),)
 # APPS is a shortcut for APPLICATIONS, combine if you're dumb enough to specify both
 override APPLICATIONS	:=	$(strip $(APPLICATIONS) $(APPS))
endif
APPLICATIONS		?=	$(filter-out $(SUPPRESSED_APPLICATIONS), $(VALID_APPLICATIONS))
MAKE_APPLICATIONS	:=	$(filter $(VALID_APPLICATIONS), $(APPLICATIONS))
ERROR_APPLICATIONS	:=	$(filter-out $(VALID_APPLICATIONS), $(APPLICATIONS))

ifneq ($(ERROR_APPLICATIONS),)
$(error Unrecognized application(s) - $(ERROR_APPLICATIONS))
endif

# Protect against application-specific options if we are building multiple
# applications
ifneq ($(strip $(foreach opt,$(SPECIFIC_OPTIONS),$(value $(opt)))),)
ifneq ($(strip $(shell echo $(MAKE_APPLICATIONS) | wc -w)),1)
$(error must not specify multiple (or default) APPLICATIONS if any of [$(strip $(SPECIFIC_OPTIONS))] is set)
endif
endif

# Export the list of standard actions, and define targets for each per application.
# Application Makefiles will in turn generate targets based on STANDARD_ACTIONS
# so that we can tunnel more or less arbitrary targets into main.mk.
export STANDARD_ACTIONS	:=	build install library_list
APPLICATION_TEMPLATE	:=	$(addprefix %-,$(MAKE_APPLICATIONS))
ACTIONS			:=	$(foreach action,$(STANDARD_ACTIONS),$(addprefix $(action)-,$(MAKE_APPLICATIONS)))

# The 'help' target is special, in that applications are responsible for handling
# it directly.
HELP_APPLICATIONS	:=	$(addprefix help-,$(MAKE_APPLICATIONS))
ACTIONS			+=	$(HELP_APPLICATIONS)

# The 'build' targets are special, in that they depend on the non-standard
# 'libraries' target.
BUILD_APPLICATIONS	:=	$(addprefix build-,$(MAKE_APPLICATIONS))

# Library list
export LIBLIST		:=	$(OBJROOT)/build/library_list

# Global header list
export HDRLIST		:=	$(OBJROOT)/build/header_list

# Where generated host tools are to be found
export TOOLS_BIN	:=	$(OBJROOT)/build/tools

# Pick up or generate the XBS tag
ifeq ($(RC_ProjectSourceVersion),)
export XBS_BUILD_TAG	?=	"$(shell echo localbuild...$$(whoami)...$$(git symbolic-ref --short HEAD 2>/dev/null || echo 'detached')_$$(git rev-parse --short HEAD 2>/dev/null)$$([[ $$(expr $$(git status --porcelain 2>/dev/null| egrep -v '^(  |\?\?)' | wc -l)) == '0' ]] || echo '_dirty')...$$(date '+%Y/%m/%d-%H:%M:%S'))"
else
export XBS_BUILD_TAG	:=	"iBoot-$(RC_ProjectSourceVersion)"
endif

# Work out whether we are building under XBS or just buildit
ifneq ($(RC_XBS),)
ifeq ($(RC_BUILDIT),)
$(warning building under XBS)
export BUILDING_UNDER_XBS	:=	true
else
export BUILDING_UNDER_BUILDIT	:=	true
$(warning building under BUILDIT)
endif
endif

# Find our tools, now that we know what we're building and in what environment.
include makefiles/tools.mk

# Verbosity for build rules
VERBOSE			?=	NO
ifeq ($(VERBOSE),YES)
export _v =
else
export _v = @
endif

# Setup for parallel sub-makes based on 2 times number of logical CPUs
ifndef MAKEJOBS
 ifeq ($(BUILD_OS),darwin)
  export MAXJOBS	:=	$(shell echo $$((`/usr/sbin//sysctl -n hw.logicalcpu` * 2)) )
 else
  export MAXJOBS	:=	$(shell echo $$((`nproc` * 2)))
 endif
 ifeq ($(BUILDING_UNDER_XBS),true)
  export MAKEJOBS	:=	--jobs=$(MAXJOBS)
 else
  export MAKEJOBS	:= 	--jobs=$(shell echo $$(( $(MAXJOBS) > 8 ? 8 : $(MAXJOBS))) )
 endif
endif
# Help does not look so good if built parallel...
ifeq ($(MAKECMDGOALS),help)
 export MAKEJOBS = --jobs=1
endif

# debug filename hashes are needed for both library and application build phases
export DEBUG_FILENAME_HASHES	:=	$(shell ./tools/generate_debug_hashes.py --src .)

################################################################################
# Toplevel rules
#
all:		build

ifeq ($(BUILD_OS),darwin)
# By depending on libraries, we get the depended library pass out of the way
# and then application targets can build parallel.
build:		build-headers-and-libraries

# Don't depend on build; the target install knows how to do that.
install:	build-headers-and-libraries
else
# Linux can only build tests for now
build:		tests
endif

# Global headers must be built before the libraries but after we've generated
# the libary list so order them properly with this rule.
build-headers-and-libraries:	$(LIBLIST) build-headers build-libraries

build-libraries:	$(LIBLIST)
	@echo "%%%% building libraries"
	@$(MAKE) $(MAKEJOBS) -f makefiles/libraries.mk build MAKEPHASE=libraries SDKROOT=$(SDKROOT_PATH)

$(LIBLIST):	library_list
	@echo "%%%% making $(LIBLIST)"
	@cat /dev/null `find $(OBJROOT)/build -name *.library_list` | sort | uniq > $(LIBLIST)
	@$(CHECK_LIBLIST) $(LIBLIST)

build-headers:
	@echo "%%%% building global headers"
	@cat /dev/null `find $(OBJROOT)/build -name *.header_list` | sort | uniq > $(HDRLIST)
	@$(MAKE) $(MAKEJOBS) -f makefiles/headers.mk build MAKEPHASE=headers SDKROOT=$(SDKROOT_PATH)

clean:
	$(_v)rm -rf $(OBJROOT)/build 
	$(_v)rm -rf $(SYMROOT)/build

pre-help:
	@echo "%%% printing help"
	@echo "Valid APPLICATIONS"
	@echo "    ${VALID_APPLICATIONS}"
	@echo "Default APPLICATIONS"
	@echo "    ${MAKE_APPLICATIONS}"
	@echo ""

help:	pre-help $(HELP_APPLICATIONS)

build-tools:
	@echo "%%%% generating build tools"
	@$(MAKE) -f tools/Makefile build-tools

install-tools:
	@echo Generating install tools
	@$(MAKE) -f tools/Makefile install-tools

installsrc:
	ditto . $(SRCROOT)

installhdrs:
	@echo no headers installed

build-tests:
	@$(MAKE) $(MAKEJOBS) -f makefiles/tests.mk SDKROOT=$(SDKROOT_PATH) build

.PHONY: tests
tests:
	@$(MAKE) $(MAKEJOBS) -f makefiles/tests.mk SDKROOT=$(SDKROOT_PATH) run

cscope.files:
	@echo "Building file list for cscope and tags"
	@find . -name '*.h' -type f | grep -v ^..build > _cscope.files 2> /dev/null
	@find . -name '*.c' -type f | grep -v ^..build >> _cscope.files 2> /dev/null
	@find . -name '*.cpp' -type f | grep -v ^..build >> _cscope.files 2> /dev/null
	@find . -name '*.S' -type f | grep -v ^..build >> _cscope.files 2> /dev/null
	@echo > cscope.files 2> /dev/null
	@sort -u < _cscope.files >> cscope.files 2> /dev/null
	@rm -f _cscope.files _cscope.files2 2> /dev/null

cscope: cscope.files
	@echo "Building cscope database"
	@cscope -bvU 2> /dev/null

spotless realclean:
	rm -rf build cscope.*

# Linux only knows how to build tests for now, so elide this on Linux
ifeq ($(BUILD_OS),darwin)
# Define this late, so that manual dependencies stated above for
# the standard actions are satisfied before the automatic depdencies.
$(STANDARD_ACTIONS)	: % :	$(APPLICATION_TEMPLATE)

# Build actions require the build-libraries target, which is otherwise only seen as
# a co-dependency of the build target
$(BUILD_APPLICATIONS):	build-libraries

$(ACTIONS):	action		=	$(word 1, $(subst -, ,$@))
$(ACTIONS):	application	=	$(word 2, $(subst -, ,$@))
$(ACTIONS):
	@echo "%%%%%% $(action) $(application)"
	@$(MAKE) $(MAKEJOBS) -f apps/$(application)/$(application).mk $(action) SDKROOT=$(SDKROOT_PATH) APPLICATION=$(application) MAKEPHASE=$(action)
endif


