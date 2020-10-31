# Copyright (C) 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# t8002fpga iboot bootloader build config

CONFIG_FPGA		:=	true
RECOVERY_MODE_IBSS	:=	false
NO_WFI			:=	false

include $(GET_LOCAL_DIR)/watch2-config-base.mk
