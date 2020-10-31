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

#ifndef _LPDP_PHY_H
#define _LPDP_PHY_H

int	lpdp_init(const char *dt_path);
void	lpdp_quiesce(void);
int	lpdp_initialize_phy_and_pll();
void	lpdp_init_finalize();
void	lpdp_phy_reset();
int	lpdp_set_link_rate(uint32_t lr);
int	lpdp_get_link_rate(uint32_t *link_rate);
int	lpdp_phy_set_adjustment_levels(uint32_t lane, uint32_t voltage_swing, uint32_t eq, 
                                        bool *voltage_max_reached, bool *eq_max_reached);
bool	lpdp_get_supports_downspread();
int	lpdp_set_downspread(bool value);
int	lpdp_get_downspread(void);
int	lpdp_phy_get_adjustment_levels(uint32_t lane, uint32_t *voltage_swing, uint32_t *eq);
void	lpdp_phy_set_lane_count(const uint32_t lane_count);
void	lpdp_phy_configure_alpm(bool enable);

#endif //_LPDP_PHY_H
