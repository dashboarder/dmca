/*
 * Copyright (C) 2010, 2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __TARGET_DISPLAY_GAMMA_TABLES_H
#define __TARGET_DISPLAY_GAMMA_TABLES_H

/* K9x display gamma tables */

static const display_gamma_table k93a_display_gamma_tables[] =
{
	{
		0x00960C49, 0x00FFFF7F, // LGD P5 table V0
		{
			0x62, 0xca, 0x1d, 0xd0, 0x1d, 0x00, 0x00, 0x0d,
			0x0d, 0x00, 0xc0, 0x1d, 0x77, 0xd3, 0x1d, 0x1c,
			0xc7, 0x71, 0x1c, 0x00, 0x70, 0x00, 0x00, 0x00,
			0x00, 0x00, 0xd0, 0x40, 0x03, 0x0d, 0x34, 0x34,
			0x0d, 0x0d, 0x00, 0x1c, 0x77, 0xdc, 0x71, 0x1c,
			0xc7, 0x71, 0xc7, 0xc1, 0x01, 0x70, 0xc0, 0x01,
			0x07, 0xc0, 0x01, 0x40, 0x00, 0x1c, 0x00, 0x1c,
			0x1c, 0xc7, 0xdd, 0x4d, 0xd3, 0x4d, 0x00, 0x00,
			0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x04, 0x40, 0xc7, 0x01, 0xd0, 0xd0,
			0x00, 0x00, 0x70, 0x77, 0x37, 0xdd, 0x74, 0xdc,
			0x71, 0xdc, 0x1d, 0x07, 0x00, 0x1c, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xd0, 0xd0, 0x40, 0x03, 0x0d,
			0x4d, 0x43, 0x03, 0x1c, 0xc7, 0x1d, 0x77, 0x1c,
			0x07, 0xc7, 0xc1, 0x71, 0x00, 0x70, 0x00, 0x70,
			0x00, 0x00, 0x1c, 0xd0, 0x01, 0x00, 0x00, 0xc0,
			0x01, 0x07, 0x77, 0x77, 0x77, 0x13, 0x07, 0x00,
			0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x04, 0x40, 0x77, 0x00, 0x40, 0x43,
			0x03, 0x0d, 0x07, 0x07, 0xc7, 0x1d, 0x07, 0x07,
			0x70, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0,
			0x00, 0x0d, 0xd0, 0x34, 0x34, 0x0d, 0x4d, 0xd3,
			0x0d, 0x0d, 0xc0, 0x71, 0x1c, 0xc7, 0x71, 0x70,
			0x70, 0x1c, 0x1c, 0x00, 0x07, 0xc0, 0x01, 0x70,
			0x00, 0x1c, 0x00, 0x01, 0x1c, 0x00, 0x1c, 0x77,
			0xdc, 0x4d, 0x03, 0x0d, 0x00, 0x34, 0x4d, 0xd3,
			0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
	{
		0x00950C49, 0x00FFFF7F, // LGD P6 table V0 (same as P5)
		{
			0x62, 0xca, 0x1d, 0xd0, 0x1d, 0x00, 0x00, 0x0d,
			0x0d, 0x00, 0xc0, 0x1d, 0x77, 0xd3, 0x1d, 0x1c,
			0xc7, 0x71, 0x1c, 0x00, 0x70, 0x00, 0x00, 0x00,
			0x00, 0x00, 0xd0, 0x40, 0x03, 0x0d, 0x34, 0x34,
			0x0d, 0x0d, 0x00, 0x1c, 0x77, 0xdc, 0x71, 0x1c,
			0xc7, 0x71, 0xc7, 0xc1, 0x01, 0x70, 0xc0, 0x01,
			0x07, 0xc0, 0x01, 0x40, 0x00, 0x1c, 0x00, 0x1c,
			0x1c, 0xc7, 0xdd, 0x4d, 0xd3, 0x4d, 0x00, 0x00,
			0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x04, 0x40, 0xc7, 0x01, 0xd0, 0xd0,
			0x00, 0x00, 0x70, 0x77, 0x37, 0xdd, 0x74, 0xdc,
			0x71, 0xdc, 0x1d, 0x07, 0x00, 0x1c, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xd0, 0xd0, 0x40, 0x03, 0x0d,
			0x4d, 0x43, 0x03, 0x1c, 0xc7, 0x1d, 0x77, 0x1c,
			0x07, 0xc7, 0xc1, 0x71, 0x00, 0x70, 0x00, 0x70,
			0x00, 0x00, 0x1c, 0xd0, 0x01, 0x00, 0x00, 0xc0,
			0x01, 0x07, 0x77, 0x77, 0x77, 0x13, 0x07, 0x00,
			0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x04, 0x40, 0x77, 0x00, 0x40, 0x43,
			0x03, 0x0d, 0x07, 0x07, 0xc7, 0x1d, 0x07, 0x07,
			0x70, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0,
			0x00, 0x0d, 0xd0, 0x34, 0x34, 0x0d, 0x4d, 0xd3,
			0x0d, 0x0d, 0xc0, 0x71, 0x1c, 0xc7, 0x71, 0x70,
			0x70, 0x1c, 0x1c, 0x00, 0x07, 0xc0, 0x01, 0x70,
			0x00, 0x1c, 0x00, 0x01, 0x1c, 0x00, 0x1c, 0x77,
			0xdc, 0x4d, 0x03, 0x0d, 0x00, 0x34, 0x4d, 0xd3,
			0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
	{
		0x00970C49, 0x00FFFF7F, // LGD P3 table V0
		{
			0x62, 0xca, 0x04, 0x00, 0x71, 0x00, 0x40, 0x03,
			0x0d, 0x00, 0x70, 0xdc, 0xdd, 0xdd, 0x1d, 0x07,
			0x07, 0xc7, 0x01, 0xd7, 0x00, 0x00, 0x00, 0xd0,
			0xd0, 0x40, 0x43, 0x37, 0xdd, 0x34, 0xdd, 0x74,
			0x37, 0x0d, 0x00, 0xc7, 0x1d, 0x77, 0x1c, 0x1c,
			0x1c, 0xc7, 0xc1, 0x01, 0x70, 0xc0, 0x01, 0x07,
			0xc0, 0x01, 0x47, 0x07, 0x00, 0x1c, 0xc0, 0x1d,
			0xc7, 0xdd, 0x40, 0x03, 0xd0, 0x40, 0x77, 0x77,
			0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x74, 0xd0, 0x04, 0x07, 0x00, 0x0d,
			0x00, 0x00, 0x70, 0x77, 0x77, 0xd3, 0xdd, 0x71,
			0x70, 0x1c, 0xc7, 0x01, 0x00, 0x00, 0x00, 0x40,
			0x03, 0x0d, 0x34, 0x74, 0xd3, 0x34, 0xdd, 0x74,
			0x77, 0xd3, 0x00, 0xc7, 0x71, 0xc7, 0x71, 0x70,
			0x70, 0x1c, 0x1c, 0x70, 0x00, 0x70, 0x00, 0x70,
			0x00, 0x70, 0x00, 0x04, 0x00, 0x00, 0x70, 0x70,
			0x1c, 0x77, 0x37, 0x34, 0xd0, 0xd0, 0xdd, 0xdd,
			0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x74, 0x0d, 0x1d, 0x07, 0xd0, 0x40,
			0x03, 0x00, 0x00, 0x07, 0x1c, 0xc7, 0x01, 0xc0,
			0x01, 0xc0, 0x01, 0x0d, 0x34, 0x00, 0x34, 0x4d,
			0x37, 0x4d, 0xd3, 0xdd, 0xdd, 0xdd, 0x1d, 0x77,
			0x37, 0x7d, 0x70, 0x1c, 0x1c, 0xc7, 0x01, 0x07,
			0x1c, 0x1c, 0x00, 0x1c, 0xc0, 0x01, 0x1c, 0xc0,
			0x01, 0x1c, 0x1d, 0x00, 0x70, 0x70, 0xdc, 0x4d,
			0x37, 0x00, 0x00, 0x1c, 0x00, 0x1c, 0x00, 0x1c,
			0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
	{
		0x00E40C49, 0x00FFFF7F, // Samsung 6mask table V0
		{
			0x62, 0xca, 0x74, 0xd0, 0x04, 0x07, 0x00, 0x0d,
			0xd0, 0x00, 0x1c, 0x1c, 0x1c, 0x77, 0x70, 0x00,
			0x00, 0x00, 0x00, 0x4d, 0x43, 0x43, 0x43, 0xd3,
			0x40, 0x03, 0x0d, 0x34, 0x00, 0x0d, 0x00, 0x34,
			0x00, 0x00, 0x34, 0x00, 0x34, 0x00, 0x1c, 0x70,
			0x70, 0x1c, 0x1c, 0x07, 0xc7, 0x71, 0x77, 0x77,
			0xd3, 0x74, 0xd3, 0x01, 0x00, 0x00, 0x40, 0x43,
			0x77, 0x77, 0x77, 0x07, 0xc7, 0xc1, 0x41, 0x43,
			0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x04, 0x00, 0x1d, 0xb0, 0x32, 0x00,
			0x0d, 0x40, 0x1f, 0x70, 0xdc, 0x71, 0xc7, 0x01,
			0x1c, 0x00, 0x07, 0x40, 0x43, 0x03, 0x34, 0x40,
			0x03, 0x0d, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x70,
			0x70, 0x1c, 0xc7, 0x71, 0x1c, 0xc7, 0x71, 0x77,
			0x77, 0xd3, 0x4d, 0x77, 0x07, 0x00, 0x00, 0xd0,
			0x74, 0x77, 0x1c, 0x07, 0x1c, 0x00, 0xd0, 0xdd,
			0x71, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x74, 0x00, 0xdd, 0x01, 0x00, 0x00,
			0x34, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x34,
			0x40, 0x03, 0x74, 0x37, 0xdd, 0x74, 0x37, 0xdd,
			0x34, 0x4d, 0xd3, 0xd0, 0x40, 0x43, 0x03, 0x0d,
			0xd0, 0x40, 0x03, 0x0d, 0x34, 0x1c, 0xc0, 0x01,
			0x07, 0x1c, 0x1c, 0x1c, 0x1c, 0x77, 0x37, 0xdd,
			0x4d, 0x77, 0x07, 0x00, 0x00, 0x00, 0x40, 0xd3,
			0x34, 0x34, 0xdd, 0x34, 0x4d, 0xd3, 0x34, 0xdd,
			0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
	{
		0x00E30C49, 0x00FFFF7F, // Samsung 5mask table V0
		{
			0x22, 0xce, 0x4d, 0x07, 0xd0, 0x74, 0x1c, 0x1c,
			0x1c, 0x1c, 0xd3, 0xdd, 0xdd, 0x34, 0xdd, 0x74,
			0x07, 0x1c, 0x1c, 0x1c, 0x00, 0x0d, 0xd0, 0x00,
			0x34, 0x0d, 0x0d, 0x34, 0xd0, 0x40, 0x03, 0x34,
			0xd0, 0x40, 0x03, 0x34, 0x34, 0x0d, 0x0d, 0x00,
			0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0xc0, 0x71,
			0x1c, 0x77, 0xc7, 0x1d, 0x00, 0xd0, 0x00, 0x4d,
			0x07, 0x07, 0x1c, 0x00, 0x00, 0x00, 0xd0, 0x1d,
			0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x22, 0xce, 0x4d, 0x07, 0x40, 0x77, 0x07, 0x07,
			0xc7, 0x71, 0x37, 0xdd, 0x74, 0xd3, 0x34, 0x74,
			0xc7, 0xc1, 0x71, 0x07, 0x07, 0x0d, 0x00, 0x40,
			0x03, 0x34, 0x40, 0x03, 0xd0, 0x00, 0x0d, 0x00,
			0x0d, 0xd0, 0x00, 0x34, 0xd0, 0xd0, 0x40, 0x03,
			0x00, 0x07, 0xc0, 0x01, 0x00, 0x1c, 0x00, 0x07,
			0xc7, 0x1d, 0x77, 0xdc, 0x41, 0x43, 0x43, 0xd3,
			0x71, 0x00, 0x1c, 0x00, 0x0d, 0xd0, 0xc4, 0x01,
			0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x22, 0xce, 0x0d, 0x01, 0x40, 0x13, 0x77, 0x70,
			0xc0, 0xc1, 0x1d, 0xc7, 0x71, 0xc7, 0xdd, 0x71,
			0x00, 0x00, 0x07, 0x00, 0x4d, 0xd3, 0xd0, 0x34,
			0x4d, 0x37, 0x4d, 0xd3, 0x34, 0x34, 0x34, 0x4d,
			0x43, 0xd3, 0xd0, 0x74, 0xd3, 0x34, 0x00, 0x00,
			0x00, 0x70, 0x00, 0x00, 0x00, 0x07, 0x70, 0xdc,
			0x1d, 0x77, 0x07, 0x00, 0x00, 0x0d, 0xd0, 0x74,
			0xdc, 0xdd, 0x1d, 0x77, 0xdc, 0x71, 0x1c, 0x77,
			0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
	{
		0x00890C49, 0x00FFFF7F, // CMI T1 table V0
		{
			0x62, 0xca, 0x01, 0x74, 0x00, 0xd0, 0xdd, 0x1d,
			0x77, 0x4c, 0x43, 0x03, 0x40, 0xd3, 0xdd, 0xdd,
			0x1d, 0xc7, 0xc1, 0x1d, 0xc7, 0x71, 0x77, 0x77,
			0x77, 0xd3, 0x34, 0x0d, 0x0d, 0x0d, 0x34, 0xd0,
			0x00, 0x34, 0x00, 0x70, 0x70, 0xc7, 0x71, 0xdc,
			0x1d, 0x77, 0x77, 0x77, 0x43, 0x03, 0xd0, 0x00,
			0xd0, 0x74, 0x1c, 0x77, 0xdc, 0xc1, 0x01, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x40, 0x03, 0x4d, 0xd3,
			0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x01, 0x74, 0x00, 0xd0, 0x74, 0xdc,
			0x71, 0xd3, 0x34, 0x00, 0xd0, 0xd0, 0xdd, 0xdd,
			0xdd, 0x1d, 0x1c, 0xc7, 0x1d, 0xc7, 0xdd, 0xdd,
			0xdd, 0x74, 0x43, 0xd3, 0x34, 0x34, 0x34, 0x40,
			0x03, 0x34, 0x00, 0xc0, 0x71, 0x1c, 0xc7, 0x71,
			0xc7, 0xdd, 0x1d, 0x37, 0x0d, 0x34, 0x40, 0x03,
			0xd0, 0x74, 0x1c, 0x77, 0x1c, 0x07, 0x00, 0x07,
			0x00, 0x00, 0x00, 0x34, 0x00, 0x40, 0xd3, 0x34,
			0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0xca, 0x01, 0x74, 0x00, 0xd0, 0x74, 0xdc,
			0x71, 0x77, 0xd3, 0xd0, 0xd0, 0x74, 0xdc, 0x1d,
			0x77, 0x70, 0x70, 0x70, 0x1c, 0x1c, 0x77, 0xdc,
			0xdd, 0x4d, 0x37, 0x4d, 0xd3, 0x34, 0x34, 0x0d,
			0x0d, 0x34, 0x00, 0x70, 0x1c, 0x07, 0xc7, 0x1d,
			0xc7, 0x1d, 0x77, 0x37, 0x34, 0xd0, 0x00, 0x0d,
			0x74, 0xc7, 0x71, 0xc7, 0x71, 0x00, 0x07, 0x00,
			0x1c, 0x00, 0x70, 0x00, 0x1c, 0x00, 0xc0, 0x01,
			0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
	{
		0x00900C49, 0x00FFFF7F, // CMI T1x table V0
		{
			0x26, 0x11, 0x07, 0xd0, 0x1d, 0x00, 0x00, 0xdd,
			0xdd, 0xc1, 0x01, 0x07, 0x1c, 0x1c, 0xc7, 0xdd,
			0x34, 0x34, 0xd0, 0x00, 0x0d, 0x34, 0x00, 0x0d,
			0x00, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00,
			0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x1c,
			0x00, 0x00, 0x00, 0x70, 0xc0, 0xc1, 0xc1, 0xc1,
			0xc1, 0x71, 0x1c, 0x77, 0xdc, 0x71, 0x1c, 0x07,
			0xc7, 0x71, 0x70, 0x1c, 0x1c, 0x40, 0x1c, 0x70,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x26, 0xd1, 0x1d, 0xd0, 0x74, 0x1c, 0x00, 0x34,
			0x4d, 0xc7, 0x1d, 0xc7, 0x1d, 0x77, 0x77, 0xd3,
			0xd0, 0x00, 0x40, 0x03, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x07, 0x70,
			0xc0, 0x01, 0x00, 0x07, 0x70, 0x00, 0x07, 0xc0,
			0x01, 0x1c, 0xc0, 0x01, 0x07, 0x1c, 0xc7, 0x71,
			0x1c, 0x77, 0xdc, 0x1d, 0x77, 0x77, 0x77, 0xc7,
			0xdd, 0x71, 0x77, 0xdc, 0x71, 0xc7, 0x1d, 0x00,
			0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x26, 0x41, 0x77, 0x00, 0x74, 0x77, 0x00, 0x00,
			0x40, 0xd3, 0x4d, 0x77, 0x37, 0xdd, 0x34, 0x0d,
			0x40, 0x03, 0x00, 0x00, 0x00, 0x1c, 0x00, 0xc0,
			0x01, 0x70, 0x00, 0x1c, 0x70, 0xc0, 0xc1, 0x71,
			0x1c, 0xc7, 0xc1, 0xc1, 0x71, 0x1c, 0xc7, 0xc1,
			0x71, 0x1c, 0x1c, 0x77, 0x1c, 0x77, 0xdc, 0xdd,
			0x31, 0xdd, 0xdd, 0x4d, 0x37, 0x4d, 0x43, 0xd3,
			0x34, 0x4d, 0xd3, 0xd0, 0x34, 0x34, 0x34, 0x34,
			0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

		}
	},
};

#endif
