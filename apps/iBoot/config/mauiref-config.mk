# Copyright (C) 2012-2014 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# MauiRef iBoot bootloader build config

SUB_PLATFORM := s8000

include $(GET_LOCAL_DIR)/s8000ref-config-base.mk

include $(APPDIR)/products.mk
