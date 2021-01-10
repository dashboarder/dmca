# -*- make -*-
# Copyright (C) 2007-2014 Apple Inc. All rights reserved.
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
# Tools used by makefiles.
#
# We look these up just once and cache their paths accordingly.
# This is less important for things that we expect to always exist, but for
# optional tools, xcrun does no negative caching.
#

export CP		:=	/bin/cp
export AWK		:=	/usr/bin/awk

# Project local scripts.
export CHECK_LIBLIST	:=	tools/check_liblist.py

# Note that it's not safe to look up any of the toolchain pieces 
# for the 'installsrc' and 'clean' targets, as they can be invoked by the 
# project submission process on a system with no tools installed.

ifeq ($(filter clean installsrc,$(MAKECMDGOALS)),)

ifeq ($(BUILD_OS),darwin)
# xcrun only makes sense when building on OS X, but even there we
# may sometimes want to use a non-xcode SDK compiler
USE_XCRUN		?=	YES
USE_XCRUN_HOST		?=	$(USE_XCRUN)
endif

ifeq ($(USE_XCRUN),YES)

$(info %%%% locating platform tools with xcrun)

export CC		:=	$(shell xcrun -sdk $(SDKROOT) -find clang)
export CPP		:=	$(shell xcrun -sdk $(SDKROOT) -find c++)
export SIZE		:=	$(shell xcrun -sdk $(SDKROOT) -find size)
export STRIP		:=	$(shell xcrun -sdk $(SDKROOT) -find strip)
export IMAGE3MAKER	:=	$(shell xcrun -sdk $(SDKROOT) -find image3maker)
export IMG4PAYLOAD	:=	$(shell xcrun -sdk $(SDKROOT) -find img4payload)
export AR		:=	$(shell xcrun -sdk $(SDKROOT) -find ar)
export NM		:=	$(shell xcrun -sdk $(SDKROOT) -find nm)
export RANLIB		:=	$(shell xcrun -sdk $(SDKROOT) -find ranlib)
export DSYMUTIL		:=	$(shell xcrun -sdk $(SDKROOT) -find dsymutil)
export DEVICEMAP	:=	secrets/embedded_device_map # $(shell xcrun -sdk $(SDKROOT) -find embedded_device_map)
export OTOOL		:=	$(shell xcrun -sdk $(SDKROOT) -find otool)
export ANALYZER		:=	$(CC)
export ANALYZERPP	:=	$(CPP)

# XXX WTF is this?
# this may not exist, so we have to resolve it here
export AES_SYNCH_FIXUP	:=	$(shell xcrun -sdk $(SDKROOT) -find aes-synch-fixup.sh 2>/dev/null || echo /notfound)

endif

ifeq ($(USE_XCRUN_HOST),YES)
export HOST_AR		:=	$(shell xcrun -sdk $(HOST_SDKROOT) -find ar)
export HOST_CC		:=	$(shell xcrun -sdk $(HOST_SDKROOT) -find clang) -isysroot $(HOST_SDKROOT)
export HOST_CPP		:=	$(shell xcrun -sdk $(HOST_SDKROOT) -find clang++) -isysroot $(HOST_SDKROOT)
#export HOST_CC		:=	/Users/jfortier/src/PR-afl/afl-0.87b/afl-clang -isysroot $(HOST_SDKROOT)
else
export HOST_AR		?=	ar
export HOST_CC		?=	clang
export HOST_CPP		?=	clang++
endif

# Log the paths to all of the tools that we have located, so that there is no
# room for doubt about what we're invoking.

$(info CP               $(CP))
$(info AWK              $(AWK))
$(info CC               $(CC))
$(info CPP              $(CPP))
$(info SIZE             $(SIZE))
$(info STRIP            $(STRIP))
$(info IMAGE3MAKER      $(IMAGE3MAKER))
$(info IMG4PAYLOAD      $(IMG4PAYLOAD))
$(info AR               $(AR))
$(info NM               $(NM))
$(info RANLIB           $(RANLIB))
$(info DSYMUTIL         $(DSYMUTIL))
$(info DEVICEMAP        $(DEVICEMAP))
$(info OTOOL            $(OTOOL))
$(info HOST_AR          $(HOST_AR))
$(info HOST_CC          $(HOST_CC))
$(info HOST_CPP         $(HOST_CPP))

endif
