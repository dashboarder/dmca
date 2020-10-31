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
# Makefile to build unit tests
#
# Currently this only handles unit tests compiled for Mac OS/x86, but
# it should be possible to extend this to also compile for arm and run
# on targets (either in iOS or on bare metal)
#
# To add a test, add its makefile for TEST_MAKEFILES below. Each makefile
# describes a single binary, and must define the following:
#     TEST_NAME - The name of the test, will be used to name the binary
#     TEST_OBJS - The object files that go into the test
#
# Makefiles may optionally define the following:
#     TEST_INCLUDES - List of directories with include files for the test
#     TEST_CFLAGS - Flags to pass to the compiler for the test
#     TEST_SUPPORT_OBJS - Object files that go into the test without coverage info

TEST_MAKEFILES	:=	tests/unittest-tests.mk \
			drivers/sha1/tests.mk \
			lib/blockdev/tests.mk \
			lib/cksum/tests.mk \
			lib/devicetree/tests.mk \
			lib/env/tests.mk \
			lib/fs/apfs/tests-fuzz.mk \
			lib/fs/hfs/tests.mk \
			lib/fs/hfs/tests-fuzz.mk \
			lib/heap/tests.mk \
			lib/heap/tests-debugging.mk \
			lib/libc/tests.mk \
			lib/libc/tests-release.mk \
			lib/lzss/tests.mk \
			lib/nvram/tests.mk \
 			lib/paint/tests.mk \
			lib/syscfg/tests.mk \
			drivers/sha1/tests.mk \
			sys/tests.mk

#			arch/arm64/tests-mmu.mk \
			
BUILD_ONLY_TEST_MAKEFILES := \
	lib/devicetree/tests-fuzz.mk \
	lib/nvram/tests-fuzz.mk \
	lib/ticket/tests-fuzz.mk

#
# Gory details follow
#

include makefiles/macros.mk

ALL_TEST_MAKEFILES	:=	$(TEST_MAKEFILES) $(BUILD_ONLY_TEST_MAKEFILES)

ACTIONS		:=	 $(addprefix build~,$(ALL_TEST_MAKEFILES))

TESTS_FLAVOR	?=	tests-$(BUILD_OS)
ifeq ($(COVERAGE),YES)
TESTS_FLAVOR	:=	$(TESTS_FLAVOR)-coverage
endif
ifeq ($(SANITIZE),YES)
TESTS_FLAVOR	:=	$(TESTS_FLAVOR)-sanitize
endif

export TESTSDIR	:=	$(OBJROOT)/build/$(TESTS_FLAVOR)
RUN_SH		:=	$(TESTSDIR)/run.sh

$(ACTIONS):	$(ALL_TEST_MAKEFILES) $(MAKEFILE_LIST)
$(ACTIONS):	spec		= $(subst ~, ,$@)
$(ACTIONS):	action		= $(word 1,$(spec))
$(ACTIONS):	TEST_MAKEFILE	= $(word 2,$(spec)) 
$(ACTIONS):
	@echo %%%% Building tests $(TEST_MAKEFILE)
	@$(MAKE) -f makefiles/test.mk \
		 TEST_MAKEFILE="$(TEST_MAKEFILE)"

RUN_TESTBINS	:=
BUILD_TESTBINS	:=

define template
include $$(TEST_MAKEFILE)
TESTBIN		:= $$(TESTSDIR)/$$(TEST_NAME)/$$(TEST_NAME)

ifeq ($$(filter $$(TEST_MAKEFILE),$$(BUILD_ONLY_TEST_MAKEFILES)),)
RUN_TESTBINS	+= $$(TESTBIN)
else
BUILD_TESTBINS	+= $$(TESTBIN)
endif

$$(TESTBIN):	build~$$(TEST_MAKEFILE)
endef

$(foreach TEST_MAKEFILE,$(ALL_TEST_MAKEFILES),$(eval $(template)))

$(RUN_SH):	$(RUN_TESTBINS) $(MAKEFILE_LIST)
	@$(MKDIR)
	$(_v)echo '#!/bin/bash' > $@
	$(_v)echo 'result=0' >> $@
	$(_v)for i in $(RUN_TESTBINS); do \
			echo "echo 'TEST $$i'" >> $@; \
			echo "(cd $$(dirname $$i); ./$$(basename $$i)); test_result=\$$?" >> $@; \
			echo "[[ \$$test_result == 0 ]] || echo '!!! Test Failed:' $$i" >> $@; \
			echo "result=\$$((\$$result || \$$test_result))" >> $@; \
			echo >> $@; \
		done
	$(_v)echo 'exit $$result' >> $@
	$(_v)chmod a+x $@

.PHONY: build
build:		$(RUN_SH) $(BUILD_TESTBINS)

run:		build
	@echo
	@echo %%%% Running tests
	@echo
	@$(RUN_SH)
ifeq ($(COVERAGE),YES)
	@mkdir -p $(SYMROOT)/build/coverage
	@echo COVERAGE
	$(_v)./tools/gcovr -b -r $(OBJROOT) --html -o $(SYMROOT)/build/coverage/index.html --html-details
endif
