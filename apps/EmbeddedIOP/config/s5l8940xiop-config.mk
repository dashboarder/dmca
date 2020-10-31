# Copyright (C) 2009-2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

###############################################################################
# Target configuration for the s5l8940x IOP runtime
#

PLATFORM	:=	s5l8940x
PLATFORM_VARIANT:=	IOP
ARCH		:=	arm

TEXT_FOOTPRINT	:=	1024*1024

MODULES		+=	platform/$(PLATFORM)		\
			platform/$(PLATFORM)/chipid	\
			platform/$(PLATFORM)/pmgr 	\
			drivers/apple/a5iop		\
			drivers/apple/aic		\
			drivers/apple/h2fmi		\
			drivers/arasan/sdio


# tune up the CDMA s/g list allocation
OPTIONS		+=	CDMA_CHANNEL_CMDS=128

# functions we require
MODULES		+=	apps/EmbeddedIOP/function_fmi	\
			apps/EmbeddedIOP/function_sdio
