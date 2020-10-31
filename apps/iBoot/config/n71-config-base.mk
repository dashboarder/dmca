# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

include $(GET_LOCAL_DIR)/iphone8-config-base.mk

OPTIONS += \
	TARGET_DISPLAY_D520=1 \
	TARGET_VID0_SRC_SEL=0xA \
	DISPLAY_D520_TUNABLES=1 \
	WITH_HW_MIPI=1 \
	WITH_HW_AGC_MIPI=0
