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
#ifndef __PL080DMAC_CONFIG_H
#define __PL080DMAC_CONFIG_H

#include <drivers/primecell/pl080dmac.h>
#include <platform/soc/hwclocks.h>
#include <platform/soc/hwdmachannels.h>
#include <platform/soc/hwisr.h>

/* LIO Integration spec contains details on PL080DMAC configuration */

static const struct pl080dmac_config pl080dmac_configs[PL080DMAC_COUNT] = {
	{
		CLK_DMARX,
		INT_DMA_RX_TC,
		INT_DMA_RX_ERR,
		{
			/*	channel		type			config reg	control	reg	fifo_address	*/
			{ 	0x0,		PL080DMAC_PERPH_SRC,	0x00001014,	0x0A489000,	0x47380080 	},
		},
	},
	{
		CLK_DMATX,
		INT_DMA_TX_TC,
		INT_DMA_TX_ERR,
		{
			/*	channel		type			config reg	control	reg	fifo_address	*/
			{ 	0x0,		PL080DMAC_PERPH_DEST,	0x00000a80,	0x05489000,	0x47380040 	},
		},
	},
};

#endif /* __PL080DMAC_CONFIG_H */

