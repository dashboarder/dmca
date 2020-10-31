/*
 * Copyright (C) 2010-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __JEDEC_H
#define __JEDEC_H

/* Based on JEDEC LPDDR2 spec */

enum {
        JEDEC_MANUF_ID_RSVD0   = 0,
        JEDEC_MANUF_ID_SAMSUNG,
        JEDEC_MANUF_ID_QIMONDA,
        JEDEC_MANUF_ID_ELPIDA,
        JEDEC_MANUF_ID_ETRON,
        JEDEC_MANUF_ID_NANYA,
        JEDEC_MANUF_ID_HYNIX,
        JEDEC_MANUF_ID_MOSEL,
        JEDEC_MANUF_ID_WINBOND,
        JEDEC_MANUF_ID_ESMT,
        JEDEC_MANUF_ID_RSVD1,
        JEDEC_MANUF_ID_SPANSION,
        JEDEC_MANUF_ID_SST,
        JEDEC_MANUF_ID_ZMOS,
        JEDEC_MANUF_ID_INTEL,
        JEDEC_MANUF_ID_NUMONYX,
        JEDEC_MANUF_ID_MICRON,
        JEDEC_MANUF_ID_RSVD2,
	JEDEC_NUM_MANUF_IDS
};

// LPDDR4 vendor ids
#define JEDEC_LPDDR4_MANUF_ID_SAMSUNG		JEDEC_MANUF_ID_SAMSUNG
#define JEDEC_LPDDR4_MANUF_ID_HYNIX		JEDEC_MANUF_ID_HYNIX
#define JEDEC_LPDDR4_MANUF_ID_MICRON		0xFF
#define JEDEC_LPDDR4_MANUF_ID_ELPIDA		JEDEC_LPDDR4_MANUF_ID_MICRON

enum {
	JEDEC_TYPE_S4_SDRAM = 0,
	JEDEC_TYPE_S2_SDRAM
};

enum {
	JEDEC_IO_WIDTH_32 = 0,
	JEDEC_IO_WIDTH_16,
	JEDEC_IO_WIDTH_8
};

enum {
	JEDEC_DENSITY_64Mb = 0,
	JEDEC_DENSITY_128Mb,
	JEDEC_DENSITY_256Mb,
	JEDEC_DENSITY_512Mb,
	JEDEC_DENSITY_1Gb,
	JEDEC_DENSITY_2Gb,
	JEDEC_DENSITY_4Gb,
	JEDEC_DENSITY_8Gb,
	JEDEC_DENSITY_16Gb,
	JEDEC_DENSITY_32Gb
};

enum {
	JEDEC_LPDDR4_DENSITY_4Gb = 0,
	JEDEC_LPDDR4_DENSITY_6Gb,
	JEDEC_LPDDR4_DENSITY_8Gb,
	JEDEC_LPDDR4_DENSITY_12Gb,
	JEDEC_LPDDR4_DENSITY_16Gb,
	JEDEC_LPDDR4_DENSITY_24Gb,
	JEDEC_LPDDR4_DENSITY_32Gb
};

	
#define JEDEC_MR8_TYPE_SHIFT	(0)
#define JEDEC_MR8_TYPE_MASK	(0x3)
#define JEDEC_MR8_DENSITY_SHIFT	(2)
#define JEDEC_MR8_DENSITY_MASK	(0xF)
#define JEDEC_MR8_WIDTH_SHIFT	(6)
#define JEDEC_MR8_WIDTH_MASK	(0x3)

#endif // __JEDEC_H
