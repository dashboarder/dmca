/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
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
#include <drivers/charger.h>
#include <drivers/gasgauge.h>
#include <platform.h>
#include <platform/gpiodef.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/menu.h>
#include <sys/task.h>
#include <target/powerconfig.h>

extern void pmu_charger_set_charging(int dock, uint32_t input_current_ma, uint32_t *charge_current_ma);
extern void pmu_charger_print_status(void);

#define IDT_ADDR_R				0x72
#define IDT_ADDR_W				0x73

// indirect access registers
enum {
	I2C_IndData				= 0x8800,
	I2C_IndAddr_16bH			= 0x8801,
	I2C_IndAddr_16bL			= 0x8802,
	I2C_IndControl				= 0x8803,
	    I2C_IndControl_WriteSingle		= 0x01,
	    I2C_IndControl_ReadSingle		= 0x02,
	    I2C_IndControl_Write3Bytes		= 0x03,
	I2C_IndStatus				= 0x8804,
	    I2C_IndStatus_Busy_flag		= (1 << 0),
	    I2C_IndStatus_Result		= (1 << 1),
};

// memory locations
enum {
	// Accessible locations
    
	Mem_CustomerID				= 0x00D0,
	Mem_FWMajorRevHigh			= 0x00D2,
	Mem_FWMajorRevLow			= 0x00D3,
	Mem_FWMinorRevHigh			= 0x00D4,
	Mem_FWMinorRevLow			= 0x00D5,
	// 0x00D6-0x00DB reserved for data communications
	Mem_Status				= 0x00DC,
	    Mem_Status_Reset			= (1 << 7),
	    Mem_Status_DataComm			= (1 << 6),
	    Mem_Status_DOCK_PWR_EN		= (1 << 4),
	    Mem_Status_Vrect			= (1 << 0),
	Mem_Control				= 0x00DD,
	    Mem_Control_DataCommRequest		= (1 << 7),
	    Mem_Control_DataCommReset		= (1 << 6),
	    Mem_Control_DOCK_PWR_EN		= (1 << 4),
	    Mem_Control_EndOfPower		= (1 << 0),
	Mem_MeasuredVrect			= 0x00DE,
	Mem_TargetVrectHigh			= 0x00DF,
	Mem_TargetVrectLow			= 0x00E0,
	Mem_FWUpdateKey				= 0x00E1,
	    Mem_FWUpdateKeyUpdateRequest	= 0xA5,
	    Mem_FWUpdateKeyStartExecution	= 0x5A,
	Mem_FWUpdateChecksumHigh		= 0x00E2,
	Mem_FWUpdateChecksumLow			= 0x00E3,
    
	// OTP parameters
	OTP_ConfigFlags				= 0xA840,
	OTP_WakeCntrTO				= 0xA841,
	OTP_IDHeaderByte			= 0xA842,
	OTP_IDMessageByte			= 0xA843,
	OTP_IDResponseByte			= 0xA844,
	OTP_IDBypassResponseByte		= 0xA845,
	OTP_InitVrectTargetHigh			= 0xA84D,
	OTP_InitVrectTargetLow			= 0xA84E,
};

static bool idt_wait_busy(void)
{
	uint8_t status_reg[2] = { (I2C_IndStatus >> 8) & 0xFF, I2C_IndStatus & 0xFF };
	uint8_t status_data;
	for (;;) {
	    if (iic_read(CHARGER_IIC_BUS, IDT_ADDR_R, &status_reg, sizeof(status_reg), &status_data, sizeof(status_data), IIC_NORMAL) != 0) return false;
	    if ((status_data & I2C_IndStatus_Busy_flag) == 0) break;
	    task_sleep(100);
	}
	if ((status_data & I2C_IndStatus_Result) == 0) {
	    dprintf(DEBUG_CRITICAL, "charger result status: failed\n");
	    return false;
	}
	return true;
}

static int idt_write_multiple(uint16_t mem, const uint8_t *value, size_t length)
{
    size_t offset = 0;
    while (offset < length) {
	uint8_t data[6] = { (I2C_IndData >> 8) & 0xFF, I2C_IndData & 0xFF };
	if (offset == 0 || (length - offset) < 3) {
	    data[2] = value[offset];
	    data[3] = ((mem + offset) >> 8) & 0xFF;
	    data[4] = (mem + offset) & 0xFF;
	    data[5] = I2C_IndControl_WriteSingle;
	    offset += 1;
	} else {
	    data[2] = value[offset];
	    data[3] = value[offset+1];
	    data[4] = value[offset+2];
	    data[5] = I2C_IndControl_Write3Bytes;
	    offset += 3;
	}
	if (iic_write(CHARGER_IIC_BUS, IDT_ADDR_W, data, sizeof(data)) != 0) return -1;
	if (!idt_wait_busy()) return -1;
    }
    return 0;
}

static int idt_write_byte(uint16_t mem, uint8_t value) {
    return idt_write_multiple(mem, &value, sizeof(value));
}

static int idt_read(uint16_t mem, uint8_t *value)
{
    uint8_t data[5] = { (I2C_IndAddr_16bH >> 8) & 0xFF, I2C_IndAddr_16bH & 0xFF, (mem >> 8) & 0xFF, mem & 0xFF, I2C_IndControl_ReadSingle };
    if (iic_write(CHARGER_IIC_BUS, IDT_ADDR_W, data, sizeof(data)) != 0) return -1;
    if (!idt_wait_busy()) return -1;
    uint8_t reg[2] = { (I2C_IndData >> 8) & 0xFF, I2C_IndData & 0xFF };
    return iic_read(CHARGER_IIC_BUS, IDT_ADDR_R, &reg, sizeof(reg), value, sizeof(uint8_t), IIC_NORMAL);
}

static int idt_write16(uint16_t mem, uint16_t value)
{
    uint8_t data[2] = { (value >> 8) & 0xFF, value & 0xFF };
    return idt_write_multiple(mem, data, sizeof(data));
}

static int idt_read16(uint16_t mem, uint16_t *value)
{
    uint8_t hi1, hi2, low;
    do {
	if (idt_read(mem, &hi1) != 0) return -1;
	if (idt_read(mem + 1, &low) != 0) return -1;
	if (idt_read(mem, &hi2) != 0) return -1;
    } while (hi1 != hi2);
    *value = (hi1 << 8) | low;
    return 0;
}

static int charger_read_vrect(uint32_t *milliVolts)
{
    uint8_t data;
    if (idt_read(Mem_MeasuredVrect, &data) != 0) return -1;
    *milliVolts = ((data * 8) * 24500) / 4095;
    return 0;
}

bool charger_has_external(int dock)
{
    uint8_t status;
    
    // ignore failures, since that just means nothing's connected
    if (idt_read(Mem_Status, &status) != 0) return false;
    
    return (status & Mem_Status_DOCK_PWR_EN) != 0;
}

void charger_set_charging(int dock, uint32_t input_current_ma, uint32_t *charge_current_ma)
{
    // call through to PMU to configure charger
    pmu_charger_set_charging(dock, input_current_ma, charge_current_ma);

    // do not touch 9022, since we assume iBoot does only open-loop high-voltage charging before handing off to OS or battery trap.
}
    
void charger_print_status(void)
{
    pmu_charger_print_status();
    
    uint8_t status;
    if (idt_read(Mem_Status, &status) != 0) {
	dprintf(DEBUG_CRITICAL, "charger: error reading status\n");
	return;
    }
    
    uint32_t vrect;
    if (charger_read_vrect(&vrect) != 0) {
	dprintf(DEBUG_CRITICAL, "charger: error reading vrect\n");
    }
    
    printf("charger: status %02x vrect measured %u mV\n", status, vrect);
}
