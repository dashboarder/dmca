/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __DIALOG_D2255_H
#define __DIALOG_D2255_H

#define PMU_HAS_SWI             0
#define PMU_HAS_DWI             0
#define PMU_HAS_SPI             1
// TODO: use single register alias (and a value alias) for HAS_SWI,HAS_DWI,HAS_SPI
#define PMU_HAS_LCM_LDO         0
#define PMU_HAS_BIST_ADC        0
#define PMU_HAS_CHG_ABCC_FLAG   0
#define PMU_HAS_VIB             0
#define PMU_HAS_RAM             1
#define PMU_HAS_WLED            0
#define PMU_HAS_32K_RTC         1
// TODO: enable accumulators
#define PMU_HAS_ACCUMULATORS    0
#define PMU_HAS_GPIO_CONF       1
#define PMU_HAS_SYS             1
#define PMU_HAS_CHARGER         0
#define PMU_HAS_PERF_STATES     1
#define PMU_HAS_VSEL_LOCK       1
#define PMU_DRV_NEW_DRIVER      1
#define PMU_HAS_SCRATCH         1

enum {
    kDIALOG_ADDR_R      = 0xe9,
    kDIALOG_ADDR_W      = 0xe8,
    kDIALOG_REG_BYTES   = 2,
};

enum {
    kDIALOG_FAULTLOG_COUNT      = 15,   // kD2255_V_FAULTLOG_COUNT
    kDIALOG_EVENT_COUNT         = 25,   // kD2255_V_EVENT_COUNT
    kDIALOG_STATUS_COUNT        = 25,   // kD2255_V_STATUS_COUNT
    kDIALOG_CHIPID_COUNT        = 10,   // SYSCTL_MASK_REV_CODE:0x0200 -> SYSCTL_DEVICE_ID7:0x020a
    kDIALOG_GPIO_COUNT          = 21,   // IO_GPIO1_CONF1/2:0x0900,0x0901  -> IO_GPIO21_CONF1/2:0x0938,0x0939
};

#include "dCommon.h"

enum {
// ---------------------------------    
// Faultlog (virtual)
// ---------------------------------    
    kDIALOG_FAULT_LOG_A     = 0x0100,
    // ...
    // kDIALOG_FAULT_LOG_O

// ---------------------------------    
// Event, Status ,IRQ Mask (Virtual)
// ---------------------------------    
    
    kD2255_V_EVENT_A         = 0x0140,
    kD2255_V_EVENT_B         = 0x0141,
    kD2255_V_EVENT_C         = 0x0142,
    kD2255_V_EVENT_D         = 0x0143,
    kD2255_V_EVENT_E         = 0x0144,
    kD2255_V_EVENT_F         = 0x0145,
    kD2255_V_EVENT_G         = 0x0146,
    kD2255_V_EVENT_H         = 0x0147,
    kD2255_V_EVENT_I         = 0x0148,
    kD2255_V_EVENT_J         = 0x0149,
    kD2255_V_EVENT_K         = 0x014a,
    kD2255_V_EVENT_L         = 0x014b,
    kD2255_V_EVENT_M         = 0x014c,
    kD2255_V_EVENT_N         = 0x014d,
    kD2255_V_EVENT_O         = 0x014e,
    kD2255_V_EVENT_P         = 0x014f,
    kD2255_V_EVENT_Q         = 0x0150,
    kD2255_V_EVENT_R         = 0x0151,
    kD2255_V_EVENT_S         = 0x0152,
    kD2255_V_EVENT_T         = 0x0153,
    kD2255_V_EVENT_U         = 0x0154,
    kD2255_V_EVENT_V         = 0x0155,
    kD2255_V_EVENT_W         = 0x0156,
    kD2255_V_EVENT_X         = 0x0157,
    kD2255_V_EVENT_Y         = 0x0158,

    kD2255_V_STATUS_A        = 0x0180,
    // .... V_STATUS_Y
    kD2255_V_IRQ_MASK_A      = 0x01c0,
    // .... V_IRQ_MASK_Y

    kD2255_V_FAULTLOG_COUNT      = 15,
    kD2255_V_EVENT_COUNT         = 25,
    kD2255_V_STATUS_COUNT        = 25,

    kDIALOG_EVENT_A         = kD2255_V_EVENT_A,
    kDIALOG_STATUS_A        = kD2255_V_STATUS_A,
    kDIALOG_IRQ_MASK_A      = kD2255_V_IRQ_MASK_A,
};

// ---------------------------------    
// Chip Identification
// ---------------------------------    
enum {
    kDIALOG_MASK_REV_CODE   = 0x0200,
    kDIALOG_TRIM_REL_CODE   = 0x0201,
    kDIALOG_PLATFORM_ID     = 0x0202,
    kDIALOG_DEVICE_ID1      = 0x0204,
    kDIALOG_DEVICE_ID2      = 0x0205,
    kDIALOG_DEVICE_ID3      = 0x0206,
    kDIALOG_DEVICE_ID4      = 0x0207,
    kDIALOG_DEVICE_ID5      = 0x0208,
    kDIALOG_DEVICE_ID6      = 0x0209,
    kDIALOG_DEVICE_ID7      = 0x020a,

    kDIALOG_CHIP_ID         = kDIALOG_MASK_REV_CODE,
};
// ---------------------------------    
// System Control And Status
// ---------------------------------    
enum {
    kD2255_APP_TMUX         = 0x0212,
};

// ---------------------------------    
// System Configuration
// ---------------------------------    

enum {
    kD2255_SYSCTL_PRE_UVLO_CTRL      = 0x0268,
    kD2255_SYSCTL_PRE_UVLO_SET_MASK  = 0xf0,
    kD2255_SYSCTL_PRE_UVLO_SET_SHIFT = 4,
};

// ---------------------------------
// Faultlog
// ---------------------------------    

enum {
    kD2255_FAULT_LOG1       = 0x02c0,
    kD2255_FAULT_LOG2       = 0x02c1,

    kDIALOG_FAULT_LOG       = kD2255_FAULT_LOG1,
    kDIALOG_FAULT_LOG2      = kD2255_FAULT_LOG2,
};

// ---------------------------------    
// Events. Status and IRQ Control
// ---------------------------------    

enum {
    kD2255_SYSCTL_EVENT     = 0x02cf,
    kD2255_SYSCTL_STATUS    = 0x02d0,
    kD2255_SYSCTL_IRQ_MASK  = 0x02d1,
};

// ---------------------------------    
// Power Supply Control 
// Active,Standby,Hibernate are now encoded here, need TestAccess
// ---------------------------------    
enum {
    kD2255_PWRONOFF_BUCK0_EN = 0x0300,
    kD2255_PWRONOFF_BUCK1_EN = 0x0301,
    kD2255_PWRONOFF_BUCK2_EN = 0x0302,
    kD2255_PWRONOFF_BUCK3_EN = 0x0303,
    kD2255_PWRONOFF_BUCK4_EN = 0x0304,
    kD2255_PWRONOFF_BUCK5_EN = 0x0305,
    kD2255_PWRONOFF_BUCK6_EN = 0x0306,
    kD2255_PWRONOFF_BUCK7_EN = 0x0307,
    kD2255_PWRONOFF_BUCK8_EN = 0x0308,
    kD2255_PWRONOFF_LDO1_EN  = 0x0309,
    kD2255_PWRONOFF_LDO2_EN  = 0x030a,
    kD2255_PWRONOFF_LDO3_EN  = 0x030b,
    kD2255_PWRONOFF_LDO4_EN  = 0x030c,
    kD2255_PWRONOFF_LDO5_EN  = 0x030d,
    kD2255_PWRONOFF_LDO6_EN  = 0x030e,
    kD2255_PWRONOFF_LDO7_EN  = 0x030f,
    kD2255_PWRONOFF_LDO8_EN  = 0x0310,
    kD2255_PWRONOFF_LDO9_EN  = 0x0311,
    kD2255_PWRONOFF_LDO10_EN  = 0x0312,
    kD2255_PWRONOFF_LDO11_EN  = 0x0313,
    kD2255_PWRONOFF_LDO13_EN  = 0x0314,
    kD2255_PWRONOFF_LDO14_EN  = 0x0315,
    kD2255_PWRONOFF_LDO15_EN  = 0x0316,
    kD2255_PWRONOFF_CP_EN     = 0x0317,
    kD2255_PWRONOFF_BUCK3_SW1_EN    = 0x0318,
    kD2255_PWRONOFF_BUCK3_SW2_EN    = 0x0319,
    kD2255_PWRONOFF_BUCK3_SW3_EN    = 0x031a,
    kD2255_PWRONOFF_BUCK4_SW1_EN    = 0x031b,
    kD2255_PWRONOFF_GPIO_32K_EN     = 0x031c,
    kD2255_PWRONOFF_OUT_32K_EN      = 0x031d,
    kD2255_PWRONOFF_SLEEP_32K_EN    = 0x031e,
    kD2255_PWRONOFF_NRESET_EN       = 0x031f,
    kD2255_PWRONOFF_SYS_ALIVE_EN    = 0x0320,
    kD2255_PWRONOFF_PRE_UVLO_EN     = 0x0321,
};

// ---------------------------------    
// Main FSM, MFSM Events
// ---------------------------------    

enum {
    kD2255_MFSM_CTRL            = 0x0400,
    kD2255_ACT_TO_SLEEP3_DLY    = 0x0401,
    kD2255_ACT_TO_OFF_DLY       = 0x0402,
    kD2255_OFF_TIMER_DLY        = 0x0403,
    kD2255_SLEEP3_TIMER_DLY     = 0x0404,
    kD2255_MFSM_CONF1           = 0x0460,


    kD2255_MFSM_CTRL_PERF_FLOOR_OFF     = (0<<2),
    kD2255_MFSM_CTRL_PERF_FLOOR_SLEEP3  = (1<<2),
    kD2255_MFSM_CTRL_PERF_FLOOR_SLEEP2  = (2<<2),
    kD2255_MFSM_CTRL_PERF_FLOOR_ACTIVE  = (3<<2),
    kD2255_MFSM_CTRL_FORCE_SLEEP3   = (1<<1),
    kD2255_MFSM_CTRL_FORCE_OFF      = (1<<0),

    kDIALOG_SYS_CONTROL_PERF_FLOOR_OFF      = kD2255_MFSM_CTRL_PERF_FLOOR_OFF,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_SLEEP3   = kD2255_MFSM_CTRL_PERF_FLOOR_SLEEP3,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_SLEEP2   = kD2255_MFSM_CTRL_PERF_FLOOR_SLEEP2,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_ACTIVE   = kD2255_MFSM_CTRL_PERF_FLOOR_ACTIVE,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_MASK     = (0x3<<2),
    
    kDIALOG_SYS_CONTROL_HIBERNATE   = kD2255_MFSM_CTRL_FORCE_SLEEP3,
    kDIALOG_SYS_CONTROL_STANDBY     = kD2255_MFSM_CTRL_FORCE_OFF,
    // TODO: verify this
    kDIALOG_SYS_CONTROL_HIBERNATE_ALWAYS    = kDIALOG_SYS_CONTROL_HIBERNATE,

    kDIALOG_SYS_CONTROL     = kD2255_MFSM_CTRL,
};

// ---------------------------------    
// RTC Calendar and Clock, Events
// ---------------------------------    

enum {
    kDIALOG_RTC_CONTROL         = 0x0500,
    kDIALOG_RTC_TIMEZONE        = 0x0501,
    kDIALOG_RTC_SUB_SECOND_A    = 0x0502,
    kDIALOG_RTC_SUB_SECOND_B    = 0x0503,
    kDIALOG_RTC_SECOND_A        = 0x0504,
    kDIALOG_RTC_SECOND_B        = 0x0505,
    kDIALOG_RTC_SECOND_C        = 0x0506,
    kDIALOG_RTC_SECOND_D        = 0x0507,
    kDIALOG_RTC_ALARM_A         = 0x0508,
    kDIALOG_RTC_ALARM_B         = 0x0509,
    kDIALOG_RTC_ALARM_C         = 0x050a,
    kDIALOG_RTC_ALARM_D         = 0x050b,

// Events. Status. Mask in Virtual
//    kD2255_RTC_EVENT            = 0x05c0,
//    kD2255_RTC_STATUS           = 0x05c1,
//    kD2255_RTC_IRQ_MASK         = 0x05c2,
};

// ---------------------------------    
// SPI Interface
// ---------------------------------    

enum {
    kD2255_SPI_CTRL             = 0x0600, // global enable
    kD2255_EN_CMD0              = 0x0601, // buck#_spi_en
    kD2255_EN_CMD1              = 0x0602, // Antigua130: BUCK8 [0]
    kD2255_EN_CMD3              = 0x0604, // buck#_spi_dis_master
    kD2255_EN_CMD4              = 0x0605, // Antigua130: BUCK8 [0]
    kD2255_SPI_CONFIG           = 0x0660,

    // set when PMU_HAS_SPI
    kDIALOG_SPI_CONTROL            = kD2255_SPI_CTRL,
    kDIALOG_SPI_CONTROL_SPI_EN     = 1,
    
// Events. Status. Mask in Virtual
};

// ---------------------------------    
// Clock Generation
// ---------------------------------    

enum {
    kD2255_SYS_CTRL_CONF0       = 0x0700,
    kD2255_SYS_CTRL_CONF1       = 0x0760,
    kD2255_RTC_CTRL0            = 0x0761,
    kD2255_RTC_CTRL1            = 0x0762,
    kD2255_RTC_CTRL2            = 0x0763,
    kD2255_OSC_CTRL0            = 0x0764,
    kD2255_INT_OSC_TRIM         = 0x0765, // Antigua130: INT_OSC_TRIM[5:0]
    kD2255_CLK_REQ_FRC          = 0x0766,
    kD2255_BG_CTRL              = 0x0767,

    kD2255_CKLGEN_EVENT         = 0x07c0,
    kD2255_CKLGEN_STATUS        = 0x07c1,
    kD2255_CKLGEN_IRQ_MASK      = 0x07c2,
};

// ---------------------------------    
// Misc
// ---------------------------------    

enum {
    kD2255_HIB_SW_CTRL         = 0x0860, // Antigua130: SEL_2V4[2]
};

// ---------------------------------    
// GPIO configuration
// ---------------------------------    

enum {
    kD2255_GPIO1_CONF1          = 0x0900,
    kD2255_GPIO1_CONF2          = 0x0901,
    kD2255_GPIO2_CONF1,
    kD2255_GPIO2_CONF2,
    kD2255_GPIO3_CONF1,
    kD2255_GPIO3_CONF2,
    kD2255_GPIO4_CONF1,
    kD2255_GPIO4_CONF2,
    kD2255_GPIO5_CONF1,
    kD2255_GPIO5_CONF2,
    kD2255_GPIO6_CONF1,
    kD2255_GPIO6_CONF2,
    kD2255_GPIO7_CONF1,
    kD2255_GPIO7_CONF2,
    kD2255_GPIO8_CONF1,
    kD2255_GPIO8_CONF2,
    kD2255_GPIO9_CONF1,
    kD2255_GPIO9_CONF2,
    kD2255_GPIO10_CONF1,
    kD2255_GPIO10_CONF2,
    kD2255_GPIO11_CONF1,
    kD2255_GPIO11_CONF2,
    kD2255_GPIO12_CONF1,
    kD2255_GPIO12_CONF2,
    kD2255_GPIO13_CONF1,
    kD2255_GPIO13_CONF2,
    kD2255_GPIO14_CONF1,
    kD2255_GPIO14_CONF2,
    kD2255_GPIO15_CONF1,
    kD2255_GPIO15_CONF2,
    kD2255_GPIO16_CONF1,
    kD2255_GPIO16_CONF2,
    kD2255_GPIO17_CONF1,
    kD2255_GPIO17_CONF2,
    kD2255_GPIO18_CONF1_SLP1	= 0x0922,
    kD2255_GPIO18_CONF1_SLP2,
    kD2255_GPIO18_CONF1_SLP3,
    kD2255_GPIO18_CONF1_OFF,
    kD2255_GPIO18_CONF1,
    kD2255_GPIO18_CONF2,
    kD2255_GPIO19_CONF1_SLP1	= 0x0928,
    kD2255_GPIO19_CONF1_SLP2,
    kD2255_GPIO19_CONF1_SLP3,
    kD2255_GPIO19_CONF1_OFF,
    kD2255_GPIO19_CONF1,
    kD2255_GPIO19_CONF2,
    kD2255_GPIO20_CONF1_SLP1	= 0x092e,
    kD2255_GPIO20_CONF1_SLP2,
    kD2255_GPIO20_CONF1_SLP3,
    kD2255_GPIO20_CONF1_OFF,
    kD2255_GPIO20_CONF1,
    kD2255_GPIO20_CONF2,
    kD2255_GPIO21_CONF1_SLP1	= 0x0934,
    kD2255_GPIO21_CONF1_SLP2,
    kD2255_GPIO21_CONF1_SLP3,
    kD2255_GPIO21_CONF1_OFF,
    kD2255_GPIO21_CONF1,
    kD2255_GPIO21_CONF2,
    kDIALOG_SYS_GPIO_REG_START  = kD2255_GPIO1_CONF1,

// until GPIO21_CONF1, GPIO21_CONF2 ... kDIALOG_GPIO_COUNT gpios
    
    kD2255_OUT_32K              = 0x093a,
    kD2255_SLEEP_32K            = 0x093b,
    kD2255_ACTIVE_RDY           = 0x093c,
    kD2255_SLEEP1_RDY           = 0x093d,

// GPI  Control
    kD2255_BUTTON1_CONF         = 0x0960,
    kD2255_BUTTON2_CONF         = 0x0961,
    kD2255_BUTTON3_CONF         = 0x0962,
    kD2255_BUTTON4_CONF         = 0x0963,
    kD2255_BUTTON_DBL           = 0x0964,
    kD2255_BUTTON_WAKE          = 0x0965,

    kD2255_RESET_IN1_CONF1      = 0x0966,
    kD2255_RESET_IN1_CONF2      = 0x0967,
    kD2255_RESET_IN2_CONF1      = 0x0968,
    kD2255_RESET_IN2_CONF2      = 0x0969,
    kD2255_RESET_IN3_CONF1      = 0x096a,
    kD2255_RESET_IN3_CONF2      = 0x096b,

    kDIALOG_BUTTON_DBL          = kD2255_BUTTON_DBL,
};

// ---------------------------------    
// Event,Status,IRQ Control
// ---------------------------------    
enum {
    kD2255_IO_EVENT_A         = 0x09c0,
    kD2255_IO_EVENT_B         = 0x09c1,
    kD2255_IO_EVENT_C         = 0x09c2,
    kD2255_IO_EVENT_D         = 0x09c3,

    kD2255_IO_STATUS_A        = 0x09c4,
    kD2255_IO_STATUS_B        = 0x09c5,
    kD2255_IO_STATUS_C        = 0x09c6,
    kD2255_IO_STATUS_D        = 0x09c7,

    kD2255_IO_IRQ_MASK_A      = 0x09c8,
    kD2255_IO_IRQ_MASK_B      = 0x09c9,
    kD2255_IO_IRQ_MASK_C      = 0x09ca,
    kD2255_IO_IRQ_MASK_D      = 0x09cb,
// Using the Virtual ones
//    kDIALOG_EVENT_A         = kD2255_IO_EVENT_A,
//    kDIALOG_STATUS_A        = kD2255_IO_STATUS_A,
//    kDIALOG_IRQ_MASK_A      = kD2255_IO_IRQ_MASK_A,
};

// ---------------------------------    
// Buck control
// ---------------------------------    

enum {
    kD2255_BUCK0_FAST_VSEL      = 0x1000,
    kD2255_BUCK0_VSEL           = 0x1001,
    kD2255_BUCK0_FAST_VSEL_EN,
    kD2255_BUCK0_VSEL_ACTUAL,
    kD2255_BUCK0_VSEL_LOCK      = 0x1004,
    kD2255_BUCK0_MINV,
    kD2255_BUCK0_MAXV,
    kD2255_BUCK0_OFS_V,
    kD2255_BUCK0_MODE           = 0x1008,
    kD2255_BUCK0_DEBUG_OFS_V    = 0x1060,
    kD2255_BUCK0_FSM_TRIM0      = 0x1061,
    kD2255_BUCK0_FSM_TRIM1,
    kD2255_BUCK0_FSM_TRIM2,
    kD2255_BUCK0_FSM_TRIM3,
    kD2255_BUCK0_FSM_TRIM4,
    kD2255_BUCK0_FSM_TRIM5,
    kD2255_BUCK0_FSM_TRIM6      = 0x1067,
    kD2255_BUCK0_FSM_TRIM7      = 0x1068,
    kD2255_BUCK0_FSM_TRIM8,
    kD2255_BUCK0_FSM_TRIM9,
    kD2255_BUCK0_FSM_TRIM10,
    kD2255_BUCK0_FSM_TRIM11,
    kD2255_BUCK0_FSM_TRIM12,
    kD2255_BUCK0_FSM_TRIM13     = 0x106E,
    kD2255_BUCK0_CLK_TRIM0      = 0x106F,
    kD2255_BUCK0_CLK_TRIM1,
    kD2255_BUCK0_CLK_TRIM2,
    kD2255_BUCK0_CLK_TRIM3,
    kD2255_BUCK0_CALIB0         = 0x1073,
    kD2255_BUCK0_CALIB1,
    kD2255_BUCK0_CALIB2,

    kD2255_BUCK1_FAST_VSEL      = 0x1100,    
    kD2255_BUCK1_VSEL           = 0x1101,
    kD2255_BUCK1_VSEL_LOCK      = 0x1104,
    kD2255_BUCK1_FSM_TRIM6      = 0x1167,
    kD2255_BUCK1_FSM_TRIM7      = 0x1168,
    kD2255_BUCK1_CLK_TRIM1      = 0x1170,
    kD2255_BUCK1_CLK_TRIM2      = 0x1171,
    kD2255_BUCK1_CLK_TRIM3      = 0x1172,
    kD2255_BUCK1_ANA_TRIM10     = 0x1181, // Antigua130 all diff

    kD2255_BUCK2_FAST_VSEL      = 0x1200,
    kD2255_BUCK2_VSEL           = 0x1201,
    kD2255_BUCK2_VSEL_LOCK      = 0x1204,
    kD2255_BUCK2_MINV           = 0x1205,
    kD2255_BUCK2_FSM_TRIM6      = 0x1267,
    kD2255_BUCK2_FSM_TRIM7      = 0x1268,
    
    kD2255_BUCK3_FAST_VSEL      = 0x1300,
    kD2255_BUCK3_VSEL           = 0x1301,
    kD2255_BUCK3_VSEL_LOCK      = 0x1304,
    kD2255_BUCK3_FSM_TRIM6      = 0x1367,
    kD2255_BUCK3_FSM_TRIM7      = 0x1368,
    
    kD2255_BUCK4_FAST_VSEL      = 0x1400,
    kD2255_BUCK4_VSEL           = 0x1401,
    kD2255_BUCK4_VSEL_LOCK      = 0x1404,
    kD2255_BUCK4_FSM_TRIM6      = 0x1467,
    kD2255_BUCK4_FSM_TRIM7      = 0x1468,
    
    kD2255_BUCK5_FAST_VSEL      = 0x1500,
    kD2255_BUCK5_VSEL           = 0x1501,
    kD2255_BUCK5_VSEL_LOCK      = 0x1504,
    kD2255_BUCK5_FSM_TRIM6      = 0x1567,
    kD2255_BUCK5_FSM_TRIM7      = 0x1568,
    kD2255_BUCK5_ANA_TRIM14     = 0x1585, // Antigua130 ANA_TRIM14_SPARE[7:6]
    
    kD2255_BUCK6_FAST_VSEL      = 0x1600,
    kD2255_BUCK6_VSEL           = 0x1601,
    kD2255_BUCK6_VSEL_LOCK      = 0x1604,
    kD2255_BUCK6_BYP1           = 0x1609,
    kD2255_BUCK6_FSM_TRIM6      = 0x1667,
    kD2255_BUCK6_FSM_TRIM7      = 0x1668,    
    
    kD2255_BUCK7_FAST_VSEL      = 0x1700,
    kD2255_BUCK7_VSEL           = 0x1701,
    kD2255_BUCK7_VSEL_LOCK      = 0x1704,
    kD2255_BUCK7_FSM_TRIM6      = 0x1767,
    kD2255_BUCK7_FSM_TRIM7      = 0x1768,
    
    kD2255_BUCK8_FAST_VSEL      = 0x1800,
    kD2255_BUCK8_VSEL           = 0x1801,
    kD2255_BUCK8_VSEL_LOCK      = 0x1804,
    kD2255_BUCK8_FSM_TRIM6      = 0x1867,
    kD2255_BUCK8_FSM_TRIM7      = 0x1868,

    kDIALOG_BUCK_VSEL_LOCK_EN           = 1,
    kDIALOG_BUCK_VSEL_LOCK_DIS          = 0,
    // how much to add to VSEL register base to get VSEL_OFFSET
    kDIALOG_BUCK_VSEL_LOCK_OFFSET       =  (kD2255_BUCK0_VSEL_LOCK-kD2255_BUCK0_VSEL),
};

enum {
    kD2255_LDO_RTC_TRIM         = 0x2062, // Antigua130 VRTC_LDO_TRIM[5:0]

    // LDO control...
    kD2255_LDO1_VSEL            = 0x2100,
    kD2255_LDO1_VSEL_ACTUAL     = 0x2101,
    kD2255_LDO1_MINV            = 0x2160,
    kD2255_LDO1_MAXV            = 0x2161,
    kD2255_LDO1_UOV_LIM_EN_VOUT_PD= 0x2163, // Antigua130 LDO1_UV_LIM[3:2]

    kD2255_LDO2_VSEL            = 0x2200,
    kD2255_LDO2_VSEL_ACTUAL     = 0x2201,    
    kD2255_LDO2_BYPASS          = 0x2202,
    kD2255_LDO2_MINV            = 0x2260,
    kD2255_LDO2_MAXV            = 0x2261,
    kD2255_LDO2_TRIM            = 0x2262,
    kD2255_LDO2_UOV_LIM_EN_VOUT_PD= 0x2263,  // Antigua130 LDO2_UV_LIM[3:2]
    kD2255_LDO2_VSEL_BYPASS_TRIM= 0x2269,

    kD2255_LDO3_VSEL            = 0x2300,
    kD2255_LDO3_VSEL_ACTUAL     = 0x2301,    
    kD2255_LDO3_MINV            = 0x2360,
    kD2255_LDO3_MAXV            = 0x2361,
    kD2255_LDO3_UOV_LIM_EN_VOUT_PD= 0x2363, // Antigua130 LDO3_UV_LIM[3:2]

    kD2255_LDO4_VSEL            = 0x2400,
    kD2255_LDO4_VSEL_ACTUAL     = 0x2401,    
    kD2255_LDO4_MINV            = 0x2460,
    kD2255_LDO4_MAXV            = 0x2461,
    kD2255_LDO4_TRIM            = 0x2462, // Antigua130 LDO4_ITEMP[7:5]
    kD2255_LDO4_UOV_LIM_EN_VOUT_PD= 0x2463, // Antigua130 LDO4_UV_LIM[3:2]

    kD2255_LDO5_VSEL            = 0x2500,
    kD2255_LDO5_VSEL_ACTUAL     = 0x2501,    
    kD2255_LDO5_MINV            = 0x2560,
    kD2255_LDO5_MAXV            = 0x2561,
    kD2255_LDO5_UOV_LIM_EN_VOUT_PD= 0x2563, // Antigua130 LDO5_UV_LIM[3:2]
    kD2255_LDO5_ILIMIT_STATE    = 0x2565, // Antigua130 LDO_LDO5_ILIMIT_STATE[2:1] LDO5_ISTART_100MA[3]
    // .. 

    kD2255_LDO6_VSEL            = 0x2600,
    kD2255_LDO6_VSEL_ACTUAL     = 0x2601,    
    kD2255_LDO6_BYPASS          = 0x2602,
    kD2255_LDO6_MINV            = 0x2660,
    kD2255_LDO6_MAXV            = 0x2661,

    kD2255_LDO7_VSEL            = 0x2700,
    kD2255_LDO7_VSEL_ACTUAL     = 0x2701,    
    kD2255_LDO7_MINV            = 0x2760,
    kD2255_LDO7_MAXV            = 0x2761,

    kD2255_LDO8_VSEL            = 0x2800,
    kD2255_LDO8_VSEL_ACTUAL     = 0x2801,    
    kD2255_LDO8_MINV            = 0x2860,
    kD2255_LDO8_MAXV            = 0x2861,

    kD2255_LDO9_VSEL            = 0x2900,
    kD2255_LDO9_VSEL_ACTUAL     = 0x2901,    
    kD2255_LDO9_MINV            = 0x2960,
    kD2255_LDO9_MAXV            = 0x2961,
    kD2255_LDO9_TRIM            = 0x2962, // Antigua130 LDO9_DROPOUT_TRIM[7:5]

    kD2255_LDO10_VSEL           = 0x2A00,
    kD2255_LDO10_VSEL_ACTUAL    = 0x2A01,    
    kD2255_LDO10_MINV           = 0x2A60,
    kD2255_LDO10_MAXV           = 0x2A61,
    kD2255_LDO10_TRIM           = 0x2A62,

    kD2255_LDO11_VSEL           = 0x2B00,
    kD2255_LDO11_VSEL_ACTUAL    = 0x2B01,    
    kD2255_LDO11_MINV           = 0x2B60,
    kD2255_LDO11_MAXV           = 0x2B61,
    kD2255_LDO11_TRIM           = 0x2B62,

    kD2255_LDO12_TRIM           = 0x2C62,

    kD2255_LDO13_VSEL           = 0x2D00,
    kD2255_LDO13_VSEL_ACTUAL    = 0x2D01,    
    kD2255_LDO13_MINV           = 0x2D60,
    kD2255_LDO13_MAXV           = 0x2D61,
    kD2255_LDO13_TRIM           = 0x2D62,

    kD2255_LDO14_VSEL           = 0x2E00,
    kD2255_LDO14_VSEL_ACTUAL    = 0x2E01,    
    kD2255_LDO14_MINV           = 0x2E60,
    kD2255_LDO14_MAXV           = 0x2E61,
    kD2255_LDO14_TRIM           = 0x2E62,

    kD2255_LDO15_VSEL           = 0x2F00,
    kD2255_LDO15_VSEL_ACTUAL    = 0x2F01,    
    kD2255_LDO15_BYPASS         = 0x2F02,
    kD2255_LDO15_MINV           = 0x2F60,
    kD2255_LDO15_MAXV           = 0x2F61,
    kD2255_LDO15_TRIM           = 0x2F62,

    kD2255_LDO_EVENT            = 0x3FC0,
    kD2255_LDO_STATUS           = 0x3FC1,
    kD2255_LDO_IRQ_MASK         = 0x3FC2,

    kDIALOG_BYPASS_EN           = 1,
    kDIALOG_BYPASS_DIS          = 0,
};

// ---------------------------------
// POWER ON/OFF
// ---------------------------------

enum {
// Power Supply Control
    kD2255_BUCK0_EN             = 0x0300,
    kD2255_BUCK1_EN             = 0x0301,
    kD2255_BUCK2_EN             = 0x0302,
    kD2255_BUCK3_EN             = 0x0303,
    kD2255_BUCK4_EN             = 0x0304,
    kD2255_BUCK5_EN             = 0x0305,
    kD2255_BUCK6_EN             = 0x0306,
    kD2255_BUCK7_EN             = 0x0307,
    kD2255_BUCK8_EN             = 0x0308,
    kD2255_LDO1_EN              = 0x0309,
    kD2255_LDO2_EN              = 0x030a,
    kD2255_LDO3_EN              = 0x030b,
    kD2255_LDO4_EN              = 0x030c,
    kD2255_LDO5_EN              = 0x030d,
    kD2255_LDO6_EN              = 0x030e,
    kD2255_LDO7_EN              = 0x030f,
    kD2255_LDO8_EN              = 0x0310,
    kD2255_LDO9_EN              = 0x0311,
    kD2255_LDO10_EN             = 0x0312,
    kD2255_LDO11_EN             = 0x0313,
    kD2255_LDO13_EN             = 0x0314,
    kD2255_LDO14_EN             = 0x0315,
    kD2255_LDO15_EN             = 0x0316,
    kD2255_CP_EN,
    kD2255_BUCK3_SW1_EN         = 0x0318,
    kD2255_BUCK3_SW2_EN         = 0x0319,
    kD2255_BUCK3_SW3_EN         = 0x031A,
    kD2255_BUCK4_SW1_EN         = 0x031B,
    kD2255_GPIO32K_EN           = 0x031C,
    kD2255_OUT32K_EN            = 0x031D,
    kD2255_SLEEP32K_EN          = 0x031E,
    kD2255_NRESET_EN            = 0x031F,
    kD2255_SYSALIVE_EN          = 0x0320,
    kD2255_PRE_UVLO_EN          = 0x0321,
    
    
};


// ---------------------------------
// GPADC control
// ---------------------------------    

enum {
    kD2255_GPADC_MAN_CTRL1      = 0x4000,
    kD2255_GPADC_MAN_CTRL1_MASK = 0xff,
    kD2255_GPADC_MAN1_RES_LSB   = 0x4001,
    kD2255_GPADC_MAN1_RES_MSB   = 0x4002,

    kD2255_GPADC_MAN_CTRL2      = 0x4004,
    kD2255_GPADC_MAN_CTRL2_MASK = 0x7F,
    kD2255_GPADC_MAN2_RES_LSB   = 0x4005,
    kD2255_GPADC_MAN2_RES_MSB   = 0x4006,

    kD2255_GPADC_TDIE1_RISE     = 0x4147,
    // ..
    kD2255_GPADC_TDIE11_RISE    = 0x4151,

    kDIALOG_T_OFFSET_MSB	= 0x4153,
    kDIALOG_T_OFFSET_LSB	= 0x4154, // [0-3]

    kD2265_GPADC_ANA_COMPAT_CONF = 0x4172, // to disable Antigua130's ADC_Compatibility mode
	kD2265_GP_ADC_COMPAT_BYPASS = 1, // [0]

    kD2255_GPADC_EVENT0        = 0x418B,
    kD2255_GPADC_EVENT1        = 0x418C,
    kD2255_GPADC_STATUS0       = 0x4191,
    kD2255_GPADC_STATUS1       = 0x4192,
};

enum {
    kDIALOG_APP_TMUX            = kD2255_APP_TMUX,
#if 0
    kDIALOG_BIST_ADC_CTRL       = 0x0541,
    kDIALOG_BIST_ADC_LSB        = 0x0542,
    kDIALOG_BIST_ADC_MSB        = 0x0543,
    kD2186_BIST_ADC_TRIM1       = 0x0545,
    kD2186_BIST_ADC_ANA_TRIM0   = 0x0546,
    
    kD2186_SPARE_RW0            = 0x0680,
    kD2186_SPARE_RW_LAST        = 0x0687,
    kD2186_SPARE_RWTOTP0        = 0x06a0,
    kD2186_SPARE_RWTOTP_LAST    = 0x06a7,
    kD2186_SPARE_RWOTP0         = 0x06c0,
    kD2186_SPARE_RWOTP1         = 0x06c1,
#endif
};

// ---------------------------------
// RAM/TEST/SCRATCH
// ---------------------------------

enum {
    //  Legacy Scratch Pads
    kDIALOG_MEMBYTE0            = 0x5000,
    kDIALOG_MEMBYTE_LAST        = 0x5027,
    
    kDIALOG_TEST_ACCESS         = 0x7000,
        kDIALOG_TEST_ACCESS_ENA = 0x1D,	  // Antigua130 defines 1, ok to use 0x1D
        kDIALOG_TEST_ACCESS_DIS = 0x00,
    
    //  Check for Antigua130 (dialog 2265) Implementation
    kD2265_IMPLEMENTATION   = 0x7081,
	kD2265_130NM        = 130,

    //  Scratch Pad RAM
    kDIALOG_RAM0            = 0x8000,
    kDIALOG_RAM_LAST        = 0x87ff,
    
    kDIALOG_EXT_MEM_CAL0_SIZE  = 64,
    kDIALOG_GAP_SCRATCH8_SIZE  = 8, // ex diags, free now
    kDIALOG_EXT_MEM_CAL1_SIZE  = 336,
    kDIALOG_VOLTAGE_KNOBS_SIZE = 32,
    // ...
    kDIALOG_DIAG_SCRATCH_SIZE  = 32,
    
    kDIALOG_EXT_MEM_CAL0    = kDIALOG_RAM0,
    kDIALOG_EXT_MEM_CAL1    = kDIALOG_EXT_MEM_CAL0+kDIALOG_EXT_MEM_CAL0_SIZE+kDIALOG_GAP_SCRATCH8_SIZE,
    kDIALOG_VOLTAGE_KNOBS   = kDIALOG_EXT_MEM_CAL1+kDIALOG_EXT_MEM_CAL1_SIZE,
    // reserved, future
    kDIALOG_DIAG_SCRATCH    = 0x87e0,
    
    // handy
    kDIALOG_EXT_MEM_CAL_SIZE   = kDIALOG_EXT_MEM_CAL0_SIZE+kDIALOG_EXT_MEM_CAL1_SIZE,
};

#if 0
// TODO: Synthetic Accumulator Registers
enum {
    kDIALOG_ACCUMULATOR_SEL_COUNT   = 2,
    kDIALOG_IBUS_ACCUMULATOR    = 0,
    kDIALOG_VBUS_ACCUMULATOR    = 1,
};
#endif

enum {
// Virtual Event registers

    // SYSCTL_EVENT
    kD2255_EVENT_A_UOV              = (1 << 1),
    kD2255_EVENT_A_HIGH_TEMP_WARNING= (1 << 0),
    // MFSM_EVENT
    kD2255_EVENT_B_SLEEP3_EXIT      = (1 << 2),
    kD2255_EVENT_B_SLEEP2_EXIT      = (1 << 1),
    kD2255_EVENT_B_SLEEP1_EXIT      = (1 << 0),
    // RTC_EVENT
    kD2255_EVENT_C_ALARM            = (1 << 0),
    // SPI_EVENT    
    kD2255_EVENT_D_NACK             = (1 << 1),
    kD2255_EVENT_D_SPI_ERR          = (1 << 0),
    // CLKGEN_EVENT    
    kD2255_EVENT_E_XTAL_ERR         = (1 << 0),
    // IO_EVENT_A
    kD2255_EVENT_F_BUTTON4_DBL      = (1 << 7),
    kD2255_EVENT_F_BUTTON3_DBL      = (1 << 6),
    kD2255_EVENT_F_BUTTON2_DBL      = (1 << 5),
    kD2255_EVENT_F_BUTTON1_DBL      = (1 << 4),
    kD2255_EVENT_F_BUTTON4          = (1 << 3),
    kD2255_EVENT_F_BUTTON3          = (1 << 2),
    kD2255_EVENT_F_BUTTON2          = (1 << 1),
    kD2255_EVENT_F_BUTTON1          = (1 << 0),
    // IO_EVENT_B
    kD2255_EVENT_G_GPIO8            = (1 << 7),
    kD2255_EVENT_G_GPIO7            = (1 << 6),
    kD2255_EVENT_G_GPIO6            = (1 << 5),
    kD2255_EVENT_G_GPIO5            = (1 << 4),
    kD2255_EVENT_G_GPIO4            = (1 << 3),
    kD2255_EVENT_G_GPIO3            = (1 << 2),
    kD2255_EVENT_G_GPIO2            = (1 << 1),
    kD2255_EVENT_G_GPIO1            = (1 << 0),
    // IO_EVENT_C
    kD2255_EVENT_H_GPIO16           = (1 << 7),
    kD2255_EVENT_H_GPIO15           = (1 << 6),
    kD2255_EVENT_H_GPIO14           = (1 << 5),
    kD2255_EVENT_H_GPIO13           = (1 << 4),
    kD2255_EVENT_H_GPIO12           = (1 << 3),
    kD2255_EVENT_H_GPIO11           = (1 << 2),
    kD2255_EVENT_H_GPIO10           = (1 << 1),
    kD2255_EVENT_H_GPIO9            = (1 << 0),
    // IO_EVENT_D
    kD2255_EVENT_I_GPIO21           = (1 << 4),
    kD2255_EVENT_I_GPIO20           = (1 << 3),
    kD2255_EVENT_I_GPIO19           = (1 << 2),
    kD2255_EVENT_I_GPIO18           = (1 << 1),
    kD2255_EVENT_I_GPIO17           = (1 << 0),
    // GPADC_EVENT0
    kD2255_EVENT_T_ADC_ERROR        = (1 << 7),
    kD2255_EVENT_T_EOMC2            = (1 << 1),
    kD2255_EVENT_T_EOMC1            = (1 << 0),
};


// EVENT_L events are stored by LLB across sleep/wake
enum {
    kD2186_EVENT_L_VIB_ILIM         = (1 << 7),
    kD2186_EVENT_L_IBUS_OVERFLOW    = (1 << 6),
    kD2186_EVENT_L_XTAL_ERROR       = (1 << 5),
    kD2186_EVENT_L_HIB              = (1 << 4),
    kD2186_EVENT_L_DWI_TO           = (1 << 3),
    kD2186_EVENT_L_EOMC_BIST        = (1 << 2),
    kD2186_EVENT_L_EOMC             = (1 << 1),
    kD2186_EVENT_L_ALARM            = (1 << 0),
};

enum {
    kDIALOG_EVENT_HOLD_BUTTON_MASK  = EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON2),
    kDIALOG_EVENT_MENU_BUTTON_MASK  = EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON1),
    kDIALOG_EVENT_RINGER_BUTTON_MASK= EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON3),
    kDIALOG_EVENT_BUTTON4_MASK  = EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON4),
    
    kDIALOG_EVENT_BUTTONS       = (kD2255_EVENT_F_BUTTON4 |
                       kD2255_EVENT_F_BUTTON3 |
                       kD2255_EVENT_F_BUTTON2 |
                       kD2255_EVENT_F_BUTTON1),
    kDIALOG_EVENT_BUTTONS_MASK  = EVENT_FLAG_MAKE(5, kDIALOG_EVENT_BUTTONS),
    kDIALOG_EVENT_PWR_BUTTON_MASK   = kDIALOG_EVENT_HOLD_BUTTON_MASK,
    
    kDIALOG_EVENT_ALARM_MASK        = EVENT_FLAG_MAKE(2, kD2255_EVENT_C_ALARM),
    kDIALOG_EVENT_ACC_DET_MASK      = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_VBUS_DET_MASK     = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_VBUS_EXT_REM_MASK = kDIALOG_NOTEXIST_MASK,

    // FIXME: forcing ADC1
    kDIALOG_EVENT_EOMC_MASK         = EVENT_FLAG_MAKE(19, kD2255_EVENT_T_EOMC1),
    kDIALOG_EVENT_HIB_MASK          = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_ABCC_MASK         = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_CHG_END_MASK      = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_TBAT_MASK         = kDIALOG_NOTEXIST_MASK,
    
    kDIALOG_EVENT_GPIO1_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO1),
    kDIALOG_EVENT_GPIO2_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO2),
    kDIALOG_EVENT_GPIO3_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO3),
    kDIALOG_EVENT_GPIO4_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO4),
    kDIALOG_EVENT_GPIO5_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO5),
    kDIALOG_EVENT_GPIO6_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO6),
    kDIALOG_EVENT_GPIO7_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO7),
    kDIALOG_EVENT_GPIO8_MASK    = EVENT_FLAG_MAKE(6, kD2255_EVENT_G_GPIO8),
    
    kDIALOG_EVENT_GPIO9_MASK    = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO9),
    kDIALOG_EVENT_GPIO10_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO10),
    kDIALOG_EVENT_GPIO11_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO11),
    kDIALOG_EVENT_GPIO12_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO12),
    kDIALOG_EVENT_GPIO13_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO13),
    kDIALOG_EVENT_GPIO14_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO14),
    kDIALOG_EVENT_GPIO15_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO15),
    kDIALOG_EVENT_GPIO16_MASK   = EVENT_FLAG_MAKE(7, kD2255_EVENT_H_GPIO16),
    
    kDIALOG_EVENT_GPIO17_MASK   = EVENT_FLAG_MAKE(8, kD2255_EVENT_I_GPIO17),
    kDIALOG_EVENT_GPIO18_MASK   = EVENT_FLAG_MAKE(8, kD2255_EVENT_I_GPIO18),
    kDIALOG_EVENT_GPIO19_MASK   = EVENT_FLAG_MAKE(8, kD2255_EVENT_I_GPIO19),
    kDIALOG_EVENT_GPIO20_MASK   = EVENT_FLAG_MAKE(8, kD2255_EVENT_I_GPIO20),
    kDIALOG_EVENT_GPIO21_MASK   = EVENT_FLAG_MAKE(8, kD2255_EVENT_I_GPIO21),

    // TODO: check HOLD, MENU, RINGER
    kDIALOG_EVENT_HOLD_DBL_MASK     = EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON2_DBL),
    kDIALOG_EVENT_MENU_DBL_MASK     = EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON1_DBL),
    kDIALOG_EVENT_RINGER_DBL_MASK   = EVENT_FLAG_MAKE(5, kD2255_EVENT_F_BUTTON3_DBL),

    kDIALOG_EVENT_VHP_DET_MASK  = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_ON_MASK       = kDIALOG_NOTEXIST_MASK,
    kDIALOG_EVENT_LDO2_EN_MASK  = kDIALOG_NOTEXIST_MASK,
};

enum {
    kDIALOG_STATUS_USB_MASK         = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_VBUS_MASK        = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_FW_MASK          = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_ACC_DET_MASK     = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_CHARGING_MASK    = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_CHG_TO_MASK      = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_CHG_END_MASK     = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_TBAT_MASK        = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_CHG_ATT_MASK     = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_ABCC_MASK        = kDIALOG_NOTEXIST_MASK,
};

static const statusRegisters kDialogStatusFWMask = { };

enum {
    kD2255_EVENT_A_WAKEMASK = 0,
    kD2255_EVENT_B_WAKEMASK = 0,
    kD2255_EVENT_C_WAKEMASK = kD2255_EVENT_C_ALARM ,
    kD2255_EVENT_D_WAKEMASK = 0,
    kD2255_EVENT_E_WAKEMASK = 0,
    kD2255_EVENT_F_WAKEMASK = kD2255_EVENT_F_BUTTON4 | kD2255_EVENT_F_BUTTON3 | 
                              kD2255_EVENT_F_BUTTON2 | kD2255_EVENT_F_BUTTON1 ,
    kD2255_EVENT_G_WAKEMASK = 0,
    kD2255_EVENT_H_WAKEMASK = 0,
    kD2255_EVENT_I_WAKEMASK = 0,
    kD2255_EVENT_J_WAKEMASK = 0,
    kD2255_EVENT_K_WAKEMASK = 0,
    kD2255_EVENT_L_WAKEMASK = 0,
    kD2255_EVENT_M_WAKEMASK = 0,
    kD2255_EVENT_N_WAKEMASK = 0,
    kD2255_EVENT_O_WAKEMASK = 0,
    kD2255_EVENT_P_WAKEMASK = 0,
    kD2255_EVENT_Q_WAKEMASK = 0,
    kD2255_EVENT_R_WAKEMASK = 0,
    kD2255_EVENT_S_WAKEMASK = 0,
    kD2255_EVENT_T_WAKEMASK = 0,
    kD2255_EVENT_U_WAKEMASK = 0,
    kD2255_EVENT_V_WAKEMASK = 0,
    kD2255_EVENT_W_WAKEMASK = 0,
    kD2255_EVENT_X_WAKEMASK = 0,
    kD2255_EVENT_Y_WAKEMASK = 0,
};

// All events that are masked during shutdown - inverse of the wake mask,
// events that wake up the system
static const eventRegisters kDialogEventIntMasks = {
    ~kD2255_EVENT_A_WAKEMASK,
    ~kD2255_EVENT_B_WAKEMASK,
    ~kD2255_EVENT_C_WAKEMASK,
    ~kD2255_EVENT_D_WAKEMASK,
    ~kD2255_EVENT_E_WAKEMASK,
    ~kD2255_EVENT_F_WAKEMASK,
    ~kD2255_EVENT_G_WAKEMASK,
    ~kD2255_EVENT_H_WAKEMASK,
    ~kD2255_EVENT_I_WAKEMASK,
    ~kD2255_EVENT_J_WAKEMASK,
    ~kD2255_EVENT_K_WAKEMASK,
    ~kD2255_EVENT_L_WAKEMASK,
    ~kD2255_EVENT_M_WAKEMASK,
    ~kD2255_EVENT_N_WAKEMASK,
    ~kD2255_EVENT_O_WAKEMASK,
    ~kD2255_EVENT_P_WAKEMASK,
    ~kD2255_EVENT_Q_WAKEMASK,
    ~kD2255_EVENT_R_WAKEMASK,
    ~kD2255_EVENT_S_WAKEMASK,
    ~kD2255_EVENT_T_WAKEMASK,
    ~kD2255_EVENT_U_WAKEMASK,
    ~kD2255_EVENT_V_WAKEMASK,
    ~kD2255_EVENT_W_WAKEMASK,
    ~kD2255_EVENT_X_WAKEMASK,
    ~kD2255_EVENT_Y_WAKEMASK,
};

// All wake events without the buttons
static const eventRegisters kDialogEventNotButtonMasks = {
    kD2255_EVENT_A_WAKEMASK,
    kD2255_EVENT_B_WAKEMASK,
    kD2255_EVENT_C_WAKEMASK,
    kD2255_EVENT_D_WAKEMASK,
    kD2255_EVENT_E_WAKEMASK,
    kD2255_EVENT_F_WAKEMASK & ~kDIALOG_EVENT_BUTTONS,
    kD2255_EVENT_G_WAKEMASK,
    kD2255_EVENT_H_WAKEMASK,
    kD2255_EVENT_I_WAKEMASK,
    kD2255_EVENT_J_WAKEMASK,
    kD2255_EVENT_K_WAKEMASK,
    kD2255_EVENT_L_WAKEMASK,
    kD2255_EVENT_M_WAKEMASK,
    kD2255_EVENT_N_WAKEMASK,
    kD2255_EVENT_O_WAKEMASK,
    kD2255_EVENT_P_WAKEMASK,
    kD2255_EVENT_Q_WAKEMASK,
    kD2255_EVENT_R_WAKEMASK,
    kD2255_EVENT_S_WAKEMASK,
    kD2255_EVENT_T_WAKEMASK,
    kD2255_EVENT_U_WAKEMASK,
    kD2255_EVENT_V_WAKEMASK,
    kD2255_EVENT_W_WAKEMASK,
    kD2255_EVENT_X_WAKEMASK,
    kD2255_EVENT_Y_WAKEMASK,
};

// All events indicating external power supply
// XXX: D2255 verify this, when I have vbus? GPIO, maybe..
static const eventRegisters kDialogEventPwrsupplyMask = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const eventRegisters kDialogEventUSBMask = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const eventRegisters kDialogEventFWMask = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

enum {
    kDialogEventPwrsupplyCount = 0,
    kDialogEventUSBCount = 0,
    kDialogEventFWCount = 0,
};

enum {
    // FAULT_LOG_A (SYSCTL_FAULT_LOG1)
    kDIALOG_FAULT_LOG_WDOG          = FAULTLOG_FLAG_MAKE(0, (1 << 7)),
    kDIALOG_FAULT_LOG_RESET_IN_3    = FAULTLOG_FLAG_MAKE(0, (1 << 6)),
    kDIALOG_FAULT_LOG_RESET_IN_2    = FAULTLOG_FLAG_MAKE(0, (1 << 5)),
    kDIALOG_FAULT_LOG_RESET_IN_1    = FAULTLOG_FLAG_MAKE(0, (1 << 4)),
    kDIALOG_FAULT_LOG_RST           = FAULTLOG_FLAG_MAKE(0, (1 << 3)),
    kDIALOG_FAULT_LOG_POR           = FAULTLOG_FLAG_MAKE(0, (1 << 2)),
    kDIALOG_FAULT_LOG_OVER_TEMP     = FAULTLOG_FLAG_MAKE(0, (1 << 1)),
    kDIALOG_FAULT_LOG_VDD_UNDER     = FAULTLOG_FLAG_MAKE(0, (1 << 0)),
    // FAULT_LOG_B (SYSCTL_FAULT_LOG2)
    kDIALOG_FAULT_LOG_BTN_SHUTDOWN      = FAULTLOG_FLAG_MAKE(1, (1 << 2)),
    kDIALOG_FAULT_LOG_TWO_FINGER_RESET  = FAULTLOG_FLAG_MAKE(1, (1 << 1)),
    kDIALOG_FAULT_LOG_NTC_SHDN          = FAULTLOG_FLAG_MAKE(1, (1 << 0)),
};

// ---------------------------------------------------------------------------
// GPADC
// ---------------------------------------------------------------------------


enum {
    kD2255_ADC_CONTROL_MUX_SEL_VDD_MAIN     = 0,
    kD2255_ADC_CONTROL_MUX_SEL_BRICK_ID     = 1,
    kD2255_ADC_CONTROL_MUX_SEL_SPARE0       = 2,
    kD2255_ADC_CONTROL_MUX_SEL_APP_MUX_A    = 3,
    kD2255_ADC_CONTROL_MUX_SEL_APP_MUX_B    = 4,
    // ...
    kD2255_ADC_CONTROL_MUX_SEL_VLDO1    = 18,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO1    = 19,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO2    = 20,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO2    = 21,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO3    = 22,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO3    = 23,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO4    = 24,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO4    = 25,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO5    = 26,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO5    = 26,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO6    = 28,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO6    = 29,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO7    = 30,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO7    = 31,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO8    = 32,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO8    = 33,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO9    = 34,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO9    = 35,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO10   = 36,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO10   = 37,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO11   = 38,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO11   = 39,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO12   = 40,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO12   = 41,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO13   = 42,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO13   = 43,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO14   = 44,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO14   = 45,
    kD2255_ADC_CONTROL_MUX_SEL_VLDO15   = 46,
    kD2255_ADC_CONTROL_MUX_SEL_ILDO15   = 47,
    // ReatlimeClock
    kD2255_ADC_CONTROL_MUX_SEL_VRTC = 50,
    kD2255_ADC_CONTROL_MUX_SEL_IRTC = 51,
    // high impedance external input
    kD2255_ADC_CONTROL_MUX_SEL_ADC_IN   = 82,
    // NTCs
    kD2255_ADC_CONTROL_MUX_SEL_TJNCT = 88,
    kD2255_ADC_CONTROL_MUX_SEL_TCAL =  89,
    kD2255_ADC_CONTROL_MUX_SEL_TDEV1 = 90,
    kD2255_ADC_CONTROL_MUX_SEL_TDEV2 = 91,
    kD2255_ADC_CONTROL_MUX_SEL_TDEV3 = 92,
    kD2255_ADC_CONTROL_MUX_SEL_TDEV4 = 93,
    // ...
    // TINT, measure TDIE (TDIE1,TDIE11)
    kD2255_ADC_CONTROL_MUX_SEL_LDO5_TEMP    = 112,
    kD2255_ADC_CONTROL_MUX_SEL_LDO10_TEMP   = 113,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK0_TEMP   = 114,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK1_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK2_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK3_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK4_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK5_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK6_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK7_TEMP,
    kD2255_ADC_CONTROL_MUX_SEL_BUCK8_TEMP   = 122,

    kD2255_ADC_CONTROL_MUX_SEL_VBUCK0       = 144,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK1,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK2,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK3,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK4,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK5,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK6,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK7,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK8       = 152,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK0       = 176,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK1,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK2,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK3,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK4,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK5,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK6,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK7,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK8       = 184,

    kD2255_ADC_CONTROL_MUX_SEL_IBUCK0_OFF   = 208, // offset
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK1_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK2_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK3_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK4_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK5_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK6_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK7_OFF,
    kD2255_ADC_CONTROL_MUX_SEL_IBUCK8_OFF   = 216,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK3_SW1   = 240,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK3_SW2,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK3_SW3,
    kD2255_ADC_CONTROL_MUX_SEL_VBUCK4_SW1   = 243,
    
    kDIALOG_ADC_CONTROL_MUX_SEL_VDD_OUT     = kD2255_ADC_CONTROL_MUX_SEL_VDD_MAIN,
    kDIALOG_ADC_CONTROL_MUX_SEL_BRICK_ID    = kD2255_ADC_CONTROL_MUX_SEL_BRICK_ID,
    kDIALOG_ADC_CONTROL_MUX_SEL_NTC0        = kD2255_ADC_CONTROL_MUX_SEL_TCAL,
    kDIALOG_ADC_CONTROL_MUX_NUM_NTC         = 5,
    kDIALOG_ADC_CONTROL_MUX_SEL_TINT_START  = kD2255_ADC_CONTROL_MUX_SEL_LDO5_TEMP,
    kDIALOG_ADC_CONTROL_MUX_SEL_TINT_END    = kD2255_ADC_CONTROL_MUX_SEL_BUCK8_TEMP,
    kDIALOG_ADC_CONTROL_MUX_SEL_TJUNC       = kD2255_ADC_CONTROL_MUX_SEL_TJNCT,
    
    // TODO: verify BRICK_ID_OFFSET
    kDIALOG_ADC_BRICK_ID_OFFSET_MV          = 40,
    
#if 0
    kDIALOG_ADC_CONTROL_MAN_CONV        = (1 << 7),
    kDIALOG_ADC_CONTROL2_IBUS_EN        = (1 << 0),
    kDIALOG_ADC_CONTROL2_ADC_REF_EN     = (1 << 6),
    kDIALOG_ADC_CONTROL2_AUTO_VDD_OUT_EN    = (1 << 7),
#endif
};

enum {

//    kDIALOG_ADC_CONTROL_DEFAULTS = 0,
//    kDIALOG_ADC_CONTROL2_DEFAULTS = 0,
//    kDIALOG_ADC_CONTROL_ADC_REF_EN = 0,

    kDIALOG_ADC_LSB_MANADC_ERROR        = (1 << 7),
    kDIALOG_ADC_LSB_ADC_OVL             = (1 << 6),

    kDIALOG_ADC_MAN_CTL               = kD2255_GPADC_MAN_CTRL1,
    kDIALOG_ADC_CONTROL_MUX_SEL_MASK  = kD2255_GPADC_MAN_CTRL1_MASK,
    kDIALOG_ADC_LSB                   = kD2255_GPADC_MAN1_RES_LSB,
    
    kDIALOG_ADC_FULL_SCALE_MV   = 2500,
    kDIALOG_ADC_RESOLUTION_BITS = 12,
};

#if 0
enum {
    kDIALOG_APP_TMUX_SEL_MASK       = (7 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_0     = (0 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_1     = (1 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_2     = (2 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_3     = (3 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_4     = (4 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_5     = (5 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_6     = (6 << 0),
    kDIALOG_APP_TMUX_SEL_AMUX_7     = (7 << 0),
    KDIALOG_APP_TMUX_EN         = (1 << 7),
};
#endif

// ------------------------------------------------
//
// ------------------------------------------------

enum {
    kDIALOG_RTC_CONTROL_MONITOR         = (1 << 0),
    kDIALOG_RTC_CONTROL_ALARM_EN        = (1 << 6),
};

enum {
    kD2255_GPIO_IO_CONFIG_IN          = (0<<6),
    kD2255_GPIO_IO_CONFIG_OUT_OD      = (1<<6),
    kD2255_GPIO_IO_CONFIG_OUT_PP      = (2<<6), 
// NOTE: Once a GPIO is set to a safe spare configuration, the safe spare setting must
// be changed  before any changes to the associated GPIO_x_IO_TYPE, GPIOx_PUPD and 
// GPIOx_WAKE_LVL register fields can be done.
    kD2255_GPIO_IO_CONFIG_SAFE_SPARE  = (3<<6),

    kD2255_GPIO_IO_TYPE_OUT_LVL         = (0<<3),
    kD2255_GPIO_IO_TYPE_OUT_32K         = (1<<3),
    kD2255_GPIO_IO_TYPE_OUT_nRST        = (2<<3),
    kD2255_GPIO_IO_TYPE_OUT_TEMP        = (3<<3),
    kD2255_GPIO_IO_TYPE_OUT_LVL2        = (4<<3),
    kD2255_GPIO_IO_TYPE_IN_LEVEL_HIGH   = (0<<3),
    kD2255_GPIO_IO_TYPE_IN_LEVEL_LOW    = (1<<3),
    kD2255_GPIO_IO_TYPE_IN_EDGE_RISING  = (2<<3),
    kD2255_GPIO_IO_TYPE_IN_EDGE_FALLING = (3<<3),
    kD2255_GPIO_IO_TYPE_IN_EDGE_ANY     = (4<<3),

    kDIALOG_SYS_GPIO_CONFIG_MASK          = (0x3<<6),
    kDIALOG_SYS_GPIO_DIRECTION_MASK       = (0x3<<3),
    kDIALOG_SYS_GPIO_VALUE_MASK           = (1<<0),

    kDIALOG_SYS_GPIO_DIRECTION_OUT              = kD2255_GPIO_IO_TYPE_OUT_LVL,
    kDIALOG_SYS_GPIO_DIRECTION_OUT_32KHZ        = kD2255_GPIO_IO_TYPE_OUT_32K,
    kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_HIGH    = kD2255_GPIO_IO_TYPE_IN_LEVEL_HIGH,
    kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_LOW     = kD2255_GPIO_IO_TYPE_IN_LEVEL_LOW,

    kDIALOG_SYS_GPIO_OUTPUT_LEVEL_LOW       = (0 << 0),
    kDIALOG_SYS_GPIO_OUTPUT_LEVEL_HIGH      = (1 << 0),
    kDIALOG_SYS_GPIO_INPUT_WAKE             = (1 << 0),
};

// NOTE: it doesn't work with SAFESPARE
#define IS_GPIO_OUTPUT(gpio) \
    (((gpio)&kDIALOG_SYS_GPIO_CONFIG_MASK)!=kD2255_GPIO_IO_CONFIG_IN )

#define kDIALOG_STATUS_GPIO_MASK(gpio) \
    STATUS_FLAG_MAKE(6 + ((gpio)/8), (1 << ((gpio) % 8)) )

#define NUM_LDOS 29
// NOTE: here bypass is the OFFSET of the bypass register from VSEL
static const struct ldo_params ldo_D2255[NUM_LDOS] = {
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO1_VSEL,  kD2255_LDO1_EN,  1 },    // 00 - LDO1  =
    { 1200,  5, 0xff, 0xff,    2, kD2255_LDO2_VSEL,  kD2255_LDO2_EN,  1 },    // 01 - LDO2  = (Byp)
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO3_VSEL,  kD2255_LDO3_EN,  1 },    // 02 - LDO3  =
    {  600,  5, 0xff, 0xff, 0x00, kD2255_LDO4_VSEL,  kD2255_LDO4_EN,  1 },    // 03 - LDO4  =
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO5_VSEL,  kD2255_LDO5_EN,  1 },    // 04 - LDO5  =
    { 1200, 25, 0x7f, 0x7f,    2, kD2255_LDO6_VSEL,  kD2255_LDO6_EN,  1 },    // 05 - LDO6  = (Byp)
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO7_VSEL,  kD2255_LDO7_EN,  1 },    // 06 - LDO7  =
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO8_VSEL,  kD2255_LDO8_EN,  1 },    // 07 - LDO8  =
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO9_VSEL,  kD2255_LDO9_EN,  1 },    // 08 - LDO9  =
    {  600,  5, 0xff, 0xff, 0x00, kD2255_LDO10_VSEL, kD2255_LDO10_EN, 1 },    // 09 - LDO10 -
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO11_VSEL, kD2255_LDO11_EN, 1 },    // 0a - LDO11 =
    { 1800,  0,    0, 0,    0x00, 0, 0, 0 },                                  // 0b - there is no LDO12 (PP1V8_ALWAYS)
    { 2400,  5, 0xff, 0xff, 0x00, kD2255_LDO13_VSEL, kD2255_LDO13_EN, 1 },    // 0c - LDO13 =
    {  600,  5, 0xff, 0xff, 0x00, kD2255_LDO14_VSEL, kD2255_LDO14_EN, 1 },    // 0d - LDO14 =
    { 1200,  5, 0xff, 0xff,    2, kD2255_LDO15_VSEL, kD2255_LDO15_EN, 1 },    // 0e - LDO15 = (Byp)

    {  500,  5, 0, 0, 0x00, kD2255_BUCK0_VSEL, kD2255_BUCK0_EN, 1 },    // 0f - POWER_RAIL_CPU
    {  500,  5, 0, 0, 0x00, kD2255_BUCK1_VSEL, kD2255_BUCK1_EN, 1 },    // 10 - POWER_RAIL_GPU
    {  600,  5, 0, 0, 0x00, kD2255_BUCK2_VSEL, kD2255_BUCK2_EN, 1 },    // 11 - POWER_RAIL_SOC
    { 1100,  5, 0, 0, 0x00, kD2255_BUCK3_VSEL, kD2255_BUCK3_EN, 1 },    // 12
    {  600,  5, 0, 0, 0x00, kD2255_BUCK4_VSEL, kD2255_BUCK4_EN, 1 },    // 13
    {  600,  5, 0, 0, 0x00, kD2255_BUCK5_VSEL, kD2255_BUCK5_EN, 1 },    // 14 - POWER_RAIL_VDD_FIXED
    { 1100,  5, 0, 0,    8, kD2255_BUCK6_VSEL, kD2255_BUCK6_EN, 1 },    // 15 - BUCK6 (Byp)
    {  600,  5, 0, 0, 0x00, kD2255_BUCK7_VSEL, kD2255_BUCK7_EN, 1 },    // 16 - POWER_RAIL_CPU_RAM
    {  600,  5, 0, 0, 0x00, kD2255_BUCK8_VSEL, kD2255_BUCK8_EN, 1 },    // 17 - POWER_RAIL_GPU_RAM

    {  0,  0, 0, 0, 0x00, 0, kD2255_BUCK3_SW1_EN, 1 },                // 18
    {  0,  0, 0, 0, 0x00, 0, kD2255_BUCK3_SW2_EN, 1 },                // 19
    {  0,  0, 0, 0, 0x00, 0, kD2255_BUCK3_SW3_EN, 1 },                // 1a
    {  0,  0, 0, 0, 0x00, 0, kD2255_BUCK4_SW1_EN, 1 },                // 1b

    {  0,  0, 0, 0, 0x00, 0, kD2255_LDO6_EN, 7<<1 },                  // 1c
};
#define LDOP ldo_D2255

enum {
    kDIALOG_BUTTON_DBL_CLICK_RATE_MASK	= (7 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_10MS	= (0 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_20MS	= (1 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_50MS	= (2 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_60MS	= (3 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_100MS	= (4 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_200MS	= (5 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_300MS	= (6 << 0),
    kDIALOG_BUTTON_DBL_CLICK_RATE_400MS	= (7 << 0),

    kD2255_BUTTON_DBL_BTN1_DBL_EN	= (1 << 3),
    kD2255_BUTTON_DBL_BTN2_DBL_EN	= (1 << 4),
    kD2255_BUTTON_DBL_BTN3_DBL_EN	= (1 << 5),
    kD2255_BUTTON_DBL_BTN4_DBL_EN	= (1 << 6),
    
    // TODO: verify this!
    kDIALOG_BUTTON_DBL_HOLD_DBL_EN	= kD2255_BUTTON_DBL_BTN2_DBL_EN,
    kDIALOG_BUTTON_DBL_MENU_DBL_EN	= kD2255_BUTTON_DBL_BTN1_DBL_EN,
    kDIALOG_BUTTON_DBL_RINGER_DBL_EN	= kD2255_BUTTON_DBL_BTN3_DBL_EN,
};

#endif /* __DIALOG_D2255_H */
