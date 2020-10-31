# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# N56 iboot bootloader build config
TARGET		:=	iphone7

ifeq ($(PRODUCT),iBEC)
MODULES_BASIC	+=	drivers/apple/oscar
endif

include $(GET_LOCAL_DIR)/iphone7-config-base.mk

# override default platform display region size config
DISPLAY_SIZE	:=	34*1024*1024

OPTIONS += \
	DISPLAY_D600_TUNABLES=1

