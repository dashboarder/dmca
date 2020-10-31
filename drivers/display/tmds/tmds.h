/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __TMDS_H
#define __TMDS_H

/* Definitions for the TI TFP410 */

enum {
	kTFP_ADDR_R	= 0x7f,
	kTFP_ADDR_W	= 0x7e
};

enum {
	kTFP_CTL_1_MODE	= 0x08,
	kTFP_CTL_2_MODE	= 0x09,
	kTFP_CTL_3_MODE	= 0x0a,

	kTFP_DE_DLY	= 0x32,
	kTFP_DE_CTL	= 0x33,
	kTFP_DE_TOP	= 0x34
};

enum {
	kTFP_CTL_1_MODE_nPD	= (1 << 0),
	kTFP_CTL_1_MODE_EDGE	= (1 << 1),
	kTFP_CTL_1_MODE_BSEL	= (1 << 2),
	kTFP_CTL_1_MODE_DSEL	= (1 << 3),
	kTFP_CTL_1_MODE_HEN	= (1 << 4),
	kTFP_CTL_1_MODE_VEN	= (1 << 5),
	kTFP_CTL_1_MODE_TDIS	= (1 << 6)
};

enum {
	kTFP_DE_CTL_DE_DLY8	= (1 << 0),
	kTFP_DE_CTL_HS_POL	= (1 << 4),
	kTFP_DE_CTL_VS_POL	= (1 << 5),
	kTFP_DE_CTL_DE_GEN	= (1 << 6)
};

#endif /* __TMDS_H */
