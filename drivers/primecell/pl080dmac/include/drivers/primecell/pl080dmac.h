/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PRIMECELL_PL080DMAC_H
#define __PRIMECELL_PL080DMAC_H

#include <platform/soc/hwdmachannels.h>
#include <platform/soc/hwregbase.h>
#include <sys/task.h>

struct pl080dmac_channel_config {
	uint8_t		channel;		/* dmac channel */
	uint8_t		type;			/* source or destination */
	uint32_t	config;			/* source or destination config register */
	uint32_t	control;		/* source or destination control register */
	uintptr_t	fifo_address;		/* source or destination address */
};

struct pl080dmac_config {
	uint8_t				dmac_clk;
	uint32_t			dmac_tc_irq;
	uint32_t			dmac_err_irq;
	struct pl080dmac_channel_config	dmac_channel_config[PL080DMAC_SUPPORTED_CHANNEL_COUNT];
};

#define PL080DMAC_PERPH_SRC			(0)
#define PL080DMAC_PERPH_DEST			(1)

#define PL080DMAC_FLOW_CTRL_DMAC		(0)
#define PL080DMAC_FLOW_CTRL_SRC_PERPH		(1)
#define PL080DMAC_FLOW_CTRL_DEST_PERPH		(2)

#define PL080DMAC_XFER_WIDTH_BYTE		(0)
#define PL080DMAC_XFER_WIDTH_HALF_WORD		(1)
#define PL080DMAC_XFER_WIDTH_WORD		(2)

#define PL080DMAC_MAX_TRANSFER_SIZE		(0xFA0)		/* Transfer size field in Channel_Config register is 12 bits wide (2^12 - 1). 0xFA0 is just a round up. */

#endif /* __PRIMECELL_PL080DMAC_H */
