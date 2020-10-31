/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_CHARGER_H
#define __DRIVERS_CHARGER_H

#include <sys/types.h>

__BEGIN_DECLS

void charger_early_init(void);
void charger_clear_alternate_usb_current_limit(void);
bool charger_has_usb(int dock);
bool charger_has_firewire(int dock);
bool charger_has_external(int dock);
bool charger_check_usb_change(int dock, bool expected);
void charger_set_charging(int dock, uint32_t input_current_ma, uint32_t *charge_current_ma);
void charger_clear_usb_state(void);
bool charger_set_usb_brick_detect(int dock, int select);
bool charger_charge_done(int dock);
bool charger_has_batterypack(int dock);
uint32_t charger_get_max_charge_current(int dock);
int charger_read_battery_temperature(int *centiCelsiusTemperature);
int charger_read_battery_level(uint32_t *milliVolts);
int charger_set_gasgauge_interface(bool enabled);
void charger_print_status(void);

__END_DECLS

#endif /* __DRIVERS_CHARGER_H */

