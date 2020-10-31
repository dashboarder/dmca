# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# MaltaUIRef iBoot bootloader build config
SUB_PLATFORM := s8003

include $(GET_LOCAL_DIR)/s8000ref-config-base.mk

ADBE_VERSION := 2

MODULES_BOOT += \
    drivers/apple/adbe \
    drivers/apple/adfe_v2 \
    drivers/apple/dither \
    drivers/apple/dpb \
    drivers/apple/displaypipe \
    drivers/display_pmu/chestnut \
    drivers/display/pinot \
    drivers/synopsys/mipi \
    drivers/backlight/lm3534

OPTIONS += \
    DISPLAY_D620_TUNABLES=1 \
    TARGET_VID0_SRC_SEL=0x9 \
    TARGET_DISPLAY_D620=1 \
    WITH_HW_MIPI=1 \
    WITH_HW_AGC_MIPI=0

include $(APPDIR)/products.mk

# override default platform display region size config
DISPLAY_SIZE := 34*1024*1024
