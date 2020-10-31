/* 
 * Automatically generated by socgen. DO NOT EDIT!
 * 
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 * 
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 * 
 * This document may not be reproduced or transmitted in any form
 * in whole or in part, without the express written permission of
 * Apple inc.
 */

#ifndef SPDS_S8000_B0_TUNABLE_X2_PCIE_RC_H
#define SPDS_S8000_B0_TUNABLE_X2_PCIE_RC_H

#define PCIE_PCIE_CONFIG_PCIE_RC0_X2_PCIE_DSP_PF0_PCIE_CAP_DEVICE_CONTROL2_DEVICE_STATUS2_REG_DEFAULT_TUNABLE_UMASK_S8000_B0 (0xf)
#define PCIE_PCIE_CONFIG_PCIE_RC0_X2_PCIE_DSP_PF0_PCIE_CAP_DEVICE_CONTROL2_DEVICE_STATUS2_REG_DEFAULT_TUNABLE_VALUE_S8000_B0 (0x0)

#define PCIE_PCIE_CONFIG_PCIE_RC0_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_FPGA_TUNABLE_UMASK_S8000_B0                 (0x1f)
#define PCIE_PCIE_CONFIG_PCIE_RC0_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_FPGA_TUNABLE_VALUE_S8000_B0                 (0x0)

#define PCIE_PCIE_CONFIG_PCIE_RC0_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_DEFAULT_TUNABLE_UMASK_S8000_B0              (0xf8ff00)
#define PCIE_PCIE_CONFIG_PCIE_RC0_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_DEFAULT_TUNABLE_VALUE_S8000_B0              (0x1900)

#define PCIE_PCIE_CONFIG_PCIE_RC0_DEFAULT_TUNABLES_S8000_B0 \
/* Key = default_tunable_key                                                                                                   */ \
/* Register Macro Identifier                                                    Register Offset Register Size     Tunable Mask Tunable Value */ \
{  /* X2_PCIE_RC_X2_PCIE_DSP_PF0_PCIE_CAP_DEVICE_CONTROL2_DEVICE_STATUS2_REG */ 0x98,           sizeof(uint32_t), 0xf,         0x0           }, \
{  /* X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG */              0x154,          sizeof(uint32_t), 0xf8ff00,    0x1900        }, \
{                                                                               -1,             -1,               -1,          -1            }

#define PCIE_PCIE_CONFIG_PCIE_RC0_FPGA_TUNABLES_S8000_B0 \
/* Key = fpga_tunable_key                                                                                                      */ \
/* Register Macro Identifier                                                    Register Offset Register Size     Tunable Mask Tunable Value */ \
{  /* X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG */              0x154,          sizeof(uint32_t), 0x1f,        0x0           }, \
{                                                                               -1,             -1,               -1,          -1            }

#define PCIE_PCIE_CONFIG_PCIE_RC2_X2_PCIE_DSP_PF0_PCIE_CAP_DEVICE_CONTROL2_DEVICE_STATUS2_REG_DEFAULT_TUNABLE_UMASK_S8000_B0 (0xf)
#define PCIE_PCIE_CONFIG_PCIE_RC2_X2_PCIE_DSP_PF0_PCIE_CAP_DEVICE_CONTROL2_DEVICE_STATUS2_REG_DEFAULT_TUNABLE_VALUE_S8000_B0 (0x0)

#define PCIE_PCIE_CONFIG_PCIE_RC2_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_FPGA_TUNABLE_UMASK_S8000_B0                 (0x1f)
#define PCIE_PCIE_CONFIG_PCIE_RC2_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_FPGA_TUNABLE_VALUE_S8000_B0                 (0x0)

#define PCIE_PCIE_CONFIG_PCIE_RC2_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_DEFAULT_TUNABLE_UMASK_S8000_B0              (0xf8ff00)
#define PCIE_PCIE_CONFIG_PCIE_RC2_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_DEFAULT_TUNABLE_VALUE_S8000_B0              (0x1900)

#define PCIE_PCIE_CONFIG_PCIE_RC2_DEFAULT_TUNABLES_S8000_B0 \
/* Key = default_tunable_key                                                                                                   */ \
/* Register Macro Identifier                                                    Register Offset Register Size     Tunable Mask Tunable Value */ \
{  /* X2_PCIE_RC_X2_PCIE_DSP_PF0_PCIE_CAP_DEVICE_CONTROL2_DEVICE_STATUS2_REG */ 0x98,           sizeof(uint32_t), 0xf,         0x0           }, \
{  /* X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG */              0x154,          sizeof(uint32_t), 0xf8ff00,    0x1900        }, \
{                                                                               -1,             -1,               -1,          -1            }

#define PCIE_PCIE_CONFIG_PCIE_RC2_FPGA_TUNABLES_S8000_B0 \
/* Key = fpga_tunable_key                                                                                                      */ \
/* Register Macro Identifier                                                    Register Offset Register Size     Tunable Mask Tunable Value */ \
{  /* X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG */              0x154,          sizeof(uint32_t), 0x1f,        0x0           }, \
{                                                                               -1,             -1,               -1,          -1            }

#endif /* SPDS_S8000_B0_TUNABLE_X2_PCIE_RC_H */