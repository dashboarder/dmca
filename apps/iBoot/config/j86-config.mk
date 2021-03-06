# Copyright (C) 2012 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
# J86 iboot bootloader build config
TARGET			:=	ipad4b
TARGET_HAS_BASEBAND	:=	1

PINCONFIG_OBJ		:=	pinconfig-nomesa.o

include $(GET_LOCAL_DIR)/ipad4b-config-base.mk
