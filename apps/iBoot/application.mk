# Copyright (C) 2007-2014 Apple Inc. All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

#
# Global headers that are independing of product, target, etc.
#
GLOBAL_HEADERS += \
	lib/mib/mib_nodes.h

#
# iBoot application code.
#
ALL_OBJS += \
	$(LOCAL_DIR)/boot.o \
	$(LOCAL_DIR)/main.o \
	$(LOCAL_DIR)/menu_commands.o \
	$(LOCAL_DIR)/debugcmds.o \
	$(LOCAL_DIR)/default_env.o \
	$(LOCAL_DIR)/upgrade.o

#
# Target module.
#
MODULES_BASIC += \
	target/$(TARGET)

OPTIONS += \
	WITH_BOOT_STAGE=1 \
	WITH_TARGET_CONFIG=1 \
	IMAGE_MAX_COUNT=32 \
	WITH_APPLICATION_PUTCHAR=1

ifeq ($(BUILD),RELEASE)
OPTIONS += \
	TERSE_PANIC_STRINGS=1
endif

#
# Essential system modules.
#
MODULES_BASIC += \
	lib/heap \
	lib/nonce \
	lib/random \
	lib/paint \
	sys

ifeq ($(IMAGE_FORMAT),)
$(error "IMAGE_FORMAT not set")
endif
ifeq ($(IMAGE_FORMAT),img3)
MODULES += \
        lib/image/image3 \
	lib/ticket
endif
ifeq ($(IMAGE_FORMAT),im4p)
MODULES += \
        lib/image/image4
endif

#
# Libraries to link with.
#
LIBRARY_MODULES += \
	lib/libc \
	lib/mib

#
# Uncomment to enable boot time profiling. Shared memory console isn't compatible
# with boot profiling system, so it has to be ommitted when profiling
#
#MODULES_BASIC += \
#	lib/profile
#MODULES_ELIDE += \
#	drivers/apple/shmcon

################################################################################
#
# Define option and module lists for basic differentiating features.
#
# Note that these lists are not intended to be overridden by target or platform.
#
################################################################################

#
# Interactive console support.
#
OPTIONS_CONSOLE += \
        WITH_MENU=1

MODULES_CONSOLE += \
	lib/env

#
# Interactive console in release builds get the dumb menu.
#
ifeq ($(BUILD),RELEASE)
OPTIONS_CONSOLE += \
	WITH_SIMPLE_MENU=1
endif

#
# Functional tests and related commands will be added to console only in debug
# builds.
#
ifeq ($(BUILD),DEBUG)
endif

#
# This option forces the restore host to bootstrap into restore OS via iBEC
# rather than immediately from current product.
#
OPTIONS_RESTORE_STRAP += \
	WITH_RESTORE_STRAP=1


# iBEC is willing to tolerate any NAND epoch, as it may be used during
# restore to boot upgrade bits from an older install's storage
# device.
#
# XXX this needs to be a runtime option as this code is now in the library.
#
OPTIONS_RESTORE_BOOT += \
	WITH_RESTORE_BOOT=1 \
	AND_DISABLE_EPOCH_CHECK=1


################################################################################
#
# Define option and module lists for DFU-mode versus recovery mode.
#
# Note that only one of these should ever be included in any particular product.
# Also, these lists are not intended to be overridden by target or platform.
#
################################################################################

#
# DFU mode products do USB DFU rather than recovery mode.
#
OPTIONS_DFU += \
	WITH_DFU_MODE=1

MODULES_DFU +=

#
# Recovery mode products get additional logic, including console support
# since it is required for recovery mode to function properly.
#
OPTIONS_RECOVERY += \
	$(OPTIONS_CONSOLE) \
	WITH_RECOVERY_MODE=1

MODULES_RECOVERY += \
	$(MODULES_CONSOLE)


################################################################################
#
# Define option and module lists for various feature sets.  
#
# It is expected that these lists are to be decorated with platform
# and/or target-specific support reflecting differences in hardware
# required to support each feature.
#
################################################################################

#
# Display drivers support.
#
MODULES_DISPLAY +=

#
# Firmware image storage support.
#
MODULES_FIRMWARE += \
	lib/blockdev

#
# NVRAM storage support.
#
MODULES_NVRAM += \
	lib/nvram

#
# RAMDISK support.
#
MODULES_RAMDISK += \
	lib/ramdisk

#
# Syscfg storage support.
#
MODULES_SYSCFG += \
	lib/syscfg

#
# Filesystem support.
#
MODULES_FILESYSTEM += \
	lib/fs/hfs

OPTIONS_FILESYSTEM += \
	WITH_MASS_STORAGE=1


################################################################################
#
# Define option and modules lists for composition of features required to
# boot XNU.
#
# This really shouldn't be overridden in any platform or target makefiles
#
################################################################################

#
# Products that boot the kernel get some extra options.
#
OPTIONS_BOOT += \
	WITH_BOOT_XNU=1

#
# Core technology-neutral software modules required to support booting XNU.
#
MODULES_BOOT += \
	lib/devicetree \
	lib/macho \
	lib/paniclog

#
# The following feature modules list are also needed to boot XNU.
#
MODULES_BOOT +=	\
	$(MODULES_FIRMWARE) \
	$(MODULES_NVRAM) \
	$(MODULES_RAMDISK) \
	$(MODULES_SYSCFG)
