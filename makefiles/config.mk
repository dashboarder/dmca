# Copyright (C) 2007-2015 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

################################################################################
# Configuration common to all makefiles
#

# Note that use of '=' assignment is intentional for COPTFLAGS and DEPLOYMENT_TARGET_FLAGS,
# allowing either to be overridden at make invocation for local development purposes.

# Clang has the concept of "smaller even if it's slightly slower". Use it.
COPTFLAGS		=	-Oz
# Use these flags to get almost useable stack traces
#COPTFLAGS		=	-O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls

# Use latest compiler runtime.
ifneq ($(RC_XBS), YES)
ifeq ($(SDK_PRODUCT_NAME),Watch OS)
DEPLOYMENT_TARGET_FLAGS	=	-mwatchos-version-min=$(SDKVERSION)
else
ifeq ($(SDK_PRODUCT_NAME),Apple TVOS)
DEPLOYMENT_TARGET_FLAGS	=	-mtvos-version-min=$(SDKVERSION)
else
ifeq ($(SDK_PRODUCT_NAME),iPhone OS)
DEPLOYMENT_TARGET_FLAGS	=	-miphoneos-version-min=$(SDKVERSION)
endif
endif
endif
endif

GLOBAL_INCLUDES		:=	include include/posix include/gcc include/c++ $(OBJROOT)/build/include
EXTERNAL_INCLUDES	:=	$(SDKROOT)/usr/local/include/

GLOBAL_CFLAGS		:=	-static $(COPTFLAGS) -mno-implicit-float
GLOBAL_CFLAGS		+=	-ffreestanding
GLOBAL_CFLAGS		+=	-fno-common # catches multiple declarations of global variables
GLOBAL_CFLAGS		+=	-funsigned-char -fno-strict-aliasing
GLOBAL_CFLAGS		+=	-fstack-protector-strong

CEXTRAFLAGS		:=	-nostdinc -std=gnu11
CPPEXTRAFLAGS		:=	-nostdinc++ -fno-exceptions -fno-rtti -std=gnu++98

# -Werror
CWARNFLAGS		:=	-W -Wall -Wbuiltin-memcpy-chk-size -Wno-multichar -Wno-unused-parameter -Wno-unused-function -Wno-int-to-void-pointer-cast -Wno-tautological-constant-out-of-range-compare
CPPWARNFLAGS		:= -W -Wall -Wno-multichar -Wno-unused-parameter -Wno-unused-function 

# Allow GNU-isms.
CWARNFLAGS		+=	-Wno-gnu
CPPWARNFLAGS		+=	-Wno-gnu

GLOBAL_ASFLAGS		:=	-D__ASSEMBLY__ -Dglobal=globl -Dword=long
GLOBAL_ASFLAGS		+=	-integrated-as

GLOBAL_ALLFLAGS		:=	-g -DIBOOT=1 $(DEPLOYMENT_TARGET_FLAGS)

GLOBAL_LDFLAGS		:=	-Wl,-new_linker -Wl,-preload -Wl,-merge_zero_fill_sections
GLOBAL_LDFLAGS		+=	-static -nostartfiles -dead_strip -e _start -nodefaultlibs
GLOBAL_LDFLAGS		+=	$(DEPLOYMENT_TARGET_FLAGS)
GLOBAL_LDFLAGS		+= 	-Wl,-sectalign,__TEXT,__cstring,40
GLOBAL_LDFLAGS		+= 	-Wl,-sectalign,__DATA,__data,40
GLOBAL_LDFLAGS		+= 	-Wl,-sectalign,__DATA,__zerofill,40
#// Hack while waiting for <rdar://problem/16136513> No const variables should end up in __DATA
GLOBAL_LDFLAGS		+=	-Wl,-rename_section,__DATA,__const,__TEXT,__const2


GLOBAL_LDFORCELIBS	:=

LIBRARY_LDFLAGS		:=	-Wl,-new_linker -static -nostartfiles -nodefaultlibs -r

ANALYZERFLAGS		=	--analyze -D__clang_analyzer__
ifneq ($(ANALYZE_FORMAT),text)
  ANALYZERFLAGS		+=	-Xanalyzer -analyzer-output=html
  ANALYZERFLAGS		+=	-o $(OBJROOT)/build/analyzer-html
else
  ANALYZERFLAGS		+=	-Xanalyzer -analyzer-output=text
endif
ifneq ($(ANALYZE_VERBOSE),YES)
  ANALYZERFLAGS		+=	-Xclang -analyzer-disable-checker -Xclang deadcode.DeadStores
endif

ifeq ($(SANITIZE),YES)
  SANITIZE_CFLAGS	:=	-fsanitize=address
  SANITIZE_LDFLAGS	:=	-fsanitize=address
endif

# Build host platform-specfic stuff here
ifeq ($(BUILD_OS),linux)

 # uint64_t is long on Linux, long long on Darwin
 # so just disable format string warnings on Linux
 CWARNFLAGS		+=	-Wno-format
endif
