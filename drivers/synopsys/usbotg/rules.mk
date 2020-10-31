# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
# Copyright (C) 2007-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Computer, Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Computer, Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += 

OPTIONS += \
	WITH_HW_USB=1 \
	WITH_HW_SYNOPSYS_OTG=1

MODULES	+= \
	drivers/usb

ALL_OBJS += \
        $(LOCAL_DIR)/synopsys_otg.o 
