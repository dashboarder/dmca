# Copyright (c) 2007-2009 Apple Inc. All rights reserved.
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

OPTIONS += \
	WITH_NAND=1

MODULES += \
	drivers/flash_nand/OAM \
	drivers/flash_nand/id

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/raw/Whimory/Inc \
	$(LOCAL_DIR)/ppn/WhimoryPPN/Core/Misc

ALL_OBJS += \
	$(LOCAL_DIR)/debug.o
