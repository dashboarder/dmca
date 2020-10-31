/*
 * Copyright (C) 2011 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/iic.h>
#include <drivers/tristar.h>
#include <drivers/power.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>

#include <target/powerconfig.h>

#define THS7383_ADDR_R				0x35
#define THS7383_ADDR_W				0x34

enum {
	DXCTRL					= 0x01,
	    DXCTRL_DX2OVRD			= (1 << 7),
	    DXCTRL_DPDN2SW_mask			= (7 << 4),
	    DXCTRL_DPDN2SW_open			= (0 << 4),
	    DXCTRL_DPDN2SW_usb0			= (1 << 4),
	    DXCTRL_DPDN2SW_uart0		= (2 << 4),
	    DXCTRL_DPDN2SW_dig			= (3 << 4),
	    DXCTRL_DPDN2SW_brick_id_p		= (4 << 4),
	    DXCTRL_DPDN2SW_brick_id_n		= (5 << 4),
	    DXCTRL_DPDN2SW_uart2		= (6 << 4),
	    DXCTRL_DPDN2SW_uart1		= (7 << 4),
	    DXCTRL_DX1OVRD			= (1 << 3),
	    DXCTRL_DPDN1SW_mask			= (7 << 0),
	    DXCTRL_DPDN1SW_open			= (0 << 0),
	    DXCTRL_DPDN1SW_usb0			= (1 << 0),
	    DXCTRL_DPDN1SW_uart0		= (2 << 0),
	    DXCTRL_DPDN1SW_dig			= (3 << 0),
	    DXCTRL_DPDN1SW_brick_id_p		= (4 << 0),
	    DXCTRL_DPDN1SW_brick_id_n		= (5 << 0),
	    DXCTRL_DPDN1SW_usb1			= (6 << 0),
	    DXCTRL_DPDN1SW_jtag			= (7 << 0),
	    
	ACC_CTRL				= 0x02,
	    ACC_CTRL_ACC2OVRD			= (1 << 5),
	    ACC_CTRL_ACC2SW_mask		= (3 << 3),
	    ACC_CTRL_ACC2SW_open		= (0 << 3),
	    ACC_CTRL_ACC2SW_uart1_tx		= (1 << 3),
	    ACC_CTRL_ACC2SW_jtag_clk		= (2 << 3),
	    ACC_CTRL_ACC2SW_acc_pwr		= (3 << 3),
	    ACC_CTRL_ACC1OVRD			= (1 << 2),
	    ACC_CTRL_ACC1SW_mask		= (3 << 0),
	    ACC_CTRL_ACC1SW_open		= (0 << 0),
	    ACC_CTRL_ACC1SW_uart1_rx		= (1 << 0),
	    ACC_CTRL_ACC1SW_jtag_dio		= (2 << 0),
	    ACC_CTRL_ACC1SW_acc_pwr		= (3 << 0),
	
	DCP_CTRL				= 0x03,
	    DCP_CTRL_IDXSINKEN			= (1 << 3),
	    DCP_CTRL_VDXSRCSW_mask		= (7 << 0),
	    DCP_CTRL_VDXSRCSW_off		= (0 << 0),
	    DCP_CTRL_VDXSRCSW_dp1		= (1 << 0),
	    DCP_CTRL_VDXSRCSW_dn1		= (2 << 0),
	    DCP_CTRL_VDXSRCSW_dp2		= (3 << 0),
	    DCP_CTRL_VDXSRCSW_dn2		= (4 << 0),

	MISC_CTRL				= 0x05,
	    MISC_CTRL_IDBUS_RESET		= (1 << 3),
	    MISC_CTRL_IDBUS_BREAK		= (1 << 2),
	    MISC_CTRL_IDBUS_REORIENT		= (1 << 1),
	    MISC_CTRL_IDBUS_P_INSINK_EN		= (1 << 0),
	
	DIG_ID					= 0x06,
	    DIG_ID_Dx1				= (1 << 3),
	    DIG_ID_Dx0				= (1 << 2),
	    DIG_ID_ACCx1			= (1 << 1),
	    DIG_ID_ACCx0			= (1 << 0),

	STATUS1					= 0x0B,
	    STATUS1_CMD_PEND			= (1 << 0),
	
	STATUS0					= 0x0C,
	    STATUS0_IDBUS_CONNECTED		= (1 << 7),
	    STATUS0_IDBUS_ORIENT		= (1 << 6),
	    STATUS0_SWITCH_EN			= (1 << 5),
	    STATUS0_HOST_RESET			= (1 << 4),
	    STATUS0_OVP_SW			= (1 << 3),
	    STATUS0_CON_DET_L			= (1 << 2),
	    STATUS0_P_IN_STAT_mask		= (3 << 0),
	    STATUS0_P_IN_STAT_brownout		= (0 << 0),
	    STATUS0_P_IN_STAT_maintain		= (1 << 0),
	    STATUS0_P_IN_STAT_ovp		= (2 << 0),
	    STATUS0_P_IN_STAT_insdet		= (3 << 0),
	
	EVENT					= 0x0D,
	    EVENT_CRC_ERR			= (1 << 5),
	    EVENT_RESP_VALID			= (1 << 4),
	    EVENT_DIGITAL_ID			= (1 << 3),
	    EVENT_CON_DET_L			= (1 << 2),
	    EVENT_IDBUS_WAKE			= (1 << 1),
	    EVENT_P_IN				= (1 << 0),
	
	MASK					= 0x0E,
	    MASK_CRC_ERR			= (1 << 5),
	    MASK_RESP_VALID			= (1 << 4),
	    MASK_DIGITAL_ID			= (1 << 3),
	    MASK_CON_DET_L			= (1 << 2),
	    MASK_IDBUS_WAKE			= (1 << 1),
	    MASK_P_IN				= (1 << 0),
	
	REV					= 0x0F,
	    REV_VENDOR_ID_shift			= (6),
	    REV_VENDOR_ID_mask			= (3 << REV_VENDOR_ID_shift),
	    REV_VENDOR_ID_ti			= (1 << REV_VENDOR_ID_shift),
	    REV_BASE_VER_shift			= (3),
	    REV_BASE_VER_mask			= (7 << REV_BASE_VER_shift),
	    REV_METAL_VER_shift			= (0),
	    REV_METAL_VER_mask			= (7 << REV_METAL_VER_shift),
	    
	FIFO0					= 0x20,
	FIFO31					= 0x3F,
	FIFO_LEN				= (FIFO31 - FIFO0 + 1),

	FIFO_CTRL0				= 0x40,
	    FIFO_CTRL0_CMD_LENGTH_shift		= 1,
	    FIFO_CTRL0_CMD_LENGTH_mask		= (31 << FIFO_CTRL0_CMD_LENGTH_shift),
	    FIFO_CTRL0_AID_CMD_SEND		= (1 << 0),

	FIFO_CTRL1				= 0x41,
	    FIFO_CTRL1_RESP_LENGTH_shift	= 1,
	    FIFO_CTRL1_RESP_LENGTH_mask		= (31 << FIFO_CTRL1_RESP_LENGTH_shift),
	    FIFO_CTRL1_CMD_KILL			= (1 << 0),
	
	VENDOR50				= 0x50,
	VENDORFF				= 0xFF,
};

static int tristar_write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = { reg, value };
    return iic_write(TRISTAR_IIC_BUS, THS7383_ADDR_W, data, sizeof(data));
}

static int tristar_read(uint8_t reg, uint8_t *data)
{
    return iic_read(TRISTAR_IIC_BUS, THS7383_ADDR_R, &reg, sizeof(reg), data, sizeof(uint8_t), IIC_NORMAL);
}

bool ths7383_set_usb_brick_detect(int select)
{
    // check status to make sure cable is connected and determine orientation
    uint8_t status;
    if (tristar_read(STATUS0, &status) != 0) {
	return false;
    }
    if ((status & STATUS0_CON_DET_L) || !(status & STATUS0_IDBUS_CONNECTED)) {
	return false;
    }
    bool orient = (status & STATUS0_IDBUS_ORIENT) == STATUS0_IDBUS_ORIENT;

    // check ID to find out if USB is supported
    uint8_t dig_id;
    if (tristar_read(DIG_ID, &dig_id) != 0) {
	return false;
    }
    // only Dx=1 or Dx=2 have USB device mode
    if (((dig_id & DIG_ID_Dx0) != 0) == ((dig_id & DIG_ID_Dx1) != 0)) {
	return false;
    }

    uint8_t dxctrl;
    uint8_t dcp_ctrl;

    switch (select) {
	case kUSB_DP:
	case kUSB_CP2:
	    dxctrl = orient ? (DXCTRL_DX2OVRD | DXCTRL_DPDN2SW_brick_id_p) : (DXCTRL_DX1OVRD | DXCTRL_DPDN1SW_brick_id_p);
	    break;
	
	case kUSB_DM:
	case kUSB_CP1:
	    dxctrl = orient ? (DXCTRL_DX2OVRD | DXCTRL_DPDN2SW_brick_id_n) : (DXCTRL_DX1OVRD | DXCTRL_DPDN1SW_brick_id_n);
	    break;

	case kUSB_NONE:
	    dxctrl = 0;
	    break;

	default:
	    return false;
    }

    if (select == kUSB_CP1) {
	dcp_ctrl = DCP_CTRL_IDXSINKEN | (orient ? DCP_CTRL_VDXSRCSW_dp2 : DCP_CTRL_VDXSRCSW_dp1);
    } else if (select == kUSB_CP2) {
	dcp_ctrl = DCP_CTRL_IDXSINKEN | (orient ? DCP_CTRL_VDXSRCSW_dn2 : DCP_CTRL_VDXSRCSW_dn1);
    } else {
	dcp_ctrl = 0;
    }
    
    tristar_write(DCP_CTRL, dcp_ctrl);
    tristar_write(DXCTRL, dxctrl);
    
    return true;
}

bool ths7383_read_id(uint8_t digital_id[6])
{
    // check status to make sure cable is connected and ID bus is connected
    uint8_t status;
    if (tristar_read(STATUS0, &status) != 0) {
	return false;
    }
    if ((status & STATUS0_CON_DET_L) || !(status & STATUS0_IDBUS_CONNECTED)) {
	return false;
    }
    
    // FIFO should contain the ID response
    uint8_t resp;
    if (tristar_read(FIFO31, &resp) != 0) {
	return false;
    }

    // work around 10170313
    if (resp != 0x75) {
	tristar_write(MISC_CTRL, MISC_CTRL_IDBUS_REORIENT);
	task_sleep(100 * 1000);
	tristar_read(FIFO31, &resp);
    }

    if (resp != 0x75) {
	return false;
    }

    // read ID and flip order
    for (int i = 0; i < 6; i++) {
	if (tristar_read(FIFO31-(i+1), &digital_id[i])) {
	    return false;
	}
    }

    return true;
}

bool ths7383_enable_acc_pwr(bool enabled)
{
    if (enabled) {
	tristar_write(ACC_CTRL, ACC_CTRL_ACC2SW_acc_pwr | ACC_CTRL_ACC1SW_acc_pwr);
    }

    power_enable_ldo(ACC_PWR_LDO, enabled);

    if (!enabled) {
	tristar_write(ACC_CTRL, 0);
    }

    return true;
}

#if !WITH_HW_TRISTAR_CBTL1610
bool tristar_set_usb_brick_detect(int select)
{
    return ths7383_set_usb_brick_detect(select);
}

bool tristar_read_id(uint8_t digital_id[6])
{
    return ths7383_read_id(digital_id);
}

bool tristar_enable_acc_pwr(bool enabled)
{
    return ths7383_enable_acc_pwr(enabled);
}
#endif // WITH_HW_TRISTAR_CBTL1610
