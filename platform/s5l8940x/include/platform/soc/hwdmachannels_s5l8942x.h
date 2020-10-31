/*
 * Copyright (C) 2009-2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_SOC_HWDMACHANNELS_S5L8942X_H
#define __PLATFORM_SOC_HWDMACHANNELS_S5L8942X_H

#define DMA_MEMORY_TX		(1)
#define DMA_MEMORY_RX		(2)
#define DMA_SDIO		(3)
#define DMA_FMI0DATA		(5)
#define DMA_FMI0CHECK		(6)
#define DMA_FMI1DATA		(7)
#define DMA_FMI1CHECK		(8)

#define DMA_CHANNEL_COUNT	(42)

#define DMA_CHANNEL_CONFIG						\
{									\
	/*	low	high	AES OK	reqID	FIFO address */		\
	{	3,	4,	false,	0x00,	0x30000010	},	\
	{	3,	4,	false,	0x00,	0x30000020	},	\
	{	3,	4,	false,	0x01,	0x301000A0	},	\
	{	3,	4,	false,	0x01,	0x30200020	},	\
	{	5,	8,	true,	0x00,	0x31200014	},	\
	{	5,	8,	true,	0x01,	0x31200018	},	\
 	{	5,	8,	true,	0x02,	0x31300014	},	\
	{	5,	8,	true,	0x03,	0x31300018	},	\
}

#endif /* ! __PLATFORM_SOC_HWDMACHANNELS_S5L8942X_H */
