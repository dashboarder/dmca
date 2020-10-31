/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __DRIVERS_MIPI_H
#define __DRIVERS_MIPI_H

#include <drivers/display.h>
#include <sys/types.h>

__BEGIN_DECLS

int  mipi_dsim_init(struct display_timing *timing, enum colorspace color);
void mipi_dsim_enable_high_speed(bool enable);
void mipi_dsim_enable_video(bool enable);
int  mipi_dsim_quiesce(void);
int  mipi_dsim_send_short_command(u_int8_t cmd, u_int8_t data0, u_int8_t data1);
int  mipi_dsim_send_long_command(u_int8_t cmd, const u_int8_t *data, u_int32_t length);
int  mipi_dsim_read_short_command(u_int8_t cmd, u_int8_t *data);
int  mipi_dsim_read_long_command(u_int8_t cmd, u_int8_t *data, u_int32_t *length);

/* DSIM command types */
#define DSIM_TYPE_DSC_WRITE		0x05
#define DSIM_TYPE_DSC_READ		0x06
#define DSIM_TYPE_GEN_READ_1P		0x14
#define DSIM_TYPE_DSC_WRITE_1P		0x15
#define DSIM_TYPE_DSC_READ_2P		0x24
#define DSIM_TYPE_GEN_LONG_WRITE	0x29
#define DSIM_TYPE_GEN_LUT_WRITE		0x39

/* DSIM response types */
#define DSIM_RSP_READ_1B		0x11
#define DSIM_RSP_READ_2B		0x12
#define DSIM_RSP_LONG_READ		0x1A
#define DSIM_RSP_DSC_LONG_READ		0x1C
#define DSIM_RSP_DSC_READ_1B		0x21
#define DSIM_RSP_DSC_READ_2B		0x22

/* DSIM commands / registers */
#define DSIM_CMD_NOP			0x00
#define DSIM_CMD_ENTER_SLEEP		0x10
#define DSIM_CMD_EXIT_SLEEP		0x11
#define DSIM_CMD_INVERT_OFF		0x20
#define DSIM_CMD_INVERT_ON		0x21
#define DSIM_CMD_DISPLAY_OFF		0x28
#define DSIM_CMD_DISPLAY_ON		0x29
#define DSIM_CMD_ADDRESS_MODE		0x36
#define DSIM_CMD_DEVICE_ID		0xb1

__END_DECLS

#endif /* ! __DRIVERS_MIPI_H */
