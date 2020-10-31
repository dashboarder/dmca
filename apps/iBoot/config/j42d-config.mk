# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# j42d iboot bootloader build config
TARGET		:=	j42

include $(GET_LOCAL_DIR)/appletv5-config-base.mk

# override default platform SDRAM config
SDRAM_LEN		:=	2*1024*1024*1024

# SDRAM TEXT_BASE = SDRAM_BASE + SDRAM_LEN - 256MB
ifeq ($(TEXT_BANK),sdram)
TEXT_BASE		:=	0x870000000
TEXT_FOOTPRINT	:=	1024*1024
endif
