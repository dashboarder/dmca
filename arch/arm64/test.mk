# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

CC			:= $(shell xcodebuild -sdk macosx.internal -find clang)
GCOV			:= $(shell xcodebuild -sdk macosx.internal -find gcov)

TEST_DIR		:= host_tests

INCDIRS			:= . \
			   include \
			   ../arm/include \
			   ../../include

CFLAGS			:= -arch x86_64 \
			   -std=c11 \
			   -g -Wall -Werror -O \
			   $(addprefix -I,$(INCDIRS)) \
			   -DARCH_ARMv8 -DTEST

TESTS			:= mmu_test

TEST_PASSES		:= $(patsubst %,$(TEST_DIR)/%.pass,$(TESTS))

.PHONY:			all
all:			host-tests

.PHONY:			host-tests
host-tests:		$(TEST_PASSES)

.PHONY:			clean
clean:
	rm -rf $(TEST_DIR)
	rm -f *.gcno *.gcda

$(TEST_DIR)/mmu_test.pass:	$(TEST_DIR)/mmu_test
	@echo "Test running $(notdir $<)"
	@rm -f mmu_test.gcda mmu.gcda
	@$< 2>&1 1>$(TEST_DIR)/mmu_test.out || \
		{ cat $(TEST_DIR)/mmu_test.out; false; }
	@echo "Test passed $(notdir $<)"
	@$(GCOV) -a mmu.c
	@mv -f mmu.c.gcov $(TEST_DIR)
	@grep '#####' $(TEST_DIR)/mmu.c.gcov || true
	@touch $@

$(TEST_DIR)/mmu_test:		mmu_test.c mmu.c
	@echo "Build $(notdir $@)"
	@mkdir -p $(dir $@)
	@$(CC) -o $@ $^ $(CFLAGS) --coverage
