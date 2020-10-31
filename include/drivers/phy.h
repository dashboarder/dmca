/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
/*
 * Ethernet PHY support.
 */
#ifndef __DRIVERS_PHY_H
#define __DRIVERS_PHY_H

#include <sys/types.h>

__BEGIN_DECLS

enum phy_speed {
	PHY_SPEED_UNKNOWN,
	PHY_SPEED_AUTO,
	PHY_SPEED_10,
	PHY_SPEED_100,
	PHY_SPEED_1000
};

int	phy_init(int port);
int	phy_set_speed(int port, enum phy_speed speed);
bool	phy_get_link(int port);
extern int	phy_read_reg(int port, uint32_t reg, uint16_t *data);
extern int	phy_write_reg(int port, uint32_t reg, uint16_t data);

__END_DECLS

#endif /* __DRIVERS_PHY_H */
