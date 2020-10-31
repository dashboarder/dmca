/*
 * Copyright (C) 2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_TRISTAR_H
#define __DRIVERS_TRISTAR_H

#include <sys/types.h>

__BEGIN_DECLS

bool tristar_set_usb_brick_detect(int select);
bool tristar_read_id(uint8_t digital_id[6]);
bool tristar_enable_acc_pwr(bool enabled);

__END_DECLS

#endif /* __DRIVERS_CHARGER_H */

