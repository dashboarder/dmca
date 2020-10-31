/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <debug.h>
#include <drivers/gasgauge.h>
#include <sys/task.h>
#include <drivers/uart.h>
#include <sys/menu.h>
#include <drivers/power.h>
#include <drivers/charger.h>
#include <lib/env.h>
#include <lib/nvram.h>

#include <target.h>
#include <target/uartconfig.h>
#include <target/gpiodef.h>
#include <target/powerconfig.h>

enum
{
    kHDQRegControl			= 0x00,
    kHDQRegAtRate			= 0x02,	    // pre-Rev A
    kHDQRegDebug1			= 0x02,	    // Rev A FW
    kHDQRegAtRateTimeToEmpty		= 0x04,	    // pre-Rev A FW
    kHDQRegDebug2			= 0x04,	    // Rev A FW
    kHDQRegTemperature			= 0x06,
    kHDQRegVoltage			= 0x08,
    kHDQRegFlags			= 0x0a,
    kHDQRegNominalAvailableCapacity	= 0x0c,
    kHDQRegFullAvailableCapacity	= 0x0e,
    kHDQRegRemainingCapacity		= 0x10,
    kHDQRegFullChargeCapacity		= 0x12,
    kHDQRegAverageCurrent		= 0x14,
    kHDQRegTimeToEmpty			= 0x16,
    kHDQRegTimeToFull			= 0x18,
    kHDQRegStandbyCurrent		= 0x1a,
    kHDQRegStandbyTimeToEmpty		= 0x1c,	    // pre-Rev A FW
    kHDQRegPresentDOD			= 0x1c,	    // Rev A FW
    kHDQRegMaxLoadCurrent		= 0x1e,
    kHDQRegMaxLoadTimeToEmpty		= 0x20,	    // pre-Rev A FW
    kHDQRegInternal_Temp		= 0x20,	    // Rev A FW
    kHDQRegAvailableEnergy		= 0x22,
    kHDQRegAveragePower			= 0x24,
    kHDQRegTimeToEmptyAtConstantPower	= 0x26,	    // pre-Rev A FW
    kHDQRegOCV_Current			= 0x26,	    // Rev A FW
    kHDQRegOCV_Voltage			= 0x28,	    // Rev A FW
    kHDQRegCycleCount			= 0x2a,
    kHDQRegStateOfCharge		= 0x2c,
    kHDQRegRemCapOverUnder		= 0x2e,	    // Rev A FW
    kHDQRegDODatEOC			= 0x30,	    // Rev A FW
    kHDQRegTrueRemainingCapacity	= 0x32,	    // Rev A FW
    kHDQRegPassedCharge			= 0x34,	    // Rev A FW
    kHDQRegDOD0				= 0x36,	    // Rev A FW
    kHDQRegQstart			= 0x38,	    // Rev A FW
    
    kHDQRegPackConfiguration		= 0x3a,
    kHDQRegDesignCapacity		= 0x3c,

    kHDQRegDataFlashClass		= 0x3e,
    kHDQRegDataFlashBlock		= 0x3f,
    kHDQRegBlockData			= 0x40,
    kHDQRegBlockDataChecksum		= 0x60,
    kHDQRegBlockDataControl		= 0x61,

    kHDQRegNameLen			= 0x62,
    kHDQRegName				= 0x63,
    kHDQResScale			= 0x6a,	    // Rev C FW
    kHDQIMAX                            = 0x6c,     // enabled in PackConfigurationB (c=64,o=2)
    kHDQITMiscStatus                    = 0x6e,
};

enum {
    kHDQNotChargingReason       = 0x68, // A4 Only
    kHDQNominalChargeCapacity   = 0x72, // A4 Only
    kHDQChargerStatus           = 0x74, // A4 Only, Write Only
    kHDQChargingCurrent         = 0x75, // A4 Only
    kHDQChargingVoltage         = 0x76, // A4 Only
    kHDQChargerAlert            = 0x77, // A4 Only
};

enum
{
    kHDQControlRegStatus		= 0x00,
    kHDQControlRegDeviceType		= 0x01,
    kHDQControlRegFWVersion		= 0x02,
    kHDQControlRegHWVersion		= 0x03,
    kHDQControlRegDFChecksum		= 0x04,
    kHDQControlRegResetData		= 0x05,
    kHDQControlRegPrevMacWrite		= 0x07,
    kHDQControlRegChemID		= 0x08,
    kHDQControlRegSetFullsleep		= 0x10,
    kHDQControlRegSetHibernate		= 0x11,
    kHDQControlRegClearHibernate	= 0x12,
    kHDQControlRegSetShutdown		= 0x13,		// pre-Rev C FW
    kHDQControlRegClearShutdown		= 0x14,		// pre-Rev C FW
    kHDQControlRegClearFullsleep	= 0x13,		// Rev C FW
    kHDQControlRegSetHDQIntEn		= 0x15,
    kHDQControlRegClearHDQIntEn		= 0x16,
    kHDQControlRegOpenProtector		= 0x17,		// Rev B FW
    kHDQControlRegEnableDLog		= 0x18,		// Rev B FW
    kHDQControlRegDisableDLog		= 0x19,		// Rev B FW
    kHDQControlRegDFCSAll		= 0x1A,		// Rev B FW
    kHDQControlRegDFCSStaticChecm	= 0x1B,		// Rev B FW
    kHDQCOntrolRegDFCSAllStatic		= 0x1c,		// Rev B FW
    kHDQControlRegSealed		= 0x20,
    kHDQControlRegITEnable		= 0x21,
    kHDQCOntrolRegEnableFVCA		= 0x23,		// Rev C FW
    kHDQControlRegCalMode		= 0x40,
    kHDQControlRegReset			= 0x41,
};

enum {
    kHDQLifetimeBlockA                  = 0x04,
    kHDQLifetimeBlockB                  = 0x06,
};

enum {
    kHDQDataSubclassGasGaugingITCfg = 80,
    kHDQDataOffsetGasGaugingITCfgSecRelaxTime = 6,
    kHDQDataOffsetGasGaugingITCfgAverageTime = 12,
    kHDQDataOffsetGasGaugingITCfgMinDODResUpdate = 19,
    kHDQDataOffsetGasGaugingITCfgMaxResFactor = 21,
    kHDQDataOffsetGasGaugingITCfgMinResFactor = 22,
    kHDQDataOffsetGasGaugingITCfgQmaxCapacityError = 45,
    kHDQDataOffsetGasGaugingITCfgMaxQmaxChange = 46,

    kHDQDataSubclassGasGaugingState = 82,
    kHDQDataOffsetGasGaugingStateAvgPLastRun = 9,
    
    kHDQDataSubclassRaTablesData = 88,
    kHDQDataSubclassRaTablesDataX = 89,
    
    kHDQDataSealedModeSubclass = -1,
    kHDQDataSealedModeBlockManufacturerInfoBlockA = 1,
    kHDQDataSealedModeBlockManufacturerInfoBlockB = 2,
    kHDQDataSealedModeBlockManufacturerInfoBlockC = 3,
    kHDQDataSealedModeBlockLifetimeData		  = 4,

    kHDQDataOffsetBlockManufacturerInfoBlockCFlags = 0,
    kHDQDataOffsetBlockManufacturerInfoBlockCUpdateCount = 1,
    kHDQDataOffsetBlockManufacturerInfoBlockCUpdateNeedRA = 2,
};

enum {
    kHDQControlRegStatusMaskSE		= (1 << 15),
    kHDQControlRegStatusMaskFAS		= (1 << 14),
    kHDQControlRegStatusMaskSS		= (1 << 13),
    kHDQControlRegStatusMaskCSV		= (1 << 12),
    kHDQControlRegStatusMaskCCA		= (1 << 11),
    kHDQControlRegStatusMaskBCA		= (1 << 10),
    kHDQControlRegStatusMaskDLOGEN	= (1 << 9),
    kHDQControlRegStatusMaskHDQIntEn	= (1 << 8),
    kHDQControlRegStatusMaskSHUTDOWN	= (1 << 7),
    kHDQControlRegStatusMaskHIBERNATE	= (1 << 6),
    kHDQControlRegStatusMaskFULLSLEEP	= (1 << 5),
    kHDQControlRegStatusMaskSLEEP	= (1 << 4),
    kHDQControlRegStatusMaskLDMD	= (1 << 3),
    kHDQControlRegStatusMaskRUP_DIS	= (1 << 2),
    kHDQControlRegStatusMaskVOK		= (1 << 1),
    kHDQControlRegStatusMaskQEN		= (1 << 0),
};

enum {
    kHDQNotChargingReasonMaskCHG_WD     = (1<<5),
    kHDQNotChargingReasonMaskTOO_LONG   = (1<<4),    
};

#define FW_VERSION_NO_SOC1  (0x0109)
#define FW_VERSION_REV_A    (0x0119)
#define FW_VERSION_REV_B    (0x0132)
#define FW_VERSION_REV_C    (0x0310)
#define FW_VERSION_REV_D    (0x0313)
#define FW_VERSION_REV_A4   (0x0501)

#if defined(BATTERY_TRAP_DEBUG)
enum RegisterType {
    kRegisterTypeNone=-1,
    kRegisterTypeS16=0, // do not change, default value
    kRegisterTypeU16=1,
    kRegisterTypeU8=2,
    kRegisterTypeUC16=10,
};

struct RegisterInfo
{
    uint32_t reg;
    const char * name[5];   /* PreREVA,REVA,REVB,REVD,A4 */
    enum RegisterType type;
    //void *data;
    //rfun_t fun;
    //char *regName;      // this is the true name
};

static const struct RegisterInfo infoRegs[] =
{
    { kHDQRegAtRate,                        { "AtRate", "Debug1", "DataLogIndex" }, kRegisterTypeS16},
    // since this register is dependent on the previous register (all firmware versions) and
    // is not safe to read (rev B), do not print it out when dumping the device.
    //  { kHDQRegAtRateTimeToEmpty,         "AtRateTimeToEmpty", "Debug2", "DataLogBuffer" },
    { kHDQRegTemperature,                   { "Temperature" }, kRegisterTypeU16},
    { kHDQRegVoltage,                       { "Voltage" }, kRegisterTypeS16},
    { kHDQRegFlags,                         { "Flags" }, kRegisterTypeS16},
    { kHDQRegNominalAvailableCapacity,      { "NominalAvailableCapacity" }, kRegisterTypeS16},
    { kHDQRegFullAvailableCapacity,         { "FullAvailableCapacity" }, kRegisterTypeS16},
    { kHDQRegRemainingCapacity,             { "RemainingCapacity" }, kRegisterTypeS16},
    { kHDQRegFullChargeCapacity,            { "FullChargeCapacity" }, kRegisterTypeS16},
    { kHDQRegAverageCurrent,                { "AverageCurrent" }, kRegisterTypeS16},
    { kHDQRegTimeToEmpty,                   { "TimeToEmpty" }, kRegisterTypeU16},
    { kHDQRegTimeToFull,                    { "TimeToFull" }, kRegisterTypeU16},
    { kHDQRegStandbyCurrent,                { "StandbyCurrent" }, kRegisterTypeS16}, // not on A4
    { kHDQRegStandbyTimeToEmpty,            { "StandbyTimeToEmpty", "PresentDOD"    }, kRegisterTypeU16},
    { kHDQRegMaxLoadCurrent,                { "MaxLoadCurrent" }, kRegisterTypeS16}, // not on A4
    { kHDQRegMaxLoadTimeToEmpty,            { "MaxLoadTimeToEmpty", "Internal_Temp" }, kRegisterTypeU16},
    { kHDQRegAvailableEnergy,               { "AvailableEnergy", NULL, "Qmax" }, kRegisterTypeU16},
    { kHDQRegAveragePower,                  { "AveragePower" }, kRegisterTypeU16},
    { kHDQRegTimeToEmptyAtConstantPower,    { "TimeToEmptyAtConstantPower", "OCV_Current" }, kRegisterTypeS16},
    { kHDQRegOCV_Voltage,                   { NULL, "OCV_Voltage" }, kRegisterTypeS16},
    { kHDQRegCycleCount,                    { "CycleCount" }, kRegisterTypeU16},
    { kHDQRegStateOfCharge,                 { "StateOfCharge" }, kRegisterTypeS16},
    { kHDQRegRemCapOverUnder,               { NULL, "RemCapOverUnder" }, kRegisterTypeS16},
    { kHDQRegDODatEOC,                      { NULL, "DODatEOC" }, kRegisterTypeS16},
    { kHDQRegTrueRemainingCapacity,         { NULL, "TrueRemainingCapacity" }, kRegisterTypeS16},
    { kHDQRegPassedCharge,                  { NULL, "PassedCharge" }, kRegisterTypeS16},
    { kHDQRegDOD0,                          { NULL, "DOD0" }, kRegisterTypeU16},
    { kHDQRegQstart,                        { NULL, "Qstart" }, kRegisterTypeS16},
    { kHDQRegDesignCapacity,                { "DesignCapacity" }, kRegisterTypeS16},
    { kHDQIMAX,                             { NULL, NULL, NULL, "IMAX" }, kRegisterTypeS16},

    // available only in firmware A4
    { kHDQITMiscStatus,                     { NULL, NULL, NULL, NULL, "ITMiscStatus" }, kRegisterTypeS16},
    { kHDQNominalChargeCapacity,            { NULL, NULL, NULL, NULL, "NCC" }, kRegisterTypeS16},

    { kHDQControlRegStatus,                 { "Status" },       kRegisterTypeUC16 },
    { kHDQControlRegDeviceType,             { "DeviceType" },   kRegisterTypeUC16 },
    { kHDQControlRegFWVersion,              { "FWVersion" },    kRegisterTypeUC16 },
    { kHDQControlRegHWVersion,              { "HWVersion" },    kRegisterTypeUC16 },
    { kHDQControlRegChemID,                 { "ChemID" },       kRegisterTypeUC16 },

    { 0, { }, kRegisterTypeNone }
};

// available only in firmware A4
static const struct RegisterInfo chargerRegs[] =
{
    { kHDQChargerStatus,                    { NULL, NULL, NULL, NULL, "ChargerStatus" }, kRegisterTypeU8 },
    { kHDQChargingCurrent,                  { NULL, NULL, NULL, NULL, "ChargingCurrent" }, kRegisterTypeU8 },
    { kHDQChargingVoltage,                  { NULL, NULL, NULL, NULL, "ChargingVoltage" }, kRegisterTypeU8 },
    { kHDQNotChargingReason,                { NULL, NULL, NULL, NULL, "NotChargingReason" }, kRegisterTypeU8 },
    { kHDQChargerAlert,                     { NULL, NULL, NULL, NULL, "ChargerAlert" }, kRegisterTypeU8 },

    { 0, { }, kRegisterTypeNone }
};
#endif

static int
uart_break()
{
    uart_send_break(HDQGAUGE_SERIAL_PORT, true);
    task_sleep(200);
    uart_send_break(HDQGAUGE_SERIAL_PORT, false);

    task_sleep(40);

    while (uart_getc(HDQGAUGE_SERIAL_PORT, false) >= 0)
	;

    return (0);
}

static int 
uart_write(const unsigned char * data, int len)
{
    for (int idx = 0; idx < len; idx++)
	if (uart_putc(HDQGAUGE_SERIAL_PORT, data[idx]) < 0)
	    return -1;

    return 0;
}

static int 
uart_read(unsigned char * data, int len, int timeoutusecs)
{
    utime_t timeout = system_time() + timeoutusecs;

    int idx = 0;
    
    while (idx < len)
    {
	int c = uart_getc(HDQGAUGE_SERIAL_PORT, false);
	
	if (c < 0)
	{
	    if (system_time() > timeout) {
		printf("gas gauge read timed out\n");
		return (-1);
	    }

	    task_sleep(190);
	}
	else
	{
	    data[idx++] = c;

	    timeout = system_time() + timeoutusecs;
	}
    }

    return (len);
}


#define SERIAL_LOG  0

#if SERIAL_LOG
static void
dumpBuffer(unsigned char * buffer, int length)
{
    int i;
    for (i = 0; i < length; i++)
	printf(" %02x", buffer[i]);
    printf("\n");
}
#endif

static int
hdqOp(int command, int commandBits, int readBits)
{
    unsigned char buffer[32];
    int rc, i;
    int data;

    if ((commandBits > 32) || (readBits > 32))
	return (-1);

    for (i = 0; i < commandBits; i++)
	buffer[i] = (command & (1 << i)) ? 0xfe : 0xc0;

#if SERIAL_LOG
    printf("\tsend: %x\n", command);
    dumpBuffer(buffer, commandBits);
#endif

    uart_break();

    rc = uart_write(buffer, commandBits);
    if (rc < 0)
	return (rc);

    bzero(buffer, commandBits);
    rc = uart_read(buffer, commandBits, 500000);
#if SERIAL_LOG
    printf("\tread8 (%d)", rc);
    dumpBuffer(buffer, commandBits);
#endif
    if (rc < 0)
	return (rc);
    for (i = 0; i < commandBits; i++)
    {
	if (buffer[i] != ((command & (1 << i)) ? 0xfe : 0xc0))
	{
	    printf("hdq read mismatch[%d]\n", i);
	    return (-1);
	}
    }

    data = 0;
    if (readBits)
    {
	bzero(buffer, readBits);
	rc = uart_read(buffer, readBits, 500000);
#if SERIAL_LOG
	printf("\tread8 (%d)", rc);
	dumpBuffer(buffer, readBits);
#endif
	if (rc < 0)
	    return (rc);
	for (i = 0; i < readBits; i++)
	{
	    if (buffer[i] > 0xF8)
		data |= (1 << i);
	}
    }

    return (data);
}

static int
hdqRead8(int command)
{
    return (hdqOp(command, 8, 8));
}

static int
hdqRead16(int command)
{
    int msb, msb2, lsb, result, retries;

    retries = 0;
    result = -1;
    do
    {
	msb = hdqRead8(command + 1);
	if (msb < 0)
	    break;
	lsb = hdqRead8(command);
	if (lsb < 0)
	    break;
	msb2 = hdqRead8(command + 1);
	if (msb2 < 0)
	    break;
	if (msb2 == msb)
	    result = (lsb | (msb << 8));
    }
    while ((result < 0) && (++retries < 10));

    return (result);
}

static int
hdqWrite8(int command, int data)
{
    command &= 0xFF;
    command |= 0x80;
    command |= ((data & 0xFF) << 8);

    return (hdqOp(command, 16, 0));
}

static int
controlOp16(int reg)
{
    int rc;

    rc = hdqWrite8(0, reg & 0xff);
    if (rc < 0)
	return (rc);
    rc = hdqWrite8(1, reg >> 8);
    if (rc < 0)
	return (rc);

    return 0;
}

static int
controlRead16(int reg) 
{
    int rc;

    rc = hdqWrite8(0, reg & 0xff);
    if (rc < 0)
	return (rc);
    rc = hdqWrite8(1, reg >> 8);
    if (rc < 0)
	return (rc);

    return (hdqRead16(0));
}

static int
controlWrite16(int subcommand, int data)
{
    int rc;
    
    rc = hdqWrite8(kHDQRegControl, (subcommand & 0xFF));
    if (rc < 0)
	return (rc);
    rc = hdqWrite8(kHDQRegControl + 1, ((subcommand >> 8) & 0xFF));
    if (rc < 0)
	return (rc);

    rc = hdqWrite8(kHDQRegControl, (data & 0xFF));
    if (rc < 0)
	return (rc);
    rc = hdqWrite8(kHDQRegControl + 1, ((data >> 8) & 0xFF));

    return (rc);
}

static int 
readBlock(int class, int block, unsigned char *blockData)
{
    unsigned char blockDataLocal[32];
    unsigned char sum = 0, cksum;
    int rc;
    int idx, len, value;
    bool sealedMode = (class == kHDQDataSealedModeSubclass);

    if (blockData == NULL) blockData = blockDataLocal;

    rc = hdqWrite8(kHDQRegBlockDataControl, sealedMode ? 1 : 0);
    if (rc < 0)
	return (rc);

    if (!sealedMode) {
	rc = hdqWrite8(kHDQRegDataFlashClass, class);
	if (rc < 0)
	    return (rc);
    }

    len = sizeof(blockDataLocal);
    rc = hdqWrite8(kHDQRegDataFlashBlock, block);
    for (idx = 0; idx < len; idx++)
    {
	value = hdqRead8(kHDQRegBlockData + idx);
	if (value < 0)
	    return value;
	
	blockData[idx] = value;
	sum += value;
    }
    cksum = hdqRead8(kHDQRegBlockDataChecksum);
    sum += cksum;

    if (sum != 0xff)
	return -1;

    return (0);
}

static int
writeBlock(int class, int block, const unsigned char *blockData)
{
    unsigned char sum = 0, cksum;
    int rc, idx;
    bool sealedMode = (class == kHDQDataSealedModeSubclass);

    rc = hdqWrite8(kHDQRegBlockDataControl, sealedMode ? 1 : 0);
    if (rc < 0)	return (rc);

    if (!sealedMode) {
	rc = hdqWrite8(kHDQRegDataFlashClass, class);
	if (rc < 0) return (rc);
    }

    rc = hdqWrite8(kHDQRegDataFlashBlock, block);

    for (idx = 0; idx < 32; idx++)
    {
	rc = hdqWrite8(kHDQRegBlockData + idx, blockData[idx]);
	if (rc != 0) return rc;

	sum += blockData[idx];
    }
    cksum = 0xff - sum;
    rc = hdqWrite8(kHDQRegBlockDataChecksum, cksum);
    
    task_sleep(200 * 1000);
    
    return rc;
}

static uint16_t gg_firmware_version = 0;

static void gasgauge_reset(int hard)
{
#ifdef POWER_GPIO_BATTERY_SWI
    power_gpio_configure(POWER_GPIO_BATTERY_SWI, POWER_GPIO_BATTERY_SWI_CONFIG_OUTPUT);

    if ( hard ) {
	// SWI low for 3s
	power_set_gpio(POWER_GPIO_BATTERY_SWI, 1, 0);
	task_sleep(3000 * 1000);
    }
   
    // SWI high for 500ms
    power_set_gpio(POWER_GPIO_BATTERY_SWI, 1, 1);
    task_sleep(500 * 1000);

    // reset to input
    power_gpio_configure(POWER_GPIO_BATTERY_SWI, POWER_GPIO_BATTERY_SWI_CONFIG_INPUT);
    power_set_gpio(POWER_GPIO_BATTERY_SWI, 0, 0);

    // allow time for power-on
    task_sleep(500 * 1000);
#endif
}

static void gasgauge_check_reset(void)
{
#ifdef POWER_GPIO_BATTERY_SWI
    if (power_get_gpio(POWER_GPIO_BATTERY_SWI) == 0) {
	dprintf(DEBUG_CRITICAL, "gas gauge SWI line low: issuing reset\n");
	gasgauge_reset(0);
    }
#endif
}

static bool
gg_open(void) {
#if WITH_HW_CHARGER_GG_IF
    return charger_set_gasgauge_interface(true) == 0;
#endif
    return true;
}

static void
gg_close(void) {
#if WITH_HW_CHARGER_GG_IF
    charger_set_gasgauge_interface(false);
#endif
}

void gasgauge_init(void)
{
    // try not to talk to the gas gauge if we might be in sleep
    // N.B. if we were suspended, we would not have placed the gauge
    // into full sleep mode.
    if (!power_is_suspended())
    {
	if (!gg_open()) return;
    
	gasgauge_check_reset();
	uart_break();

	// wait for HDQ comms engine to start up
	task_sleep(10 * 1000);

	if (gg_firmware_version == 0) {
	    gg_firmware_version = controlRead16(kHDQControlRegFWVersion);
	}

	if (gg_firmware_version >= FW_VERSION_REV_C) {
	    controlOp16(kHDQControlRegClearFullsleep);
	}
	
	gg_close();
    }
}

// 0 success, 1 timeout, -1 error
int gasgauge_reset_timer(int debug_print_level, unsigned int timeout)
{
    int success=0;

#if WITH_HW_CHARGER_GG_IF
    if (!gg_open()) return -1;

    gasgauge_check_reset();
    uart_break();

    if ( gg_firmware_version==0 ) gg_firmware_version = controlRead16(kHDQControlRegFWVersion);
    if ( gg_firmware_version>=FW_VERSION_REV_A4 ) {
        int ncr;

        dprintf(debug_print_level, "reset_timer (%d):", timeout);
        
        for ( timeout-- ; 1 ; timeout-- ) {
            task_sleep( 1000*1000 ); // one second

            ncr = hdqRead8(kHDQNotChargingReason);
            dprintf(debug_print_level, " %#x=%#x", kHDQNotChargingReason, ncr);

            if ( (ncr&kHDQNotChargingReasonMaskTOO_LONG)==0 ) break;// cond cleared
            if ( (ncr&kHDQNotChargingReasonMaskCHG_WD)!=0 ) break;  // same as clear cond
            if ( timeout==0 ) { success=1; break; }

        }

        dprintf(debug_print_level, "\n");
    }

    gg_close();
#endif

    return success;
}

#if defined(BATTERY_TRAP_DEBUG)
/* ported from iOS hdqtool */
static const char* register_name(const struct RegisterInfo * desc, uint16_t fwVersion)
{
    if ( fwVersion==0 ) return NULL;

    if ( fwVersion >= FW_VERSION_REV_A4 ) {
        if ( desc->reg==kHDQRegStandbyCurrent ) return NULL;
        if ( desc->reg==kHDQRegMaxLoadCurrent ) return NULL;
    }

    if ( desc->name[4] && (fwVersion >= FW_VERSION_REV_A4) )
        return desc->name[4];
    if ( desc->name[3] && (fwVersion >= FW_VERSION_REV_D) )
    	return desc->name[3];
    // no name changes between REVB and REVC
    if ( desc->name[2] && (fwVersion >= FW_VERSION_REV_B) )
        return desc->name[2];
    if ( desc->name[1] && (fwVersion >= FW_VERSION_REV_A) )
        return desc->name[1];

    return desc->name[0];   // pre REVA
}

static int read_register(int *data, const struct RegisterInfo * desc)
{
    int result;

    switch ( desc->type ) {
        case kRegisterTypeU8: result=hdqRead8(desc->reg); break;
        case kRegisterTypeS16: result=hdqRead16(desc->reg); break;
        case kRegisterTypeU16: result=hdqRead16(desc->reg); break;
        case kRegisterTypeUC16: result=controlRead16(desc->reg); break;
        case kRegisterTypeNone: return -1;
        default: return -1;
    }
    if (result >= 0) {
	if (desc->type == kRegisterTypeS16) {
	    *data = (int32_t)((int16_t)result);
	} else {
	    *data = result;
	}
    }

    return result;
}

static int dump_device(const struct RegisterInfo *reglist, char * title)
{
#if 0

    if (gg_firmware_version>=FW_VERSION_REV_A4) {
        dprintf(DEBUG_CRITICAL, "Name: not available\n");
    } else {
        char name[16] = { 0 };

        int len = hdqRead8(kHDQRegNameLen);
        if ( len<0 )
            return len;
        if (len >= (int)sizeof(name))
            len = sizeof(name)-1;
        for (idx = 0; idx < len; idx++) {
            value = hdqRead8(kHDQRegName + idx);
            if ( value<0 )
                return value;
            name[idx]=value&0xff;
        }

        name[idx]=0;
        dprintf(DEBUG_CRITICAL, "Name: %-16s\n", name);
    }
#endif

    printf("%s Registers:\n", title);
    for (int idx = 0; reglist[idx].type!=kRegisterTypeNone ; idx++) {
        int value;

        const char *name=register_name( &reglist[idx], gg_firmware_version);
        if ( !name ) continue;

        const int err=read_register(&value, &reglist[idx]);
        if ( err<0 ) {
            printf("0x%02x: error in reading %s\n", reglist[idx].reg, name);
        } else {
            printf("0x%02x: 0x%04x, %6d", reglist[idx].reg, (uint16_t)value, value);
            printf(" %s\n", (name)?name:"");
        }
    }

    return 0;
}

static void gasgauge_dump(void)
{
    printf("%s, %d:Dumping Gauge Registers\n", __func__, __LINE__);

    if (!gg_open()) return;
        gasgauge_check_reset();
        uart_break();
    if (gg_firmware_version == 0) {
        gg_firmware_version = controlRead16(kHDQControlRegFWVersion);
    }

    dump_device(infoRegs, "Info");
    dump_device(chargerRegs, "Charger");

    gg_close();
}

static int gasgauge_log(print_level)
{
    static utime_t last_time;
    utime_t now = system_time()/1000;
    utime_t delta;

    if (now > last_time)
	delta = now - last_time;
    else
	delta = 0;
    printf("Gas Gauge Precharge timestamp: %us delta %u.%03us\n", ((uint32_t)now/1000), ((uint32_t)delta)/1000, ((uint32_t)delta)%1000); 
    last_time = now;
#if WITH_ENV
    if (env_get_bool("debug-gg", false)) {
	dump_device(infoRegs, "Info");
	dump_device(chargerRegs, "Charger");
	print_level = DEBUG_CRITICAL;
    }
#endif
    return print_level;
}
#endif

void gasgauge_print_status(void)
{
    gasgauge_needs_precharge(DEBUG_CRITICAL, false);
}

// @return 0, no prech?0:1e
// @return 1<<0, needs precharge
// @return 1<<1, needs charger timer reset
// @return -1 error
int gasgauge_needs_precharge(int debug_print_level, bool debug_show_target)
{
    int needs_precharge=0;
    int flags, capacity, current, fcc;
    int temperature, voltage;
    const int gg_flag_mask=target_get_precharge_gg_flag_mask();
    const int boot_battery_capacity=target_get_boot_battery_capacity();
#if WITH_HW_CHARGER_GG_IF
    int ncr, cs, cc, cv, ca; // FW_VERSION_REV_A4 only
#endif

    if (!gg_open()) return false;

    gasgauge_check_reset();
    uart_break();

    flags = hdqRead16(kHDQRegFlags);
    if (flags == -1) goto err;

    if (gg_firmware_version == 0) {
	gg_firmware_version = controlRead16(kHDQControlRegFWVersion);
    }

    if (gg_firmware_version >= FW_VERSION_REV_A) {
	capacity = hdqRead16(kHDQRegTrueRemainingCapacity);
    } else {
	capacity = hdqRead16(kHDQRegRemainingCapacity);
    }
    if (capacity == -1) goto err;

    fcc = hdqRead16(kHDQRegFullChargeCapacity);
    if (fcc == -1) goto err;
    
    current = hdqRead16(kHDQRegAverageCurrent);
    if (current == -1) goto err;
    
    temperature = hdqRead16(kHDQRegTemperature);
    if (temperature == -1) goto err;
    
    voltage = hdqRead16(kHDQRegVoltage);
    if (voltage == -1) goto err;
	
    // read charger fields when available, ignore errors
#if WITH_HW_CHARGER_GG_IF
    if ( gg_firmware_version>=FW_VERSION_REV_A4 ) {
        ncr = hdqRead8(kHDQNotChargingReason);
        cs = hdqRead8(kHDQChargerStatus);
        cc = hdqRead8(kHDQChargingCurrent);
        cv = hdqRead8(kHDQChargingVoltage);
        ca = hdqRead8(kHDQChargerAlert);

        if  ( ncr&kHDQNotChargingReasonMaskTOO_LONG ) needs_precharge|=GG_NEEDS_RESET;
        if  ( ncr&kHDQNotChargingReasonMaskCHG_WD ) needs_precharge|=GG_COMMS_WD;
    }
#endif

#if defined(BATTERY_TRAP_DEBUG)
    debug_print_level = gasgauge_log(debug_print_level);
#endif
    gg_close();
	
    // cast values to (int16_t) to extend sign bit for negative numbers (use int
    // above so -1 error is distinguished from 0xffff value).

    dprintf(debug_print_level, "gg flags %#x", flags);
    if (debug_show_target) dprintf(debug_print_level, " (tgt !& %#x)", gg_flag_mask );
    dprintf(debug_print_level, "; cap %d/%d mAh", (int16_t)capacity, (int16_t)fcc);
    if (debug_show_target) dprintf(debug_print_level, " (tgt >= %d)", boot_battery_capacity );
    dprintf(debug_print_level, "; %d mV; %d mA; %d.%d C\n",
	   (int16_t)voltage, (int16_t)current,
	   ((int16_t)temperature - 2732) / 10,  ((int16_t)temperature - 2732) % 10);

#if WITH_HW_CHARGER_GG_IF
    if ( gg_firmware_version>=FW_VERSION_REV_A4 ) {
        dprintf(DEBUG_CRITICAL, "gg charger: 0x%x=0x%x 0x%x=0x%x 0x%x=0x%x 0x%x=0x%x 0x%x=0x%x\n", 
                kHDQNotChargingReason, ncr,
                kHDQChargerStatus, cs,
                kHDQChargingCurrent, cc,
                kHDQChargingVoltage, cv,
                kHDQChargerAlert, ca);
    }
#endif

    // require precharge if SOC1 is set (OS shutdown condition), but also
    // if pack doesn't have enough charge to make it through OS boot

    if ( ((flags & gg_flag_mask) != 0) || ((int16_t)capacity < boot_battery_capacity ) )
    {
	// Firmware version 1.09: EVT1 battery pack, not to be trusted
	if (gg_firmware_version == FW_VERSION_NO_SOC1) return 0;

        // SOC1 is set, I need precharge
	   needs_precharge|=GG_NEEDS_PRECHARGE;
    }
    
    // precharge and/or needs charger reset or nevermind
    return needs_precharge;
    
err:
    gg_close();
    return 0; // should return error, really
}

void gasgauge_will_shutdown(void)
{
    if (!gg_open()) return;

    gasgauge_check_reset();
    uart_break();

    task_sleep(10 * 1000);

    if (gg_firmware_version == 0) {
	gg_firmware_version = controlRead16(kHDQControlRegFWVersion);
    }

    if (gg_firmware_version >= FW_VERSION_REV_C) {
	controlOp16(kHDQControlRegSetFullsleep);
    }
    
    gg_close();
}

bool gasgauge_full(void)
{
    unsigned int soc;

    const int rc=gasgauge_read_soc( &soc );

    return ( rc==0 ) && (soc == 100);
}

int gasgauge_read_soc(unsigned int *soc)
{
	if (!gg_open()) return -1;

	gasgauge_check_reset();
	uart_break();

	int rc = hdqRead16(kHDQRegStateOfCharge);

	gg_close();

	if (rc == -1) return -1;

	*soc = rc;

	return 0;
}

#ifndef GASGAUGE_BATTERYID_BLOCK
#define GASGAUGE_BATTERYID_BLOCK kHDQDataSealedModeBlockManufacturerInfoBlockB
#endif

int gasgauge_get_battery_id(u_int8_t *buf, int size)
{
    if (!power_has_batterypack())
	return -1;
	
    if(buf == NULL)
    {
	dprintf(DEBUG_INFO, "buffer is NULL\n");
	return -1;
    } 

    if(size > 32)
    {
	dprintf(DEBUG_INFO, "trying to read more than 32 bytes, size: %d\n", size);
	return -1;
    }


    if (!gg_open()) return -1;

    gasgauge_check_reset();
    uart_break();

    int ret = readBlock(kHDQDataSealedModeSubclass, GASGAUGE_BATTERYID_BLOCK, buf);
    
    gg_close();
    
    return ret;
}

struct atv_header {
    uint8_t table_entries_table_format;
    uint8_t checksum;
    uint8_t vi_steps_per_entry_revision;
    uint8_t table_type;
} __attribute__((packed));

struct atv_data_entry {
    uint8_t lower_temperature_limit;
    uint8_t higher_temperature_limit;
    uint8_t current_voltage_value[4];
} __attribute__((packed));

struct atv_table {
    struct atv_header header;
    struct atv_data_entry data_entry[];
} __attribute__((packed));

static inline uint8_t high_nibble(uint8_t value) { return (value >> 4) & 0x0F; }
static inline uint8_t low_nibble(uint8_t value) { return value & 0x0F; }

#if defined(GASGAUGE_CHARGETABLE_BLOCK) && !defined(TARGET_USE_CHARGE_TABLE)
#error gas gauge charge table with no default value
#endif

bool gasgauge_read_charge_table(struct power_charge_limits *table_out, size_t num_elems)
{
    // Returns true if the gas gauge contained a valid charge table and the parameter
    // was modified to include it.  Returns false if the gas gauge did not have
    // a charge table, or it did not fit in the passed-in table size.  If false
    // is returned, the target of the table parameter will not be modified.
    
#ifdef GASGAUGE_CHARGETABLE_BLOCK
    uint8_t chargetable_data[32];
    struct atv_table *chargetable = (struct atv_table *)chargetable_data;
    uint8_t table_entries;
    uint8_t vi_steps_per_entry;

    if (!gg_open()) return false;

    gasgauge_check_reset();
    uart_break();

    bool ok = false;
    int retries = 0;
    
    while (!ok)
    {    
	if ((++retries) >= 2)
	    return false;

	if (readBlock(kHDQDataSealedModeSubclass, GASGAUGE_CHARGETABLE_BLOCK, chargetable_data) != 0)
	    continue;
	      
	// check table type
	if (chargetable->header.table_type != 0xFF) {
	    dprintf(DEBUG_INFO, "gas gauge charge table invalid type: %#x\n", chargetable_data[3]);
	    continue;
	}

	// decode header
	table_entries = high_nibble(chargetable->header.table_entries_table_format);
	vi_steps_per_entry = high_nibble(chargetable->header.vi_steps_per_entry_revision);

	// make sure not to overflow the table, which has room for the header (four bytes) plus
	// up to four entries (six bytes each)
	uint8_t table_bytes = offsetof(struct atv_table, data_entry[table_entries]);

	if (table_bytes > sizeof(chargetable_data)) {
	    dprintf(DEBUG_INFO, "gas gauge charge table inconsistent: %d data entries, %d bytes\n", table_entries, table_bytes);
	    continue;
	}

	// make sure there are enough entries for the input table
	if (num_elems < ((table_entries * vi_steps_per_entry))) {
	    dprintf(DEBUG_INFO, "gas gauge charge table has too many entries: %d only space for %zu\n", (table_entries * vi_steps_per_entry + 1), num_elems);
	    continue;
	}
	    
	// calculate checksum over the data entries (ignore other data in the block)
	uint8_t checksum = 0;
	for (unsigned i = 0; i < table_bytes; i++) {
	    checksum += chargetable_data[i];
	}
	if ((checksum & 0xFF) != 0x00) {
	    dprintf(DEBUG_INFO, "gas gauge charge table bad checksum: checksum %#x checksum byte %#x expecting %#x\n",
		     checksum, chargetable->header.checksum, (256 - ((checksum - chargetable->header.checksum) & 0xFF)));
	    continue;
	}

	ok = true;
    }

    gg_close();

    // fill in the table
    unsigned out_idx = 0;

    // decode each data entry
    for (int data_entry_idx = 0; data_entry_idx < table_entries; data_entry_idx++)
    {
	for (int vi_idx = 0; vi_idx < vi_steps_per_entry; vi_idx++)
	{
	    table_out[out_idx].lowerTempLimit = chargetable->data_entry[data_entry_idx].lower_temperature_limit * 100;
	    table_out[out_idx].upperTempLimit = chargetable->data_entry[data_entry_idx].higher_temperature_limit * 100;

	    // do not limit the last entry in the table based on voltage
	    if ((vi_idx+1) == vi_steps_per_entry)
		table_out[out_idx].upperVoltageLimit = USHRT_MAX;
	    else
		table_out[out_idx].upperVoltageLimit = atv_voltage_limit[low_nibble(chargetable->data_entry[data_entry_idx].current_voltage_value[vi_idx])];

	    table_out[out_idx].currentSetting = atv_current_limit[high_nibble(chargetable->data_entry[data_entry_idx].current_voltage_value[vi_idx])];
	    out_idx++;
	}
    }

    // fill out the rest with zeros
    while (out_idx < num_elems)
    {
	table_out[out_idx].upperVoltageLimit = 0;
	table_out[out_idx].lowerTempLimit = 0;
	table_out[out_idx].upperTempLimit = 0;
	table_out[out_idx].currentSetting = 0;
	out_idx++;
    }
    
#if 0
    for (unsigned i = 0 ; i < num_elems; i++) {
	dprintf(DEBUG_INFO, "\t{ %lu,\t%ld,\t%ld,\t%lu\t},\t\n", table_out[i].upperVoltageLimit, table_out[i].lowerTempLimit, table_out[i].upperTempLimit, table_out[i].currentSetting);
    }
#endif
    
    return true;
#endif
    
    return false;
}

int gasgauge_read_temperature(int *centiCelsius) 
{
	if(0 == centiCelsius) return -1;

	if (!gg_open()) return -1;
	
	gasgauge_check_reset();
	uart_break();
	
	int deciKelvin = hdqRead16(kHDQRegTemperature);
	
	gg_close();
	
	if((deciKelvin < 0) || (deciKelvin > 6000)) {
		printf("hdqgauge temperature bogus: %ddK\n", deciKelvin);
		return -1;
	}
	*centiCelsius = (deciKelvin-2732) * 10;
	return 0;
}

int gasgauge_read_voltage(unsigned int *milliVolt) 
{
	if(0 == milliVolt) return -1;

	if (!gg_open()) return -1;
	
	gasgauge_check_reset();
	uart_break();
	
	unsigned int mV = hdqRead16(kHDQRegVoltage);
	
	gg_close();
	
	if(mV > 6000) {
		printf("hdqgauge voltage bogus: %dmV\n", mV);
		return -1;
	}
	*milliVolt = mV;
	return 0;
}

int gasgauge_read_design_capacity(unsigned int *milliAmpHours) 
{
	if(0 == milliAmpHours) return -1;
	
	if (!gg_open()) return -1;
	
	gasgauge_check_reset();
	uart_break();
	
	int mAh = hdqRead16(kHDQRegDesignCapacity);
	
	gg_close();
	
	if (mAh < 0) return -1;

	*milliAmpHours = mAh;
	return 0;
}

#if GG_CHECK_RA_TABLES
struct ra_table_info_t {
    uint16_t chemID;
    uint16_t R_a[15];
    int16_t avg_p;
};

struct ra_table_info_t ra_table_info[] =
{
    /* ATL */
    { 0x126, { 188, 188, 201, 232, 186, 176, 210, 251, 256, 266, 302, 363, 568, 1029, 1251 }, -955 },
    /* LGC */
    { 0x127, { 193, 193, 208, 252, 235, 186, 211, 247, 281, 287, 273, 369, 833, 1514, 1558 }, -937 },
    /* SONY */
    { 0x128, { 230, 230, 232, 253, 187, 169, 200, 236, 224, 214, 256, 301, 492, 973, 1217 }, -949 },
    /* SDI */
    { 0x129, { 165, 165, 204, 252, 186, 178, 200, 176, 175, 193, 208, 186, 240, 598, 1435 }, -938 },
};
#define NUM_RA_TABLES (sizeof(ra_table_info)/sizeof(ra_table_info[0]))

#define RA_UPDATE_TEMPERATURE (2832) /* 283.2K = 10C */
#define RA_UPDATE_CAPACITY (25) /* percent of design capacity */
#define RA_UPDATE_FW_VERSION (0x0117)

static const uint32_t kHDQUnsealKey = 0x36720414;

static const int kHDQIntervalReset = 3*1000*1000;
static const int kHDQIntervalRelax = 5*1000*1000;

static bool gg_ra_update_done = false;
static char *gg_ra_update_log = NULL;
static const int gg_ra_update_log_size = 2048;

static void ra_log_val(const char *label, int value)
{
    size_t offset = strlen(gg_ra_update_log);
    snprintf(gg_ra_update_log + offset, gg_ra_update_log_size - offset, "%s%s=%d", (offset == 0) ? "" : ",", label, value);
}

static void ra_log_block(const char *label, const uint8_t *block)
{
    size_t offset = strlen(gg_ra_update_log);
    snprintf(gg_ra_update_log + offset, gg_ra_update_log_size - offset, "%s%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	(offset == 0) ? "" : ",", label, block[0], block[1], block[2], block[3], block[4], block[5], block[6], block[7], block[8], block[9], block[10], block[11], block[12], block[13], block[14], block[15],
	block[16], block[17], block[18], block[19], block[20], block[21], block[22], block[23], block[24], block[25], block[26], block[27], block[28], block[29], block[30], block[31]);
}
#endif

bool gasgauge_check_health(unsigned int vbat_mv)
{
#if GG_CHECK_RA_TABLES
    /* If the gas gauge has corrupted resistance tables causing it to report too low
     * capacity values (FullChargeCapacity, RemainingCapacity), the device will exhibit poor
     * battery life and can get stuck in the battery trap forever.  Attempt to detect this case,
     * restore the resistance tables to their factory defaults, and hope this fixes the issue.
     */

    // only attempt once per boot
    dprintf(DEBUG_SPEW, "gasgauge_check_health: gg_ra_update_done=%d\n", gg_ra_update_done);
    if (gg_ra_update_done)
	return false;

    // check that the voltage is high enough that:
    // - we should have exited by now (e.g., if we're at 3.8V and still in the trap, something is wrong)
    // - we have more than enough charge to perform the update without browning out the gauge
    dprintf(DEBUG_SPEW, "gasgauge_check_health: vbat_mv=%d\n", vbat_mv);
    if (vbat_mv < ALWAYS_BOOT_BATTERY_VOLTAGE)
	return false;

    uint16_t temperature = hdqRead16(kHDQRegTemperature);

    // a low temperature can depress the capacity
    dprintf(DEBUG_SPEW, "gasgauge_check_health: temp=%d\n", temperature);
    if (temperature < RA_UPDATE_TEMPERATURE)
	return false;

    int16_t full_charge_capacity = hdqRead16(kHDQRegFullChargeCapacity);
    int16_t design_capacity = hdqRead16(kHDQRegDesignCapacity);

    dprintf(DEBUG_SPEW, "gasgauge_check_health: fcc=%d dc=%d\n", full_charge_capacity, design_capacity);
    // primary check: the capacity should be lower than any real battery (or negative)
    if (full_charge_capacity >= ((design_capacity * RA_UPDATE_CAPACITY) / 100))
	return false;

    // get the battery chemistry and look it up in the table of defaults.
    uint16_t chem_id = controlRead16(kHDQControlRegChemID);

    const struct ra_table_info_t *ra_table = NULL;
    for (unsigned idx = 0; idx < NUM_RA_TABLES; idx++) {
	if (ra_table_info[idx].chemID == chem_id) {
	    ra_table = &ra_table_info[idx];
	    break;
	}
    }
    dprintf(DEBUG_SPEW, "gasgauge_check_health: chem_id=%#x ra_table=%p\n", chem_id, ra_table);
    // if it's a chemistry we don't know, do not attempt an update
    if (ra_table == NULL)
	return false;

    // only apply the update with the firmware version known to exhibit this problem
    uint16_t fw_version = controlRead16(kHDQControlRegFWVersion);
    dprintf(DEBUG_SPEW, "gasgauge_check_health: fw version=%#x\n", fw_version);
    if (fw_version != RA_UPDATE_FW_VERSION)
	return false;

    // If a future data flash update corrects this problem and wants to disable this algorithm,
    // it can write non-zero to byte zero of Manufacturer Info Block C (zero on all current packs).
    uint8_t manufacturer_info_c[32];
    if (readBlock(kHDQDataSealedModeSubclass, kHDQDataSealedModeBlockManufacturerInfoBlockC, manufacturer_info_c) < 0)
	return false;
    dprintf(DEBUG_SPEW, "gasgauge_check_health: block c: %#x %#x\n", manufacturer_info_c[0], manufacturer_info_c[1]);
    if (manufacturer_info_c[kHDQDataOffsetBlockManufacturerInfoBlockCFlags] != 0)
	return false;

    dprintf(DEBUG_CRITICAL, "gas gauge full charge capacity %d/%d mAh (vbat %d mV temp %d dK chem ID %#x fw %#x): updating resistance tables\n", full_charge_capacity, design_capacity, vbat_mv, temperature, chem_id, fw_version);

    uint8_t gg_state[32];
    uint8_t ra_table_0[32];
    uint8_t ra_table_x[32];

    // Create update log
    gg_ra_update_log = malloc(gg_ra_update_log_size);
    gg_ra_update_log[0] = '\0';
    
    ra_log_val("vbat", vbat_mv);
    ra_log_val("t", temperature);
    ra_log_val("v", hdqRead16(kHDQRegVoltage));
    ra_log_val("ac", hdqRead16(kHDQRegAverageCurrent));
    ra_log_val("f", hdqRead16(kHDQRegFlags));
    ra_log_val("rc", hdqRead16(kHDQRegRemainingCapacity));
    ra_log_val("fcc", full_charge_capacity);
    ra_log_val("dc", design_capacity);
    ra_log_val("s", controlRead16(kHDQControlRegStatus));
    ra_log_val("fv", fw_version);
    ra_log_val("rd", controlRead16(kHDQControlRegResetData));
    ra_log_val("ci", chem_id);
    ra_log_block("mic", manufacturer_info_c);

    // unseal gas gauge
    controlWrite16(kHDQUnsealKey & 0xFFFF, (kHDQUnsealKey >> 16) & 0xFFFF);

    if (readBlock(kHDQDataSubclassGasGaugingState, 0, gg_state) < 0)
	return false;
	
    if (readBlock(kHDQDataSubclassRaTablesData, 0, ra_table_0) < 0)
	return false;
    
    if (readBlock(kHDQDataSubclassRaTablesDataX, 0, ra_table_x) < 0)
	return false;

    ra_log_block("82", gg_state);
    ra_log_block("88", ra_table_0);
    ra_log_block("89", ra_table_x);    
    
    // update Avg P Last Run
    gg_state[kHDQDataOffsetGasGaugingStateAvgPLastRun] = (ra_table->avg_p >> 8) & 0xFF;
    gg_state[kHDQDataOffsetGasGaugingStateAvgPLastRun+1] = ra_table->avg_p & 0xFF;

    // update data tables
    ra_table_0[0] = 0x00;
    ra_table_0[1] = 0x55;
    
    ra_table_x[0] = 0x00;
    ra_table_x[1] = 0x00;
    
    for (int idx = 0; idx < 15; idx++) {
	int offset = 2 + 2*idx;

	ra_table_0[offset] = ra_table_x[offset] = (ra_table->R_a[idx] >> 8) & 0xFF;
	ra_table_0[offset+1] = ra_table_x[offset+1] = ra_table->R_a[idx] & 0xFF;
    }

    // increment update count and store in manufacturer info Block C
    if (manufacturer_info_c[kHDQDataOffsetBlockManufacturerInfoBlockCUpdateCount] < 255)
	manufacturer_info_c[kHDQDataOffsetBlockManufacturerInfoBlockCUpdateCount]++;

    // flash new data
    writeBlock(kHDQDataSubclassRaTablesDataX, 0, ra_table_x);
    writeBlock(kHDQDataSubclassRaTablesData, 0, ra_table_0);
    writeBlock(kHDQDataSubclassGasGaugingState, 0, gg_state);
    writeBlock(kHDQDataSealedModeSubclass, kHDQDataSealedModeBlockManufacturerInfoBlockC, manufacturer_info_c);

    ra_log_block("82'", gg_state);
    ra_log_block("88'", ra_table_0);
    ra_log_block("89'", ra_table_x);

    // pause charger to reduce current to the battery
    bool charging = power_enable_charging(true, false);
    
    if (charging) {
	// if we were charging, wait for the battery voltage to relax, to 
	task_sleep(kHDQIntervalRelax);
    }

    ra_log_val("ac'", hdqRead16(kHDQRegAverageCurrent));

    // reset gas gauge
    controlRead16(kHDQControlRegReset);

    // wait for reset before communicating with the gauge
    task_sleep(kHDQIntervalReset);

    // re-enable charger
    power_enable_charging(true, true);

    // log new values
    
    ra_log_val("t*", hdqRead16(kHDQRegTemperature));
    ra_log_val("v*", hdqRead16(kHDQRegVoltage));
    ra_log_val("ac*", hdqRead16(kHDQRegAverageCurrent));
    ra_log_val("f*", hdqRead16(kHDQRegFlags));
    ra_log_val("rc*", hdqRead16(kHDQRegRemainingCapacity));
    ra_log_val("fcc*", hdqRead16(kHDQRegFullChargeCapacity));
    ra_log_val("dc*", hdqRead16(kHDQRegDesignCapacity));
    ra_log_val("s*", controlRead16(kHDQControlRegStatus));
    ra_log_val("fv*", controlRead16(kHDQControlRegFWVersion));
    ra_log_val("rd*", controlRead16(kHDQControlRegResetData));
    ra_log_val("ci*", controlRead16(kHDQControlRegChemID));
    if (readBlock(kHDQDataSealedModeSubclass, kHDQDataSealedModeBlockManufacturerInfoBlockC, manufacturer_info_c) == 0)
	ra_log_block("mic*", manufacturer_info_c);

    gg_ra_update_done = true;
    return true;
#endif

    return false;
}

void gasgauge_late_init(void)
{
#if GG_CHECK_RA_TABLES
    // if the gas gauge resistance tables were updated, make a log in NVRAM
    // for the OS to pick up and report.
    if (gg_ra_update_done)
    {
#if WITH_ENV
	env_set("gg-data-update-info", gg_ra_update_log, ENV_PERSISTENT);
#if WITH_NVRAM
	nvram_save();
#endif
#endif
    }
#endif
}

 
#if defined(WITH_MENU) && WITH_MENU

static int 
dataBlock(int class, int block, unsigned char *blockData)
{
    unsigned char blockDataLocal[32];
    unsigned char sum = 0, cksum;
    int rc;
    int idx, len, value;
    bool sealedMode = (class == kHDQDataSealedModeSubclass);

    if (blockData == NULL) blockData = blockDataLocal;

    rc = hdqWrite8(kHDQRegBlockDataControl, sealedMode ? 1 : 0);
    if (rc < 0)
	return (rc);

    if (!sealedMode) {
	rc = hdqWrite8(kHDQRegDataFlashClass, class);
	if (rc < 0)
	    return (rc);
    }

    printf("Class %d block %d\n", class, block);

    printf("  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    len = sizeof(blockDataLocal);
    rc = hdqWrite8(kHDQRegDataFlashBlock, block);
    for (idx = 0; idx < len; idx++)
    {
	if (16 == idx)
	    printf("\n");
	value = hdqRead8(kHDQRegBlockData + idx);
	if (value < 0)
	    return value;
	
	blockData[idx] = value;
	sum += value;
	printf(" %02x", value);
    }
    cksum = hdqRead8(kHDQRegBlockDataChecksum);
    sum += cksum;
    printf("\nsum %s, cksum 0x%x\n", (sum == 0xff) ? "ok" : "bad", cksum);

    if (sum != 0xff)
	return -1;

    return (0);
}

static int
writeData(int class, int block, struct cmd_arg *args)
{
    unsigned char oldData[32] = { 0 };
    unsigned char newData[32];
    int idx;
    
    printf("\nCURRENT DATA:\n\n");
    
    dataBlock(class, block, oldData);

    printf("\nNEW DATA (changes \x1b[1;31mhighlighted\x1b[0m):\n\n");
    
    printf("  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (idx = 0; idx < 32; idx++)
    {
	if (16 == idx)
	    printf("\n");

	newData[idx] = strtoul(args[idx].str, NULL, 16);
	if (newData[idx] != oldData[idx])
	    printf(" \x1b[1;31m%02x\x1b[0m", newData[idx]);
	else
	    printf(" %02x", newData[idx]);
    }
    
    printf("\nwriting data...");
    if (writeBlock(class, block, newData) == 0)
	printf("done:\n\n");
    else
	printf("failed!\n\n");
    dataBlock(class, block, 0);

    return 0;
}

static void
usage(struct cmd_arg *args)
{
    puts("not enough arguments.\n");

    printf("%s read <addr>           print reg at addr\n", args[0].str);
    printf("%s control <addr>        print control reg at addr\n", args[0].str);
    printf("%s flash <class> <block> dump flash block\n", args[0].str);
    printf("%s flashwrite <class> <block> <b1> <b2> ... <b32>\n", args[0].str);
    printf("%s reset                 reset gas gauge (toggle SWI line)\n", args[0].str);
    printf("%s unseal <key>          unseal gas gauge\n", args[0].str);
    printf("%s break                 no-op\n", args[0].str);
#if defined(BATTERY_TRAP_DEBUG)
    printf("%s dump                  dump gas gauge registers\n", args[0].str);
    printf("%s trap                  call battery trap check\n", args[0].str);
#endif
}

static int do_hdq(int argc, struct cmd_arg *args)
{
    int16_t value;
    int ret = 0;

    if (!gg_open()) {
	printf("failed to open gas gauge\n");
	return -1;
    }

    uart_break();

    if (argc < 2) {
	ret = -1;
    } else if (!strcmp("read", args[1].str) && (argc == 3)) {
	value = hdqRead16(args[2].u);
	printf("0x%x, %d\n", value, value);
    } else if (!strcmp("control", args[1].str) && (argc == 3)) {
	value = controlRead16(args[2].u);
	printf("0x%x, %d\n", value, value);
    } else if (!strcmp("flash", args[1].str) && (argc == 4)) {
	dataBlock(args[2].u, args[3].u, NULL);
    } else if (!strcmp("flashwrite", args[1].str) && (argc == 36)) {
	writeData(args[2].u, args[3].u, &args[4]);
    } else if (!strcmp("reset", args[1].str) && (argc == 2)) {
	gasgauge_reset(1); // hard reset
    } else if (!strcmp("checkreset", args[1].str) && (argc == 2)) {
	gasgauge_check_reset();
    } else if (!strcmp("unseal", args[1].str) && (argc == 3)) {
	uint32_t key = args[2].u;
	controlWrite16(key & 0xFFFF, (key >> 16) & 0xFFFF);
    } else if (!strcmp("break", args[1].str) && (argc == 2)) {
	// no-op
#if defined(BATTERY_TRAP_DEBUG)
    } else if (!strcmp("dump", args[1].str) && (argc == 2)) {
	gasgauge_dump();
    } else if (!strcmp("trap", args[1].str) && (argc == 2)) {
	gasgauge_needs_precharge(DEBUG_CRITICAL, true);
#endif
    } else {
	ret = -1;
    }

    gg_close();

    if (ret)
	usage(args);

    return ret;
}

MENU_COMMAND_DEBUG(hdq, do_hdq, "HDQ read/write", NULL);
#endif  /* WITH_MENU */
