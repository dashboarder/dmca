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
#ifndef __PLATFORM_SOC_HWDMACHANNELS_H
#define __PLATFORM_SOC_HWDMACHANNELS_H

#define DMA_AES_RD				(0)
#define DMA_AES_WR				((1 << DMA_SELECTOR_SHIFT) | (0))

#define DMA_SELECTOR_SHIFT			(4)
#define DMA_SELECTOR_MASK			(0xf)
#define DMA_CHANNEL_MASK			(0xf)

#define PL080DMAC_SUPPORTED_CHANNEL_COUNT	(1)

#endif /* ! __PLATFORM_SOC_HWDMACHANNELS_H */
