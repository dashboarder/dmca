# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

###############################################################################
# Target configuration for the s5l8950x AE2 runtime
#

PLATFORM	:=	s5l8950x
PLATFORM_VARIANT:=	Audio
ARCH		:=	arm

TEXT_FOOTPRINT	:=	1024*1024

OPTIONS		+=	MAP_SRAM_CACHED=1

MODULES		+=	platform/$(PLATFORM)		\
			platform/$(PLATFORM)/chipid	\
			drivers/apple/a5iop		\
			drivers/apple/aic		\
			drivers/apple/audio

# We need C++ support
MODULES		+=	lib/libc++

# functions we require
MODULES		+=	apps/EmbeddedIOP/function_audio \
			apps/EmbeddedIOP/function_audiodsp
