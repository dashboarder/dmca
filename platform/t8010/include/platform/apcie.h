/*                                                                              
 * Copyright (c) 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef PLATFORM_APCIE_H
#define PLATFORM_APCIE_H

#define APCIE_LANE_CFG_X1_X1_X1_X1	(0)
#define APCIE_LANE_CFG_X2_X1_X1		(1)
#define APCIE_LANE_CFG_X1_X1_X2		(2)
#define APCIE_LANE_CFG_X2_X2		(3)

#define APCIE_LANE_CONFIG_NVME0_X1	APCIE_LANE_CFG_X1_X1_X1_X1
#define APCIE_LANE_CONFIG_NVME0_X2	APCIE_LANE_CFG_X2_X1_X1

#endif
