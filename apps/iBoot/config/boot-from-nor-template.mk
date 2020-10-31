# Copyright (C) 2011 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

MODULES_FIRMWARE		+=	\
	$(SUBMODULES_NOR)

MODULES_NVRAM			+=	\
	$(SUBMODULES_NOR)

MODULES_SYSCFG			+=	\
	$(SUBMODULES_NOR)

MODULES_FILESYSTEM		+=	\
	$(SUBMODULES_NAND)		\
	drivers/flash_nand/ftl
