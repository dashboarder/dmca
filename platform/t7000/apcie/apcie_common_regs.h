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
#ifndef _APCIE_COMMON_REGS_H
#define _APCIE_COMMON_REGS_H

#include <platform/soc/hwregbase.h>

#define rAPCIE_PHY_RST_CTRL(n)			(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x040 + (n) * 0x010))
#define rAPCIE_PHY_CLKREQ_CTRL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x044 + (n) * 0x010))

#define APCIE_PHY_RST_CTRL_phy_sw_reset		(1 << 0)

#define rAPCIE_NANDSYSCLK_CTRL			(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x080))

#define rAPCIE_PORT_CTRL(n)			(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x100 + (n) * 0x80))
#define rAPCIE_PORT_SQUELCH_CTRL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x104 + (n) * 0x80))
#define rAPCIE_PORT_REFCLK_CTRL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x108 + (n) * 0x80))
#define rAPCIE_PORT_OVERRIDE_CTRL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x10c + (n) * 0x80))
#define rAPCIE_PORT_CLKREQ_CTRL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x110 + (n) * 0x80))
#define rAPCIE_PORT_ACG_IDLE_CTRL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x114 + (n) * 0x80))
#define rAPCIE_PORT_RST_CTRL(n)			(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x118 + (n) * 0x80))
#define rAPCIE_PIPE_OVERRIDE_VAL(n)		(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x11c + (n) * 0x80))
#define rAPCIE_PORT_PRIORITY(n)			(*(volatile uint32_t *)(APCIE_COMMON_BASE_ADDR + 0x120 + (n) * 0x80))

#define APCIE_PORT_CTRL_port_en			(1 << 0)
#define APCIE_PORT_CTRL_appclk_auto_dis		(1 << 8)
#define APCIE_PORT_CTRL_nvme_en			(1 << 16)
#define APCIE_PORT_CTRL_muxed_auxclk_auto_dis	(1 << 20)

#define APCIE_PORT_REFCLK_CTRL_refclk_en	(1 << 0)
#define APCIE_PORT_REFCLK_CTRL_refclk_auto_dis	(1 << 8)

#define APCIE_PORT_OVERRIDE_CTRL_pipe_ovr_en	(1 << 0)

#define APCIE_PORT_RST_CTRL_perst_n		(1 << 0)

#define APCIE_PHY_BASE_ADDR(port)		(PCI_REG_BASE + 0x1000000 * ((port) + 1) + 0x5000)

#define rAPCIE_PHY_PHY_REF_USE_PAD(port)	(*(volatile uint32_t *)(APCIE_PHY_BASE_ADDR(port) + 0xc3c))

#endif

