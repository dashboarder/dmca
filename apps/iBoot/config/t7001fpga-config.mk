# Copyright (C) 2012-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# t7001fpga iboot bootloader build config

TARGET			:=	t7000fpga
SUB_PLATFORM		:=	t7001
AMC_REG_VERSION		:=	5

include $(GET_LOCAL_DIR)/t7000fpga-config.mk
