# Copyright (C) 2012-2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# ElbaUIRef iBoot bootloader build config

TARGET := elbaref
ADBE_VERSION		:=	2
LPDP_VERSION		:=	2

include $(GET_LOCAL_DIR)/s8001ref-config-base.mk

MODULES_BOOT		+= \
	drivers/apple/adbe \
	drivers/apple/adfe_v2 \
	drivers/apple/dither \
	drivers/apple/dpb_v2 \
	drivers/apple/displaypipe \
	drivers/displayport \
	drivers/apple/lpdp_phy \
	drivers/analogix/displayport \
	drivers/display/edp \
	drivers/backlight/lp8559

# override default platform display region size config
DISPLAY_SIZE		:=	108*1024*1024

OPTIONS += \
	ELBAUIREF=1

include $(APPDIR)/products.mk
