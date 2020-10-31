# Copyright (C) 2014 Apple, Inc. All rights reserved.
#
# This document is the property of Apple, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#

LOCAL_DIR	:= $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

MIB_HDR_DIR	:= $(OBJROOT)/build/include/$(LOCAL_DIR)

MIB_GEN_HDR	:= $(LOCAL_DIR)/mib_gen_hdr.awk
MIB_BASE	:= $(LOCAL_DIR)/mib.base

build:		$(MIB_HDR_DIR)/mib_nodes.h

$(MIB_HDR_DIR)/mib_nodes.h:	$(MIB_BASE) $(MIB_GEN_HDR)
	@echo GEN $(MIB_HDR_DIR)/mib_nodes.h
	$(_v)mkdir -p $(MIB_HDR_DIR)
	$(_v)awk -f $(MIB_GEN_HDR) $(MIB_HDR_DIR) $(MIB_BASE)

