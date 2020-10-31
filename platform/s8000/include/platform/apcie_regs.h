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

#define APCIE_COUNTER_PORT_STRIDE		(0x40)
#define APCIE_NUM_COUNTERS			(8)

#define rAPCIE_COUNTER_VALUE(port, n)		(*(volatile uint32_t *)(APCIE_COUNTER_BASE_ADDR + (port) * APCIE_COUNTER_PORT_STRIDE + (n) * 4))
#define rAPCIE_COUNTER_COMMAND(port)		(*(volatile uint32_t *)(APCIE_COUNTER_BASE_ADDR + (port) * APCIE_COUNTER_PORT_STRIDE + 0x20))

#define APCIE_COUNTER_ENABLE			(1)
#define APCIE_COUNTER_CLEAR			(2)
#define APCIE_COUNTER_CAPTURE			(4)

#define APCIE_PORT_BASE_ADDR(port)		(APCIE_CONFIG_BASE_ADDR + APCIE_CONFIG_PORT_STRIDE * (port))
#define PCIE_PORT_BASE_ADDR(port)		(PCI_CONFIG_BASE + ((port) << PCI_DEVICE_SHIFT))

#define rAPCIE_CONFIG_LINKCFG(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x080))
#define rAPCIE_CONFIG_LINKCTRL(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x084))
#define rAPCIE_CONFIG_LINKSTS(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x088))
#define rAPCIE_CONFIG_PMETO(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x08c))
#define rAPCIE_CONFIG_PMETOACK_TMOTLIM(port)	(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x090))

#define rAPCIE_CONFIG_INBCTRL(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x130))
#define rAPCIE_CONFIG_OUTBCTRL(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x134))
#define rAPCIE_CONFIG_LINKPMGRSTS(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x804))
#define rAPCIE_CONFIG_LINKCDMSTS(port)		(*(volatile uint32_t *)(APCIE_PORT_BASE_ADDR(port) + 0x808))

#define APCIE_CONFIG_LINKCFG_LTSSM_EN		(1 << 0)
#define APCIE_CONFIG_LINKSTS_LINK_STATUS	(1 << 0)
#define APCIE_CONFIG_LINKSTS_CORE_RST_PEND	(1 << 1)
#define APCIE_CONFIG_LINKSTS_PIPE_CLK_ACTIVE	(1 << 2)
#define APCIE_CONFIG_LINKSTS_CLKREQ_N		(1 << 3)
#define APCIE_CONFIG_LINKSTS_L1_STATE		(1 << 4)
#define APCIE_CONFIG_LINKSTS_L1_SUBSTATE	(1 << 5)
#define APCIE_CONFIG_LINKSTS_L2_STATE		(1 << 6)

#define APCIE_CONFIG_INBCTRL_CPL_MUST_PUSH_EN	(1 << 1)
#define APCIE_CONFIG_OUTBCTRL_EARLY_PT_RESP_EN	(1 << 0)

#define rAPCIE_CONFIG_SPACE8(port, offset)	(*(volatile uint8_t *)(PCIE_PORT_BASE_ADDR(port) + (offset)))
#define rAPCIE_CONFIG_SPACE16(port, offset)	(*(volatile uint16_t *)(PCIE_PORT_BASE_ADDR(port) + (offset)))
#define rAPCIE_CONFIG_SPACE32(port, offset)	(*(volatile uint32_t *)(PCIE_PORT_BASE_ADDR(port) + (offset)))

#define rAPCIE_PCIE_CAP_LINK_CONTROL2(port)	rAPCIE_CONFIG_SPACE16(port, 0xa0)
#define rAPCIE_PORT_LOGIC_PORT_LINK_CTRL(port)	rAPCIE_CONFIG_SPACE32(port, 0x710)
#define rAPCIE_PORT_LOGIC_GEN2_CTRL(port)	rAPCIE_CONFIG_SPACE32(port, 0x80c)

#endif /* _APCIE_REGS_H */
