/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef _APCIE_REGS_H
#define _APCIE_REGS_H

#include <platform/soc/hwregbase.h>
#include <soc/t8010/a0/apcie_config.h>
#include <soc/t8010/a0/apcie_lcount.h>
#include <soc/t8010/a0/x2_pcie_rc.h>

#define APCIE_COUNTER_PORT_STRIDE		(APCIE_LCOUNT_BLK_TIME_L0_1_OFFSET - APCIE_LCOUNT_BLK_TIME_L0_0_OFFSET)
#define APCIE_NUM_COUNTERS			(8)

#define APCIE_PORT_COUNTER_BASE_ADDR(port)	(APCIE_COUNTER_BASE_ADDR + (port) * APCIE_COUNTER_PORT_STRIDE)
#define APCIE_PORT_BASE_ADDR(port)		(APCIE_CONFIG_BASE_ADDR + APCIE_CONFIG_PORT_STRIDE * (port))
#define PCIE_PORT_BASE_ADDR(port)		(PCI_CONFIG_BASE + ((port) << PCI_DEVICE_SHIFT))

#define rAPCIE_COUNTER_VALUE(port, n)		(*(volatile uint32_t *)(APCIE_PORT_COUNTER_BASE_ADDR(port) + (n) * 4))
#define rAPCIE_COUNTER_COMMAND(port)		(*(volatile uint32_t *)(APCIE_PORT_COUNTER_BASE_ADDR(port) + APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_OFFSET))

#define APCIE_COUNTER_ENABLE			APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_ENABLE_UMASK
#define APCIE_COUNTER_CLEAR			APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_CLEAR_UMASK
#define APCIE_COUNTER_CAPTURE			APCIE_LCOUNT_BLK_LCOUNT_COMMAND_0_CAPTURE_UMASK

#define PCIE_RC_CAP_LINK_CONTROL2_OFFSET(link)			(PCIE_PORT_BASE_ADDR(link) + X2_PCIE_RC_X2_PCIE_DSP_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_OFFSET)
#define PCIE_RC_CAP_LINK_CONTROL2_PCIE_TARGET_LINK_SPEED_UMASK	X2_PCIE_RC_X2_PCIE_DSP_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_PCIE_CAP_TARGET_LINK_SPEED_UMASK

#define PCIE_RC_PORT_LOGIC_GEN2_CTRL_OFFSET(link)		(PCIE_PORT_BASE_ADDR(link) + X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_GEN2_CTRL_OFF_OFFSET)
#define PCIE_RC_PORT_LOGIC_GEN2_CTRL_NUM_OF_LANES_SHIFT		X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT
#define PCIE_RC_PORT_LOGIC_GEN2_CTRL_NUM_OF_LANES_UMASK		X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_UMASK

#define PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_OFFSET(link)		(PCIE_PORT_BASE_ADDR(link) + X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_OFFSET)
#define PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_LINK_CAPABLE_SHIFT	X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_SHIFT
#define PCIE_RC_PORT_LOGIC_PORT_LINK_CTRL_LINK_CAPABLE_UMASK	X2_PCIE_RC_X2_PCIE_DSP_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_UMASK

#define PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_OFFSET(link)			(PCIE_PORT_BASE_ADDR(link) + X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_OFFSET)
#define PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_PWR_ON_VALUE_SUPPORT_UMASK	X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_PWR_ON_VALUE_SUPPORT_UMASK
#define PCIE_RC_PORT_LOGIC_L1SUB_CAPABILITY_REG_COMM_MODE_SUPPORT_UMASK		X2_PCIE_RC_X2_PCIE_DSP_PF0_L1SUB_CAP_L1SUB_CAPABILITY_REG_COMM_MODE_SUPPORT_UMASK

#define rAPCIE_PMA_LANE_TX_XCVR_DIAG_FUNC_PWR_CTRL(lane)	(*(volatile uint32_t *)(APCIE_PHY_PMA_TX_LANE_BASE_ADDR + (PMA_TX_LANE_REGISTERS_BLK_XCVR_DIAG_FUNC_PWR_CTRL_OFFSET + ((lane) * APCIE_PHY_PMA_TX_LANE_STRIDE))))
#define rAPCIE_PMA_LANE_TX_XCVR_DIAG_DCYA(lane)			(*(volatile uint32_t *)(APCIE_PHY_PMA_TX_LANE_BASE_ADDR + (PMA_TX_LANE_REGISTERS_BLK_XCVR_DIAG_DCYA_OFFSET + ((lane) * APCIE_PHY_PMA_TX_LANE_STRIDE))))
#define rAPCIE_PMA_LANE_TX_DIAG_TX_BOOST(lane)			(*(volatile uint32_t *)(APCIE_PHY_PMA_TX_LANE_BASE_ADDR + (PMA_TX_LANE_REGISTERS_BLK_TX_DIAG_TX_BOOST_OFFSET + ((lane) * APCIE_PHY_PMA_TX_LANE_STRIDE))))

#define rAPCIE_PMA_LANE_RX_REE_GEN_CTRL_EN(lane)		(*(volatile uint32_t *)(APCIE_PHY_PMA_RX_LANE_BASE_ADDR + (PMA_RX_LANE_REGISTERS_BLK_RX_REE_GEN_CTRL_EN_OFFSET + ((lane) * APCIE_PHY_PMA_RX_LANE_STRIDE))))
#define rAPCIE_PMA_LANE_RX_REE_FREE0_CFGD(lane)			(*(volatile uint32_t *)(APCIE_PHY_PMA_RX_LANE_BASE_ADDR + (PMA_RX_LANE_REGISTERS_BLK_RX_REE_FREE0_CFGD_OFFSET + ((lane) * APCIE_PHY_PMA_RX_LANE_STRIDE))))

#endif /* _APCIE_REGS_H */
