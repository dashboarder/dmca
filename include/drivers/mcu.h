/*
 * Copyright (C) 2010 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_MCU_H
#define __DRIVERS_MCU_H

#include <sys/types.h>

__BEGIN_DECLS

int mcu_init(void);
int mcu_start_recover(void);
int mcu_start_boot(void);
int mcu_set_passthrough_mode(bool on);
int mcu_send_info_frames(bool on);
int mcu_quiesce_uart(void);

__END_DECLS

#endif /* __DRIVERS_MCU_H */

