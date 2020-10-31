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
# XXX update this
# Makefile to build a unit test binary
#
# We expect the following to be set:
#
# TEST_MAKEFILE
#	XXX update this
#

# Read the test makefile after reinitializing all variables the test might test
#

include makefiles/macros.mk
include makefiles/config.mk

TEST_OBJS		:=

TEST_SUPPORT_OBJS	:=

EXTRA_TEST_PRODUCTS	:=

# TESTDIR is the directory inside TESTSDIR that houses this test's
# object files and binaries. We need a unique tree per test because
# some source files may be compiled multiple times with different options
TESTDIR		=	$(TESTSDIR)/$(TEST_NAME)

include $(TEST_MAKEFILE)

TESTBIN		:=	$(call TOTESTDIR,$(TEST_NAME))

COMMON_CFLAGS	:=      -DWITH_HOST_LIBC=1 -DENABLE_RELEASE_ASSERTS=1 -funsigned-char -fno-strict-aliasing -g -Oz
COMMON_CFLAGS	+=	-fstack-protector-all
COMMON_INCLUDES	:=	$(SRCROOT)/include usr $(SRCROOT)/tests/include $(OBJROOT)/build/include

# XXX: should fix up tests so that this isn't needed
CWARNFLAGS		+=	-Wno-missing-field-initializers

ifeq ($(BUILD_OS),linux)
# iBoot assumes some BSDisms are available from the libc, but
# this isn't the case when building with Linux's libc, including
# the required prototypes in every .c file is somewhat of a dirty
# hack, but it gets the job done
COMMON_CFLAGS	+=	-include $(SRCROOT)/tests/include/non-posix.h
TEST_SUPPORT_OBJS	+=	tests/non-posix.o
endif

ifeq ($(COVERAGE),YES)
COVERAGE_CFLAGS	:=	-ftest-coverage -fprofile-arcs
COMMON_LDFLAGS	+=	-ftest-coverage -fprofile-arcs
endif

_CFLAGS		:=	$(COMMON_CFLAGS) $(CWARNFLAGS) $(SANITIZE_CFLAGS) $(TEST_CFLAGS) -DHOST_TEST=1
_INCLUDES	:=	$(addprefix -I,$(TEST_INCLUDES)) $(addprefix -I,$(COMMON_INCLUDES))
_LDFLAGS	:=	$(COMMON_LDFLAGS) $(SANITIZE_LDFLAGS) $(TEST_LDFLAGS)
ifeq ($(BUILD_OS),darwin)
_LDFLAGS	+=	-Wl,-no_pie
endif

BUILT_OBJS	:=	$(call TOTESTDIR,$(TEST_OBJS) $(TEST_SUPPORT_OBJS))
ALL_DEPS	+=	$(BUILT_OBJS:%.o=%.d)

GLOBAL_SRCDEPS	+=	$(MAKEFILE_LIST)

# rule to build object files, omitting coverage info for files listed in TEST_SUPPORT_OBJS
$(TESTDIR)/%.o: _cflags = $(_CFLAGS) $(and $(filter-out $(addprefix %,$(TEST_SUPPORT_OBJS)),$@),$(COVERAGE_CFLAGS))
$(TESTDIR)/%.o: gcda = $(@:%.o=%.gcda)
$(TESTDIR)/%.o: $(SRCROOT)/%.c $(GLOBAL_SRCDEPS)
	@echo HOST_CC $@
	@$(MKDIR)
	@[ ! -f "$(gcda)" ] || rm "$(gcda)" # remove test coverage data files
	$(_v)${HOST_CC} -c -o $@ $(_cflags) $(DEBUG_FILE_HASH) $(_INCLUDES) -MD -MT $@ -MF $(@:%o=%d) $<

$(TESTBIN):	$(BUILT_OBJS) $(GLOBAL_SRCDEPS)
	@echo HOST_LD $@
	$(_v)$(HOST_CC) $(_LDFLAGS) -o $@ $(BUILT_OBJS)

BUILD_PREREQS		+=	$(TESTBIN) $(EXTRA_TEST_PRODUCTS)

build:	$(BUILD_PREREQS)

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(ALL_DEPS)
endif
# Empty rule for .d files
%.d:
%.Td:
