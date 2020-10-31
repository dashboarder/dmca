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
#ifndef LPDP_SETTINGS_H
#define LPDP_SETTINGS_H

static struct lpdp_port_calibration lpdp_port_calibration_table[1][1] = {
    { {LPDP_PHY_LANE_VREG_ADJ_390_mV, 0}, },
};

#define LPDP_PORT_CALIBRATION_TABLE_FIXED (true)

#endif
