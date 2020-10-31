/*
 * Copyright (C) 2012 Apple Computer, Inc. All rights reserved.
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

#include <target/powerconfig.h>

#define CBTL1610_ADDR_R				0x35
#define CBTL1610_ADDR_W				0x34

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
	    ACC_CTRL_ACC2OVRD			= (1 << 7),
	    ACC_CTRL_ACC2SW_mask		= (7 << 4),
	    ACC_CTRL_ACC2SW_open		= (0 << 4),
	    ACC_CTRL_ACC2SW_uart1_tx		= (1 << 4),
	    ACC_CTRL_ACC2SW_jtag_clk		= (2 << 4),
	    ACC_CTRL_ACC2SW_acc_pwr		= (3 << 4),
	    ACC_CTRL_ACC2SW_brick_id		= (4 << 4),
	    ACC_CTRL_ACC1OVRD			= (1 << 3),
	    ACC_CTRL_ACC1SW_mask		= (7 << 0),
	    ACC_CTRL_ACC1SW_open		= (0 << 0),
	    ACC_CTRL_ACC1SW_uart1_rx		= (1 << 0),
	    ACC_CTRL_ACC1SW_jtag_dio		= (2 << 0),
	    ACC_CTRL_ACC1SW_acc_pwr		= (3 << 0),
	    ACC_CTRL_ACC1SW_brick_id		= (4 << 0),
	
	DCP_CTRL				= 0x03,
	    DCP_CTRL_IDXSINKEN			= (1 << 3),
	    DCP_CTRL_VDXSRCSW_mask		= (7 << 0),
	    DCP_CTRL_VDXSRCSW_off		= (0 << 0),
	    DCP_CTRL_VDXSRCSW_dp1		= (1 << 0),
	    DCP_CTRL_VDXSRCSW_dn1		= (2 << 0),
	    DCP_CTRL_VDXSRCSW_dp2		= (3 << 0),
	    DCP_CTRL_VDXSRCSW_dn2		= (4 << 0),

	MISC_CTRL				= 0x05,
	    MISC_CTRL_DPDN2_TERM		= (1 << 5),
	    MISC_CTRL_DPDN1_TERM		= (1 << 4),
	    MISC_CTRL_IDBUS_RESET		= (1 << 3),
	    MISC_CTRL_IDBUS_BREAK		= (1 << 2),
	    MISC_CTRL_IDBUS_REORIENT		= (1 << 1),
	    MISC_CTRL_IDBUS_P_INSINK_EN		= (1 << 0),

	DIG_ID					= 0x06,
	    DIG_ID_Dx1				= (1 << 3),
	    DIG_ID_Dx0				= (1 << 2),
	    DIG_ID_ACCx1			= (1 << 1),
	    DIG_ID_ACCx0			= (1 << 0),

	FAULT_ENABLE				= 0x09,
	    DYNAMIC_CHRG_PUMP			= (1 << 7),
	    CHECK_VICT				= (1 << 6),
	    CHECK_AGGR				= (1 << 5),
	    PROTECT_UNDER			= (1 << 4),
	    PROTECT_DIG				= (1 << 3),
	    PROTECT_UART			= (1 << 2),
	    PROTECT_USB				= (1 << 1),
	    PROTECT_ACC				= (1 << 0),

	EVENT1					= 0x0A,
	    EVENT1_Dx2_FAULT			= (1 << 2),
	    EVENT1_Dx1_FAULT			= (1 << 1),
	    EVENT1_ACCx_FAULT			= (1 << 0),
	
	STATUS1					= 0x0B,
	    STATUS1_CMD_PEND			= (1 << 0),
	
	STATUS0					= 0x0C,
	    STATUS0_IDBUS_CONNECTED		= (1 << 7),
	    STATUS0_IDBUS_ORIENT		= (1 << 6),
	    STATUS0_SWITCH_EN			= (1 << 5),
	    STATUS0_HOST_RESET			= (1 << 4),
	    STATUS0_POWER_GATE_EN		= (1 << 3),
	    STATUS0_CON_DET_L			= (1 << 2),
	    STATUS0_P_IN_STAT_mask		= (3 << 0),
	    STATUS0_P_IN_STAT_brownout		= (0 << 0),
	    STATUS0_P_IN_STAT_maintain		= (1 << 0),
	    STATUS0_P_IN_STAT_ovp		= (2 << 0),
	    STATUS0_P_IN_STAT_insdet		= (3 << 0),

	EVENT0					= 0x0D,
	    EVENT_IO_FAULT			= (1 << 7),
	    EVENT_IDBUS_TIMEOUT			= (1 << 6),
	    EVENT_FIFO_ERR			= (1 << 5),
	    EVENT_FIFO_RDY			= (1 << 4),
	    EVENT_CRC_ERR			= (1 << 5),
	    EVENT_RESP_VALID			= (1 << 4),
	    EVENT_DIGITAL_ID			= (1 << 3),
	    EVENT_CON_DET_L			= (1 << 2),
	    EVENT_IDBUS_WAKE			= (1 << 1),
	    EVENT_P_IN				= (1 << 0),

	MASK					= 0x0E,
	    MASK_IO_FAULT			= (1 << 7),
	    MASK_IDBUS_TIMEOUT			= (1 << 6),
	    MASK_CRC_ERR			= (1 << 5),
	    MASK_RESP_VALID			= (1 << 4),
	    MASK_DIGITAL_ID			= (1 << 3),
	    MASK_CON_DET_L			= (1 << 2),
	    MASK_IDBUS_WAKE			= (1 << 1),
	    MASK_P_IN				= (1 << 0),

	REV					= 0x0F,
	    REV_VENDOR_ID_shift			= (6),
	    REV_VENDOR_ID_mask			= (3 << REV_VENDOR_ID_shift),
	    REV_VENDOR_ID_nxp			= (2 << REV_VENDOR_ID_shift),
	    REV_BASE_VER_shift			= (3),
	    REV_BASE_VER_mask			= (7 << REV_BASE_VER_shift),
	    REV_METAL_VER_shift			= (0),
	    REV_METAL_VER_mask			= (7 << REV_METAL_VER_shift),

	DP1_DP2_UART_CTL			= 0x10,
	    DP1_DP2_UART_CTL_DP2_SLEW_mask	= (3 << 2),
	    DP1_DP2_UART_CTL_DP2_SLEW_10ns	= (0 << 2),
	    DP1_DP2_UART_CTL_DP2_SLEW_20ns	= (1 << 2),
	    DP1_DP2_UART_CTL_DP2_SLEW_40ns	= (2 << 2),
	    DP1_DP2_UART_CTL_DP2_SLEW_80ns	= (3 << 2),
	    DP1_DP2_UART_CTL_DP1_SLEW_mask	= (3 << 0),
	    DP1_DP2_UART_CTL_DP1_SLEW_10ns	= (0 << 0),
	    DP1_DP2_UART_CTL_DP1_SLEW_20ns	= (1 << 0),
	    DP1_DP2_UART_CTL_DP1_SLEW_40ns	= (2 << 0),
	    DP1_DP2_UART_CTL_DP1_SLEW_80ns	= (3 << 0),
	
	AUTH_CTRL0				= 0x11,
	    AUTH_CTRL0_LOCK_STAT_VALID		= (1 << 7),
	    AUTH_CTRL0_ESN_LOCK_STAT		= (1 << 6),
	    AUTH_CTRL0_KEYSET2_LOCK_STAT	= (1 << 5),
	    AUTH_CTRL0_KEYSET1_LOCK_STAT	= (1 << 4),
	    AUTH_CTRL0_SELECT_AUTH_DOMAIN_mask	= (1 << 3),
	    AUTH_CTRL0_SELECT_AUTH_DOMAIN_0	= (0 << 3),
	    AUTH_CTRL0_SELECT_AUTH_DOMAIN_1	= (1 << 3),
	    AUTH_CTRL0_I2C_AUTH_DONE		= (1 << 1),
	    AUTH_CTRL0_I2C_AUTH_START		= (1 << 0),
	
	ACC_FAULT_STATUS			= 0x12,
	    ACC_FAULT_STATUS_RVR_COMP_OUT	= (1 << 6),
	    ACC_FAULT_STATUS_ACC_FINGERS_5	= (1 << 5),
	    ACC_FAULT_STATUS_ACC_FINGERS_4	= (1 << 4),
	    ACC_FAULT_STATUS_ACC_FINGERS_3	= (1 << 3),
	    ACC_FAULT_STATUS_ACC_FINGERS_2	= (1 << 2),
	    ACC_FAULT_STATUS_ACC_FINGERS_1	= (1 << 1),
	    ACC_FAULT_STATUS_ACC_FINGERS_0	= (1 << 0),
	
	ACC_FAULT_CTRL0				= 0x13,
	    ACC_FAULT_CTRL0_EN_2X_OFFSET	= (1 << 7),
	    ACC_FAULT_CTRL0_BP_DISABLE_ACC_DISCONNECT	= (1 << 6),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_mask	= (15 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_no	= (0 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_12us	= (1 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_36us	= (2 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_60us	= (3 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_100us	= (4 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_200us	= (5 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_500us	= (6 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_1000us	= (7 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_2ms	= (8 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_5ms	= (9 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_10ms	= (10 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_20ms	= (11 << 0),
	    ACC_FAULT_CTRL0_BP_DEGLITCH_50ms	= (12 << 0),
	
	ACC_FAULT_CTRL1				= 0x14,
	    ACC_FAULT_CTRL1_BP_MODE_mask	= (3 << 6),
	    ACC_FAULT_CTRL1_BP_MODE_no		= (0 << 6),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_mask	= (63 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_800mohm	= (0 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_270mohm	= (32 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_170mohm	= (48 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_130mohm	= (56 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_100mohm	= (52 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_90mohm	= (60 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_600mohm	= (62 << 0),
	    ACC_FAULT_CTRL1_BP_SW_CTRL_40mohm	= (63 << 0),

	MISC_IO					= 0x1D,
	    MISC_IO_IDBUS_TIMEOUT_mask		= (3 << 3),
	    MISC_IO_IDBUS_TIMEOUT_disabled	= (0 << 3),
	    MISC_IO_IDBUS_TIMEOUT_5s		= (1 << 3),
	    MISC_IO_IDBUS_TIMEOUT_10s		= (2 << 3),
	    MISC_IO_IDBUS_TIMEOUT_30s		= (3 << 3),
	    MISC_IO_UART2_LOOP_BK		= (1 << 2),
	    MISC_IO_UART1_LOOP_BK		= (1 << 1),
	    MISC_IO_UART0_LOOP_BK		= (1 << 0),
	
	CON_DET_SMPL				= 0x1E,
	    CON_DET_SMPL_CON_DET_PULLUP_mask	= (3 << 5),
	    CON_DET_SMPL_CON_DET_PULLUP_20kohm	= (0 << 5),
	    CON_DET_SMPL_CON_DET_PULLUP_40kohm	= (1 << 5),
	    CON_DET_SMPL_CON_DET_PULLUP_60kohm	= (2 << 5),
	    CON_DET_SMPL_CON_DET_PULLUP_80kohm	= (3 << 5),
	    CON_DET_SMPL_SMPL_DUR_mask		= (3 << 3),
	    CON_DET_SMPL_SMPL_DUR_15us		= (0 << 3),
	    CON_DET_SMPL_SMPL_DUR_70us		= (1 << 3),
	    CON_DET_SMPL_SMPL_DUR_130us		= (2 << 3),
	    CON_DET_SMPL_SMPL_DUR_260us		= (3 << 3),
	    CON_DET_SMPL_SMPL_RATE_mask		= (3 << 1),
	    CON_DET_SMPL_SMPL_RATE_660Hz	= (0 << 1),
	    CON_DET_SMPL_SMPL_RATE_265Hz	= (1 << 1),
	    CON_DET_SMPL_SMPL_RATE_130Hz	= (2 << 1),
	    CON_DET_SMPL_SMPL_RATE_70Hz		= (3 << 1),
	    CON_DET_SMPL_SMPL_MODE_mask		= (1 << 0),
	    CON_DET_SMPL_SMPL_MODE_TriStar2	= (0 << 0),
	    CON_DET_SMPL_SMPL_MODE_TriStar1	= (1 << 0),
	
	RD_FIFO					= 0x1F,
	
	FIFO0					= 0x20,
	FIFO63					= 0x5F,
	FIFO_LEN				= (FIFO63 - FIFO0 + 1),
	
	FIFO_MTP2_TIMING			= 0x20,
	FIFO_KEY_CTRL				= 0x21,
	    FIFO_KEY_CTRL_LOCK_REQ		= (1 << 5),
	    FIFO_KEY_CTRL_ESN			= (1 << 4),
	    FIFO_KEY_CTRL_KEY2_2		= (1 << 3),
	    FIFO_KEY_CTRL_KEY2_1		= (1 << 2),
	    FIFO_KEY_CTRL_KEY1_2		= (1 << 1),
	    FIFO_KEY_CTRL_KEY1_1		= (1 << 0),
	FIFO_KEY_ESN_BYTE0			= 0x22,
	FIFO_KEY_ESN_BYTE1			= 0x23,
	FIFO_KEY_ESN_BYTE2			= 0x24,
	FIFO_KEY_ESN_BYTE3			= 0x25,
	FIFO_KEY_ESN_BYTE4			= 0x26,
	FIFO_KEY_ESN_BYTE5			= 0x27,
	FIFO_KEY_ESN_BYTE6			= 0x28,
	FIFO_KEY_ESN_BYTE7			= 0x29,
	FIFO_MTP2_PRG_CTRL			= 0x2E,
	FIFO_ENONCE_M_BYTE0			= 0x49,
	FIFO_ENONCE_M_BYTE1			= 0x50,
	FIFO_ENONCE_M_BYTE2			= 0x51,
	FIFO_ENONCE_M_BYTE3			= 0x52,
	FIFO_ENONCE_M_BYTE4			= 0x53,
	FIFO_ENONCE_M_BYTE5			= 0x54,
	FIFO_ENONCE_M_BYTE6			= 0x55,
	FIFO_ENONCE_M_BYTE7			= 0x56,
	FIFO_ESN_BYTE0				= 0x57,
	FIFO_ESN_BYTE1				= 0x58,
	FIFO_ESN_BYTE2				= 0x59,
	FIFO_ESN_BYTE3				= 0x5A,
	FIFO_ESN_BYTE4				= 0x5B,
	FIFO_ESN_BYTE5				= 0x5C,
	FIFO_ESN_BYTE6				= 0x5D,
	FIFO_ESN_BYTE7				= 0x5E,
	
	FIFO_CTRL1				= 0x60,
	    FIFO_CTRL1_ULIMITED_RX		= (1 << 7),
	    FIFO_CTRL1_RESP_LENGTH_shift	= (1),
	    FIFO_CTRL1_RESP_LENGTH_mask		= (63 << FIFO_CTRL1_RESP_LENGTH_shift),
	    FIFO_CTRL1_RD_TRIG_LVL_shift	= (1),
	    FIFO_CTRL1_RD_TRIG_LVL_mask		= (63 << FIFO_CTRL1_RD_TRIG_LVL_shift),
	    FIFO_CTRL1_CMD_KILL			= (1 << 0),
	
	FIFO_CTRL0				= 0x61,
	    FIFO_CTRL0_CMD_LENGTH_shift		= (1),
	    FIFO_CTRL0_CMD_LENGTH_mask		= (63 << FIFO_CTRL0_CMD_LENGTH_shift),
	    FIFO_CTRL0_AID_CMD_SEND		= (1 << 0),
	
	FIFO_FILL_STATUS			= 0x62,
	    FIFO_FILL_STATUS_FIFO_RD_LVL_shift	= (0),
	    FIFO_FILL_STATUS_FIFO_RD_LVL_mask	= (127 << FIFO_FILL_STATUS_FIFO_RD_LVL_shift),
};

static int tristar_write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = { reg, value };
    return iic_write(TRISTAR_IIC_BUS, CBTL1610_ADDR_W, data, sizeof(data));
}

static int tristar_read(uint8_t reg, uint8_t *data)
{
    return iic_read(TRISTAR_IIC_BUS, CBTL1610_ADDR_R, &reg, sizeof(reg), data, sizeof(uint8_t), IIC_NORMAL);
}

bool cbtl1610_set_usb_brick_detect(int select)
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

bool cbtl1610_read_id(uint8_t digital_id[6])
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
    if (tristar_read(FIFO0, &resp) != 0) {
	return false;
    }

    if (resp != 0x75) {
	return false;
    }

    // read ID
    for (int i = 0; i < 6; i++) {
	if (tristar_read(FIFO0 + i + 1, &digital_id[i])) {
	    return false;
	}
    }

    return true;
}

bool cbtl1610_enable_acc_pwr(bool enabled)
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

#if WITH_HW_TRISTAR_THS7383

extern bool target_has_tristar2(void);
extern bool ths7383_set_usb_brick_detect(int select);
extern bool ths7383_read_id(uint8_t digital_id[6]);
extern bool ths7383_enable_acc_pwr(bool enabled);

bool tristar_set_usb_brick_detect(int select)
{
    return target_has_tristar2() ? cbtl1610_set_usb_brick_detect(select) : ths7383_set_usb_brick_detect(select);
}

bool tristar_read_id(uint8_t digital_id[6])
{
    return target_has_tristar2() ? cbtl1610_read_id(digital_id) : ths7383_read_id(digital_id);
}

bool tristar_enable_acc_pwr(bool enabled)
{
    return target_has_tristar2() ? cbtl1610_enable_acc_pwr(enabled) : ths7383_enable_acc_pwr(enabled);
}

#else // !WITH_HW_TRISTAR_THS7383

bool tristar_set_usb_brick_detect(int select)
{
    return cbtl1610_set_usb_brick_detect(select);
}

bool tristar_read_id(uint8_t digital_id[6])
{
    return cbtl1610_read_id(digital_id);
}

bool tristar_enable_acc_pwr(bool enabled)
{
    return cbtl1610_enable_acc_pwr(enabled);
}

#endif // WITH_HW_TRISTAR_THS7383