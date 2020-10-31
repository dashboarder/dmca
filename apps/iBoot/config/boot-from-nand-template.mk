# Copyright (C) 2011 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.

SUBMODULES_NAND_BOOT		+=		\
	$(SUBMODULES_NAND)			\
	drivers/flash_nand/boot

MODULES_FIRMWARE		+=		\
	$(SUBMODULES_NAND_BOOT)			\
	drivers/flash_nand/boot/firmware

MODULES_NVRAM			+=		\
	$(SUBMODULES_NAND_BOOT)			\
	drivers/flash_nand/boot/nvram

MODULES_SYSCFG			+=		\
	$(SUBMODULES_NAND_BOOT)			\
	drivers/flash_nand/boot/syscfg

MODULES_FILESYSTEM		+=		\
	$(SUBMODULES_NAND_BOOT)			\
	drivers/flash_nand/ftl
