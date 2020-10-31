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

#ifndef DRIVER_DISPLAYPORT_H
#define DRIVER_DISPLAYPORT_H	1

#include <drivers/display.h>
#include <drivers/displayAV.h>

typedef struct
{
	uint32_t mode           : 4;    // 3:0
	uint32_t type           : 4;    // 7:4
	uint32_t min_link_rate  : 8;    // 15:8
	uint32_t max_link_rate  : 8;    // 23:16
	uint32_t lanes          : 4;    // 27:24
	uint32_t ssc            : 1;    // 28
	uint32_t alpm           : 1;    // 29
	uint32_t vrr_enable     : 1;    // 30
	uint32_t vrr_on         : 1;    // 31
	uint32_t rx_n1;
	uint32_t rx_n2;
	uint32_t rx_n3;
	uint32_t rx_n5;
	bool fast_link_training;
} dp_t;

#endif //DRIVER_DISPLAYPORT_H
