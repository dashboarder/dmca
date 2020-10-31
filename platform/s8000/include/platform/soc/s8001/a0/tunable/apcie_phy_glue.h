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

#ifndef SPDS_S8001_A0_TUNABLE_APCIE_PHY_GLUE_H
#define SPDS_S8001_A0_TUNABLE_APCIE_PHY_GLUE_H

#define PCIE_APCIE_PHY_GLUE_BLK_REFBUF_CFG0_DEFAULT_TUNABLE_UMASK_S8001_A0 (0x318c)
#define PCIE_APCIE_PHY_GLUE_BLK_REFBUF_CFG0_DEFAULT_TUNABLE_VALUE_S8001_A0 (0x2108)

#define PCIE_APCIE_PHY_GLUE_BLK_REFBUF_CFG1_DEFAULT_TUNABLE_UMASK_S8001_A0 (0x318c)
#define PCIE_APCIE_PHY_GLUE_BLK_REFBUF_CFG1_DEFAULT_TUNABLE_VALUE_S8001_A0 (0x2108)

#define PCIE_APCIE_PHY_GLUE_DEFAULT_TUNABLES_S8001_A0 \
/* Key = default_tunable_key                                                           */ \
/* Register Macro Identifier            Register Offset Register Size     Tunable Mask Tunable Value */ \
{  /* APCIE_PHY_GLUE_BLK_REFBUF_CFG0 */ 0x0,            sizeof(uint32_t), 0x318c,      0x2108        }, \
{  /* APCIE_PHY_GLUE_BLK_REFBUF_CFG1 */ 0x4,            sizeof(uint32_t), 0x318c,      0x2108        }, \
{                                       -1,             -1,               -1,          -1            }

#endif /* SPDS_S8001_A0_TUNABLE_APCIE_PHY_GLUE_H */