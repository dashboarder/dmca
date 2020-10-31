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
#ifndef __DITHER_V2_H
#define __DITHER_V2_H

#define	rDITHER_ENABLE		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0000))	//Enable Dither block
#define	rDITHER_METHOD		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0004))	//Main dithering method setting
#define	rDITHER_VERSION		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0008))	//Block revision number
#define	rDITHER_BOOST_POP	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x000c))	//Bit-popping and bit-boost
#define	rDITHER_OUTPUT_WIDTH	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0010))	//Output bit-width setting
#define	rDITHER_ST_OFFSET	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0014))	//Phase offset for Blue/Red wrt green
#define	rDITHER_ST_SEQUENCE	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0018))	//Phase sequence for over 4 frames
#define	rDITHER_ST_KERNEL0	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x001c))	//Kernel0
#define	rDITHER_ST_KERNEL1	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0020))	//Kernel1
#define	rDITHER_ST_KERNEL2	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0024))	//Kernel2
#define	rDITHER_ST_KERNEL3	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0028))	//Kernel3
#define	rDITHER_ST_KERNEL4	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x002c))	//Kernel4
#define	rDITHER_ST_KERNEL5	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0030))	//Kernel5
#define	rDITHER_ST_KERNEL6	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0034))	//Kernel6
#define	rDITHER_ST_KERNEL7	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0038))	//Kernel7
#define	rDITHER_SKEWLOC0	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x003c))	//Skew location0
#define	rDITHER_SKEWLOC1	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0040))	//Skew location1
#define	rDITHER_SKEWLOC2	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0044))	//Skew location2
#define	rDITHER_SKEWLOC3	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0048))	//Skew location3
#define	rDITHER_SKEWLOC4	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x004c))	//Skew location4
#define	rDITHER_SKEWLOC5	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0050))	//Skew location5
#define	rDITHER_SKEWLOC6	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0054))	//Skew location6
#define	rDITHER_SKEWLOC7	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0058))	//Skew location7
#define	rDITHER_SKEWLOC8	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x005c))	//Skew location8
#define	rDITHER_SKEWLOC9	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0060))	//Skew location9
#define	rDITHER_SKEWLOC10	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0064))	//Skew location10
#define	rDITHER_SKEWLOC11	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0068))	//Skew location11
#define	rDITHER_SKEWLOC12	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x006c))	//Skew location12
#define	rDITHER_SKEWLOC13	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0070))	//Skew location13
#define	rDITHER_SKEWLOC14	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0074))	//Skew location14
#define	rDITHER_REG_UPDATE	(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0078))	//Register Update Control
#define	rDITHER_CRC_CON		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x007c))	//CRC Control Register
#define	rDITHER_CRC_R		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0080))	//CRC for R component
#define	rDITHER_CRC_G		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0084))	//CRC for G component
#define	rDITHER_CRC_B		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0088))	//CRC for B component
#define	rDITHER_HACTIVE		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x008c))	//Dither Horizontal Active Width
#define	rDITHER_SPARE		(*(volatile u_int32_t *)(DITHER_BASE_ADDR + 0x0090))	//Spare Register

#endif /* ! __DITHER_V2_H */
