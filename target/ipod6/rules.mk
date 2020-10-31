# Copyright (C) 2013 Apple Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#
LOCAL_DIR := $(GET_LOCAL_DIR)
TARGET_DIR := $(LOCAL_DIR)

OPTIONS	+= \
	DIALOG_D2238=1 \
	TARGET_BOOT_CPU_VOLTAGE=CHIPID_CPU_VOLTAGE_MED \
	TARGET_BOOT_RAM_VOLTAGE=0 \
	TARGET_BOOT_SOC_VOLTAGE=0 \
	TARGET_PMU_DEV_ID=0x78 \
	AMC_NUM_CHANNELS=1 \
	AMC_NUM_RANKS=1 \
 	TARGET_DSIM_DPHYCTL=0x00003000 \
 	TARGET_DSIM_CONFIG=0x00400000 \
 	TARGET_DSIM_UP_CODE=0x1 \
 	TARGET_DSIM_DOWN_CODE=0x1 \
 	TARGET_DSIM_SUPPRESS=0x1 \
	TARGET_USES_GP_CM=1 \
 	WITH_TARGET_CHARGETRAP=1 \
 	WITH_DALI=1 \
	WIFI_DTPATH=\"arm-io/sdio\" \
	BT_DTPATH=\"arm-io/uart1/bluetooth\" \
	MULTITOUCH_DTPATH=\"arm-io/multi-touch\"

ifeq ($(CONFIG_SIM),true)
  OPTIONS	+= \
	  CONFIG_SIM=1 \
	  AMP_CALIBRATION_SKIP=1
endif

ifeq ($(CONFIG_FPGA),true)
  OPTIONS	+= \
	  CONFIG_FPGA=1 \
	  DISPLAY_FPGA_TUNABLES=1 \
	  SUPPORT_FPGA=1 \
	  AMP_CALIBRATION_SKIP=1 \
	  WITH_TARGET_AMC_PARAMS=1 \
	  WITH_TARGET_AMP_PARAMS=1
endif

ifeq ($(NO_WFI),true)
OPTIONS	+= \
	NO_ARM_HALT=1
endif

ifeq ($(RECOVERY_MODE_IBSS),true)
OPTIONS += \
	WITH_RECOVERY_MODE_IBSS=1
endif

ifeq ($(PRODUCT),LLB)
# iBoot on M7 is in block 0, so LLB needs to expose the "anc_llb" block device.
OPTIONS += \
	WITH_LLB_BLKDEV=1
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

ALL_OBJS += $(LOCAL_DIR)/init.o \
	    $(LOCAL_DIR)/target_prepare_dali.o

ifeq ($(CONFIG_SIM),true)
  ALL_OBJS +=	$(LOCAL_DIR)/init_sim.o \
		$(LOCAL_DIR)/pinconfig_sim.o
else
  ifeq ($(CONFIG_FPGA),true)
    ALL_OBJS +=	$(LOCAL_DIR)/init_fpga.o \
		$(LOCAL_DIR)/pinconfig_fpga.o
  else

    ALL_OBJS += $(LOCAL_DIR)/init_product.o \
		$(LOCAL_DIR)/pinconfig_product.o \
		$(LOCAL_DIR)/pinconfig_proto2a.o \
		$(LOCAL_DIR)/pinconfig_proto2b.o \
		$(LOCAL_DIR)/pinconfig_evt.o

 	ifneq ($(PRODUCT),LLB)
   		ALL_OBJS += \
   			$(LOCAL_DIR)/properties.o
   	endif
   
  endif
endif

ifeq ($(PRODUCT),iBoot)
# Fast NVRAM driver may be used by iBoot
OPTIONS += \
	WITH_LLB_NVRAM=1 \
	WITH_LLB_BLKDEV=1
endif
