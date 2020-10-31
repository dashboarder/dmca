# Copyright (C) 2010-2011, 2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

#
# Derivation of MODULES based on PRODUCT.
#
# This file should contain NO target-specific knowledge.
#

#
# Always build 'basic' modules.
#
MODULES			+=	$(MODULES_BASIC)


################################################################################
#
# iBoot
#
ifeq ($(PRODUCT),iBoot)

TEXT_BANK		:=	sdram
OPTIONS			+=	$(OPTIONS_RECOVERY) $(OPTIONS_BOOT) $(OPTIONS_FILESYSTEM) $(OPTIONS_RESTORE_STRAP)
MODULES			+=	$(MODULES_RECOVERY) $(MODULES_BOOT) $(MODULES_FILESYSTEM) $(MODULES_DISPLAY)

endif # iBoot


################################################################################
#
# iBEC
#
ifeq ($(PRODUCT),iBEC)

TEXT_BANK		:=	sdram
OPTIONS			+=	$(OPTIONS_RECOVERY) $(OPTIONS_BOOT) $(OPTIONS_FILESYSTEM) $(OPTIONS_RESTORE_BOOT)
MODULES			+=	$(MODULES_RECOVERY) $(MODULES_BOOT) $(MODULES_FILESYSTEM) 

endif # iBEC


################################################################################
#
# iBSS
#
ifeq ($(PRODUCT),iBSS)

TEXT_BANK		:=	sram
ifneq ($(RECOVERY_MODE_IBSS),true)
OPTIONS			+=	$(OPTIONS_DFU) $(OPTIONS_RESTORE_STRAP)
MODULES			+=	$(MODULES_DFU)

#
# It is also possible, for development or triage purposes, to rebuild
# iBSS with uart-only console support by changing NEVER below to DEBUG;
# however, do NOT submit such a change to trunk.
#
ifeq ($(BUILD),NEVER)
OPTIONS 		+= 	$(OPTIONS_CONSOLE)
MODULES 		+= 	$(MODULES_CONSOLE)
endif

else # RECOVERY_MODE_IBSS is true

# Dongle products get an iBSS with all of iBoot's recovery mode accroutements,
# EXCEPT for filesystem support
OPTIONS			+=	$(OPTIONS_RECOVERY) $(OPTIONS_BOOT) $(OPTIONS_RESTORE_BOOT)
MODULES			+=	$(MODULES_RECOVERY) $(MODULES_BOOT)

endif # RECOVERY_MODE_IBSS

endif # iBSS


################################################################################
#
# LLB
#
ifeq ($(PRODUCT),LLB)

TEXT_BANK		:=	sram
OPTIONS			+=	$(OPTIONS_DFU) $(OPTIONS_RESTORE_STRAP)
MODULES			+=	$(MODULES_DFU) $(MODULES_FIRMWARE)

#
# It is also possible, for development or triage purposes, to rebuild
# LLB with uart-only console support by changing NEVER below to DEBUG;
# however, do NOT submit such a change to trunk.
#
# This is intentionally of considerably less utility than the iBSS
# variant of the DFU-mode console since it only becomes available to
# break into if iBoot firmware image load/boot fails and the unit
# falls over into DFU mode.
#
ifeq ($(BUILD),NEVER)
OPTIONS 		+= 	$(OPTIONS_CONSOLE)
MODULES 		+= 	$(MODULES_CONSOLE)
endif

endif # LLB
