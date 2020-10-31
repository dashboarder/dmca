/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <lib/env.h>
#include <lib/paint.h>
#include <lib/syscfg.h>
#include <drivers/wpc.h>
#include "wpc.h"

static const uint32_t wpcConfig_DeGamma[WPC_CONFIG_DE_GAMMA_COUNT] = {
	0, 	159, 	317, 	476, 	634, 	793, 	960,
	1146, 	1352, 	1577, 	1824, 	2091, 	2380,	2691,
	3025,	3382,	3762,	4166,	4594,	5046,	5524,
	6027,	6556,	7111,	7692,	8300,	8936,	9599,
	10289,	11008,	11755,	12532,	13337,	14172,	15036,
	15930,	16855,	17810,	18796,	19813,	20862,	21942,
	23054,	24199,	25376,	26585,	27828,	29104,	30413,
	31756,	33133,	34544,	35990,	37470,	38986,	40536,
	42122,	43743,	45401,	47094,	48824,	50590,	52393,
	54233,	56110,	58024,	59976,	61965,	63993,	66059,
	68163,	70306,	72487,	74708,	76968,	79267,	81606,
	83984,	86402,	88861,	91360,	93899,	96479,	99100,
	101762,	104466,	107211,	109997,	112825,	115695,	118607,
	121562,	124559,	127599,	130681,	133807,	136976,	140188,
	143444,	146743,	150086,	153473,	156905,	160380,	163901,
	167466,	171075,	174730,	178430,	182175,	185966,	189802,
	193684,	197612,	201586,	205606,	209672,	213786,	217945,
	222152,	226406,	230706,	235054,	239450,	243893,	248384,
	252922,	257509,
};

static const uint32_t wpcConfig_EnGamma[WPC_CONFIG_EN_GAMMA_COUNT] = {
	0, 	103, 	207, 	310, 	413, 	517, 	620, 	721,
	814,	900,	981,	1057,	1129,	1198,	1264,	1327,
	1388,	1447,	1503,	1558,	1611,	1663,	1713,	1762,
	1809,	1856,	1901,	1946,	1989,	2032,	2073,	2114,
	2154,	2194,	2233,	2271,	2308,	2345,	2381,	2417,
	2452,	2487,	2521,	2555,	2588,	2621,	2653,	2685,
	2717,	2748,	2779,	2809,	2840,	2869,	2899,	2928,
	2957,	2985,	3014,	3042,	3069,	3097,	3124,	3151,
	3178,	3230,	3282,	3333,	3383,	3432,	3480,	3528,
	3575,	3621,	3667,	3712,	3756,	3800,	3843,	3886,
	3928,	3970,	4011,	4052,	4092,	4132,	4171,	4210,
	4249,	4287,	4325,	4362,	4399,	4436,	4472,	4508,
	4543,	4614,	4683,	4750,	4817,	4883,	4947,	5011,
	5074,	5136,	5197,	5257,	5316,	5374,	5432,	5489,
	5545,	5601,	5656,	5710,	5764,	5817,	5870,	5922,
	5973,	6024,	6074,	6124,	6174,	6222,	6271,	6319,
	6366,	6460,	6552,	6643,	6732,	6820,	6906,	6991,
	7074,	7157,	7238,	7318,	7398,	7476,	7553,	7629,
	7704,	7778,	7852,	7924,	7996,	8067,	8137,	8206,
	8275,	8343,	8410,	8477,	8542,	8608,	8672,	8736,
	8800,	8925,	9048,	9169,	9288,	9405,	9520,	9633,
	9745,	9855,	9964,	10071,	10176,	10281,	10383,	10485,
	10585,	10684,	10782,	10879,	10975,	11069,	11163,	11256,
	11347,	11438,	11528,	11617,	11705,	11792,	11878,	11963,
	12048,	12215,	12379,	12541,	12699,	12856,	13009,	13161,
	13310,	13457,	13602,	13745,	13886,	14025,	14162,	14298,
	14431,	14564,	14694,	14824,	14951,	15078,	15203,	15326,
	15449,	15570,	15689,	15808,	15925,	16042,	16157,	16271,
};

void wpc_write_reg(uint32_t reg, uint32_t value);
uint32_t wpc_read_reg(uint32_t reg);

void wpc_init(uint32_t display_width, uint32_t display_height)
{
	// WPC Configuration defined in ADBE TRM v0.1.14. Section 4.4
	
	// Active Region Start
	wpc_write_reg(WPC_CONFIG_ACTIVE_REGION_START_OFFSET, 
		WPC_CONFIG_ACTIVE_REGION_START_START_X_INSRT(0) | 
		WPC_CONFIG_ACTIVE_REGION_START_START_Y_INSRT(0));
	
	// Active Region Size
	wpc_write_reg(WPC_CONFIG_ACTIVE_REGION_SIZE_OFFSET, 
		WPC_CONFIG_ACTIVE_REGION_SIZE_WIDTH_INSRT(display_width) | 
		WPC_CONFIG_ACTIVE_REGION_SIZE_HEIGHT_INSRT(display_height));
		
	// Pixel Gain
	for (int i = 0; i < WPC_CONFIG_PIXEL_GAIN_COUNT; i ++) {
		wpc_write_reg(WPC_CONFIG_PIXEL_GAIN_OFFSET(i), 1 << 19);
	}
		
	// GainStep (Round using 48.16 Fixed Point)
	wpc_write_reg(WPC_CONFIG_GAIN_STEP_X_OFFSET, (uint32_t)((((16777216ULL << 16) / display_width) + (1<<15)) >> 16));
	wpc_write_reg(WPC_CONFIG_GAIN_STEP_Y_OFFSET, (uint32_t)((((16777216ULL << 16) / display_height) + (1<<15)) >> 16));
	
	// Kick the values in to the registers.
	wpc_write_reg(WPC_CONFIG_UPDATE_CONTROL_OFFSET, 0x3);
}

void wpc_install_gamma_table(u_int32_t *red_lut, u_int32_t *green_lut, u_int32_t *blue_lut, struct syscfg_wpcl *wpcl)
{	
	// DeGamma
	for (int i = 0; i < WPC_CONFIG_DE_GAMMA_COUNT; i ++) {
		wpc_write_reg(WPC_CONFIG_DE_GAMMA_OFFSET(i), wpcConfig_DeGamma[i]);
	}
	
	// DeGamma Last
	wpc_write_reg(WPC_CONFIG_DE_GAMMA_LAST_OFFSET, 262144);
	
	// EnGamma
	for (int i = 0; i < WPC_CONFIG_EN_GAMMA_COUNT; i ++) {
		wpc_write_reg(WPC_CONFIG_EN_GAMMA_OFFSET(i), wpcConfig_EnGamma[i]);
	}
	
	// EnGamma Last
	wpc_write_reg(WPC_CONFIG_EN_GAMMA_LAST_OFFSET, 16384);
}

void wpc_write_reg(uint32_t reg, uint32_t value)
{
	(*(volatile u_int32_t *)(WPC_BASE_ADDR + reg)) = value;
}

uint32_t wpc_read_reg(uint32_t reg)
{
	return (*(volatile u_int32_t *)(WPC_BASE_ADDR + reg));
}