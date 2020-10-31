#
# Copyright (c) 2009 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	drivers/flash_nand/raw/Whimory/Inc

MODULES += \
	drivers/flash_nand/OAM

ALL_OBJS += \
	$(LOCAL_DIR)/nandid.o
