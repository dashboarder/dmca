# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# n28a iboot bootloader build config

CONFIG_SIM	:=	false
RECOVERY_MODE_IBSS	:=	false

include $(GET_LOCAL_DIR)/ipod6-config-base.mk
