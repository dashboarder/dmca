# Copyright (C) 2014 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

# only used for "external" chargers (like DIALOG_D2231)

OPTIONS += \
	WITH_HW_CHARGER_DIALOG=1 \
	WITH_HW_CHARGER=1

# the actual code is included by "pmu" driver, but we use this config to switch how that one compiles

MODULES += \
	drivers/dialog/pmu

