# Copyright (C) 2011-2013, 2015 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

EXTERNAL_STATICLIBS	+= $(SDKROOT)/usr/local/lib/libcorecrypto_iboot.a

OPTIONS			+= \
	WITH_CORECRYPTO=1 \
	MENU_TASK_SIZE=0x3000 \
	USB_VENDOR_TASK_SIZE=0x3000