# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
SUB_PLATFORM	:=	s8003

include $(GET_LOCAL_DIR)/n66-config-base.mk

OPTIONS += \
       	TARGET_VCO_RANGE=2 \
       	TARGET_LPF_CTRL=4 \
       	TARGET_ICP_CTR=5