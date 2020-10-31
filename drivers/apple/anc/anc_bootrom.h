/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _ANC_BOOTROM_H_
#define _ANC_BOOTROM_H_

#include <stdint.h>

int anc_bootrom_init(bool reset, int resetMode);
bool anc_bootrom_read_phys_page(uint32_t band,
								uint32_t dip,
								uint32_t page,
								uint32_t num_lbas,
								void     *data,
								uint32_t *meta);
#endif // _ANC_BOOTROM_H_
