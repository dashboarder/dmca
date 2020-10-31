# Copyright (C) 2009-2011, 2014 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

TOBUILDDIR	=	$(addprefix $(BUILDDIR)/,$(1))
TOHDRDIR	=	$(addprefix $(HDRDIR)/,$(1))
TOLIBDIR	=	$(addprefix $(LIBDIR)/,$(1))
TOLIBOBJDIR	=	$(addprefix $(LIBOBJDIR)/,$(1))
TOSYMDIR	=	$(addprefix $(SYMDIR)/,$(1))
TOTOOLSDIR	=	$(addprefix $(TOOLS_BIN)/,$(1))
TOTESTDIR	=	$(addprefix $(TESTDIR)/,$(1))

# build the target directory in a rule
MKDIR		=	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi

# gets the local directory of the makefile. Requires GNU make 3.81
GET_LOCAL_DIR	=	$(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

# returns a variable's numeric value or zero if empty
NUMBER		=	$(or $(1),0)

# evaluate arithmetic using the bash $(()) construct
EVALUATE	=	$(shell echo $$(( $(1) )))

# add two variables (empty variables count as zero)
ADD		= 	$(call EVALUATE, $(call NUMBER,$(1)) + $(call NUMBER,$(2)))

# safe case transformations
UPCASE		=	$(shell echo $(1) | tr a-z A-Z)
DOWNCASE	=	$(shell echo $(1) | tr A-Z a-z)

# test whether OPTIONS mentions the argument
OPTION_ISSET	=	$(patsubst %,true,$(filter $(1),$(OPTIONS)) $(filter $(1)=%,$(OPTIONS)))

# fetch the value that OPTIONS defines for the argument
OPTION_VALUE	=	$(patsubst $(1)=%,%,$(filter $(1)=%,$(OPTIONS)))

# evil hack to get a space as the first argument to $(subst ...)
empty		:=

# for generating definition of obfuscated logging ID from filename 
HASH_FOR_FILE		=	$(or $(word 2,$(subst :, ,$(filter $<:%,$(DEBUG_FILENAME_HASHES)))),$(error Error: debug hash not found for source file $< needed for $@))
DEBUG_FILE_HASH		=	-DDEBUG_FILE_HASH=0x$(HASH_FOR_FILE)ULL
