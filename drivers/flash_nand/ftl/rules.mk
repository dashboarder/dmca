# Copyright (c) 2011 Apple Inc. All rights reserved.
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
	WITH_NAND_FILESYSTEM=1

MODULES += \
	drivers/flash_nand

ALL_OBJS += \
	$(LOCAL_DIR)/nand.o

WITH_NAND_FTL := true
