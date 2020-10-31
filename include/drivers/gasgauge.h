/*
 * Copyright (C) 2009 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef __DRIVERS_GASGAUGE_H
#define __DRIVERS_GASGAUGE_H

#include <sys/types.h>
#include <drivers/power.h>

__BEGIN_DECLS

void gasgauge_init(void);
void gasgauge_late_init(void);
void gasgauge_print_status(void);

#define GG_NEEDS_PRECHARGE (1<<0)
#define GG_NEEDS_RESET     (1<<1)
#define GG_COMMS_WD        (1<<2)

int gasgauge_needs_precharge(int debug_print_level, bool debug_show_target);
void gasgauge_will_shutdown(void);
bool gasgauge_full(void);
int gasgauge_get_battery_id(u_int8_t *buf, int size);
bool gasgauge_check_health(unsigned int vbat_mv);
bool gasgauge_read_charge_table(struct power_charge_limits *table, size_t num_elems);

// gasgauge_read_temperature: returns 0 on success 
int gasgauge_read_temperature(int *centiCelsiusTemperature);
// gasgauge_read_voltage: returns 0 on success 
int gasgauge_read_voltage(unsigned int *milliVolt);
int gasgauge_read_design_capacity(unsigned int *milliAmpHours);
int gasgauge_read_soc(unsigned int *soc);
int gasgauge_reset_timer(int debug_print_level, unsigned int timeout);

// Gas Gauge Flags
enum {
    kHDQRegFlagsMaskOTC			= (1 << 15),
    kHDQRegFlagsMaskOTD			= (1 << 14),
    kHDQRegFlagsMaskCHG_INH		= (1 << 11),
    kHDQRegFlagsMaskXCHG		= (1 << 10),

    kHDQRegFlagsMaskIMAXOK		= (1 << 13),    // Aquarius
    kHDQRegFlagsMaskTC2			= (1 << 11),    // 0x800 Aquarius
    kHDQRegFlagsMaskTC1			= (1 << 10),    // 0x400 Aquarius

    kHDQRegFlagsMaskFC			= (1 << 9),
    kHDQRegFlagsMaskCHG			= (1 << 8),
    kHDQRegFlagsMaskCCA_REQ		= (1 << 5),	// Aquarius
    kHDQRegFlagsMaskLOWV		= (1 << 4),	// A4
    kHDQRegFlagsMaskSOC1		= (1 << 2),
    kHDQRegFlagsMaskSOCF		= (1 << 1),
    kHDQRegFlagsMaskDSG			= (1 << 0),
};

__END_DECLS

#if (WITH_HW_GASGAUGE && DEBUG_BUILD)
// define BATTERY_TRAP_DEBUG for full gas gauge register dump in gasgauge_needs_precharge
#define BATTERY_TRAP_DEBUG
#endif
#endif /* __DRIVERS_GAUGE_H */

