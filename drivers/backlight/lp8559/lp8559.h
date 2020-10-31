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
#ifndef __DRIVERS_LP8559_H
#define __DRIVERS_LP8559_H

#define lp8559_IIC_ADDRESS	(0x4C)

#define lp8559_COMMAND		(0x00)
#define lp8559_LOCK		(0x0F)
#define lp8559_CONFIG		(0x10)
#define lp8559_CURRENT		(0x11)
#define lp8559_BCFG1		(0x15)
#define lp8559_BCFG2		(0x16)
#define lp8559_BCFG3		(0x17)
#define lp8559_BCFG5		(0x19)
#define lp8559_HYBRIDLO		(0x1A)
#define lp8559_VHR1		(0x1E)
#define lp8559_BCTRL		(0x20)
#define lp8559_TRIM2		(0x51)
#define lp8559_TRIM5		(0x54)
#define lp8559_TRIM9		(0x58)
#define lp8559_LEDCFG		(0xE6)
#define lp8559_TMLOCK		(0xEE)
#define lp8559_EPROMID		(0xFC)

int beacon_reg_read(uint8_t reg_offset, uint8_t *value);
int beacon_reg_write(uint8_t reg_offset, uint8_t value);

#endif /* __DRIVERS_LP8559_H */
