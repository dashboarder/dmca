# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# fastsim-capri-ipad iboot bootloader build config

SUB_PLATFORM		:=	t7001
AMC_REG_VERSION		:=	5

include $(GET_LOCAL_DIR)/typhonic-config.mk

# override default platform display region size config
DISPLAY_SIZE		:=	64*1024*1024

