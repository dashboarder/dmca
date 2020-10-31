/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <platform.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>

bool chipid_get_production_mode(void)
{
	while (!chipid_get_read_done());

	return ((rCHIPIDL >> 0) & 1) != 0;
}

bool chipid_get_secure_mode(void)
{
	while (!chipid_get_read_done());

	return ((rCHIPIDH >> 1) & 1) != 0;
}

u_int32_t chipid_get_security_domain(void)
{
	while (!chipid_get_read_done());

#if SUPPORT_FPGA
	return kPlatformSecurityDomainDarwin;
#else
	return (rCHIPIDH >> 1) & 3;
#endif
}

u_int32_t chipid_get_minimum_epoch(void)
{
	while (!chipid_get_read_done());

	return (rCHIPIDH >> 3) & 0x7F;
}

bool chipid_get_ecid_image_personalization_required(void)
{
	return ((rCHIPIDH >> 14) & 1) != 0;
}

u_int32_t chipid_get_board_id(void)
{
	while (!chipid_get_read_done());

	return (rCHIPIDH >> 10) & 3;
}

u_int32_t chipid_get_chip_id(void)
{
	while (!chipid_get_read_done());

	return (rCHIPIDH >> 16) & 0xFFFF;
}

u_int32_t chipid_get_chip_revision(void)
{
	while (!chipid_get_read_done());

	return (((rCHIPIDL >> 24) & 0xF) << 4) | (((rCHIPIDL >> 28) & 0xF) << 0);
}

u_int32_t chipid_get_osc_frequency(void)
{
	return OSC_FREQ;
}

u_int64_t chipid_get_ecid_id(void)
{
	u_int64_t ecid = 0;

	while (!chipid_get_read_done());

#if SUPPORT_FPGA
	ecid = 0x000012345678ABCDULL;
#else
	/*
	 * For the sake of the next poor bastard that comes along and has
	 * to decode the ECID produced by this silliness, the code here
	 * produces output formatted thus:
	 *
	 * 6              4               3               1
	 * 3              8               2               6               0
	 * ......................lllllllllllllllllllllwwwwwyyyyyyyyxxxxxxxx
	 *
	 * l - lot number
	 * w - wafer number
	 * y - wafer y position
	 * x - wafer x position
	 */
	ecid |= ((rDIEIDL >>  0)) & ((1ULL << (21 -  0)) - 1);	// LOT_ID

	ecid <<= (26 - 21);
	ecid |= ((rDIEIDL >> 21)) & ((1ULL << (26 - 21)) - 1);	// WAFER_NUM

	ecid <<= (10 -  2);
	ecid |= ((rDIEIDH >>  2)) & ((1ULL << (10 -  2)) - 1);	// Y_POS

	ecid <<= (32 - 26);
	ecid |= ((rDIEIDL >> 26)) & ((1ULL << (32 - 26)) - 1);	// X_POS_H

	ecid <<= ( 2 -  0);
	ecid |= ((rDIEIDH >>  0)) & ((1ULL << ( 2 -  0)) - 1);	// X_POS_L
#endif

	return ecid;
}

u_int64_t chipid_get_die_id(void)
{
	while (!chipid_get_read_done());

	return ((u_int64_t)rDIEIDH << 32) | rDIEIDL;
}

//  chipid_get_tv_dac_cals
//    Saves the three tv dac calibration values in the given array.
//
void chipid_get_tv_dac_cals(u_int32_t *dac_cals)
{
	while (!chipid_get_read_done());

	dac_cals[0] = (rCHIPIDL >>  9) & 0x1F;
	dac_cals[1] = (rCHIPIDL >> 14) & 0x1F;
	dac_cals[2] = (rCHIPIDL >> 19) & 0x1F;
}

bool chipid_get_read_done(void)
{
#if !SUPPORT_FPGA
	return ((rEFUSE_READ_DONE >> 0) & 1) != 0;
#else
	return true;
#endif
}

