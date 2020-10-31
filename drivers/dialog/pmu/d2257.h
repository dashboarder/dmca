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

// Specs at <rdar://problem/16832282> Aria: Specification and Data-sheet from Vendor (Dialog)
#ifndef __DIALOG_D2257_H
#define __DIALOG_D2257_H

#define PMU_HAS_SWI             0
#define PMU_HAS_DWI             0
#define PMU_HAS_SPI             1
// TODO: use single register alias (and a value alias) for HAS_SWI,HAS_DWI,HAS_SPI
#define PMU_HAS_LCM_LDO         0
// TODO: add BIST ADC
#define PMU_HAS_BIST_ADC        0
#define PMU_HAS_CHG_ABCC_FLAG   0
#define PMU_HAS_VIB             0
#define PMU_HAS_RAM             1
// TODO: enable wled
#define PMU_HAS_WLED            1
#define PMU_HAS_32K_RTC         1
// TODO: enable accumulators
#define PMU_HAS_ACCUMULATORS    0
#define PMU_HAS_GPIO_CONF       1
#define PMU_HAS_SYS             1
#define PMU_HAS_SCRATCH         1
#define PMU_HAS_CHARGER         1
#define PMU_HAS_PERF_STATES     1
#define PMU_DRV_NEW_DRIVER      1

enum {
	kDIALOG_ADDR_R		= 0x79,
	kDIALOG_ADDR_W		= 0x78,
	kDIALOG_REG_BYTES	= 2,
};

enum {
        kDIALOG_FAULTLOG_COUNT  = 2,      // @0x0100 (read only the first 2)
	kDIALOG_EVENT_COUNT	= 0x1f,   // @0x0140
	kDIALOG_STATUS_COUNT	= 0x1f,   // @0x0180
	kDIALOG_CHIPID_COUNT	= 4,      // @0x0000
	kDIALOG_GPIO_COUNT	= 22,
};

#include "dCommon.h"

enum {
// ---------------------------------    
// Faultlog (virtual)
// ---------------------------------    
    kDIALOG_FAULT_LOG_A     = 0x0100,
    // ...
    // kDIALOG_FAULT_LOG_R

// ---------------------------------    
// Event, Status ,IRQ Mask (Virtual)
// ---------------------------------    
    
    kD2255_V_EVENT_AA         = 0x0140, // 0
    kD2255_V_EVENT_AB         = 0x0141, // 1
    kD2255_V_EVENT_AC         = 0x0142, // 2
    kD2255_V_EVENT_AD         = 0x0143, // 3
    kD2255_V_EVENT_AE         = 0x0144, // 4
    kD2255_V_EVENT_AF         = 0x0145, // 5 BTNs
    kD2255_V_EVENT_AG         = 0x0146, // 6 GPIO1-8
    kD2255_V_EVENT_AH         = 0x0147, // 7 GPIO9-16
    kD2255_V_EVENT_AI         = 0x0148, // 8 GPIO17-22
    kD2255_V_EVENT_AJ         = 0x0149, // 9 VBUS/VCENTER
    kD2255_V_EVENT_AK         = 0x014a, // 10
    kD2255_V_EVENT_AL         = 0x014b, // 11
    kD2255_V_EVENT_AM         = 0x014c, // 12
    kD2255_V_EVENT_AN         = 0x014d, // 13
    kD2255_V_EVENT_AO         = 0x014e, // 14
    kD2255_V_EVENT_AP         = 0x014f, // 15
    kD2255_V_EVENT_AQ         = 0x0150, // 16
    kD2255_V_EVENT_AR         = 0x0151, // 17
    kD2255_V_EVENT_AS         = 0x0152, // 18
    kD2255_V_EVENT_AT         = 0x0153, // 19
    kD2255_V_EVENT_AU         = 0x0154, // 20
    kD2255_V_EVENT_AV         = 0x0155, // 21
    kD2255_V_EVENT_AW         = 0x0156, // 22
    kD2255_V_EVENT_AX         = 0x0157, // 23 EOMC
    kD2255_V_EVENT_AY         = 0x0158, // 24
    kD2255_V_EVENT_AZ,
    kD2255_V_EVENT_BA,
    kD2255_V_EVENT_BB,
    kD2255_V_EVENT_BC,
    kD2255_V_EVENT_BD,
    kD2255_V_EVENT_BE,

    kD2255_V_STATUS_AA        = 0x0180,
    // .... V_STATUS_Y
    kD2255_V_STATUS_BE        = 0x019e,


    kD2255_V_IRQ_MASK_AA      = 0x01c0,
    // .... V_IRQ_MASK_Y
    kD2255_V_IRQ_MASK_BE      = 0x01de,

    kD2255_V_FAULTLOG_COUNT      = 0x12,
    kD2255_V_EVENT_COUNT         = 0x1f,
    kD2255_V_STATUS_COUNT        = 0x1f,

    kDIALOG_EVENT_A         = kD2255_V_EVENT_AA,
    kDIALOG_STATUS_A        = kD2255_V_STATUS_AA,
    kDIALOG_IRQ_MASK_A      = kD2255_V_IRQ_MASK_AA,
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
    kD2257_APP_TMUX         = 0x0212,
};

// ---------------------------------    
// System Configuration
// ---------------------------------    

enum {
    kD2257_SYSCTL_PRE_UVLO_CONF = 0x026b,
};

// ---------------------------------    
// Faultlog
// ---------------------------------    

enum {
    kD2257_FAULT_LOG1       = 0x02c0,
    kD2257_FAULT_LOG2       = 0x02c1,

    kDIALOG_FAULT_LOG       = kD2257_FAULT_LOG1,
    kDIALOG_FAULT_LOG2      = kD2257_FAULT_LOG2,
};

// ---------------------------------    
// Events. Status and IRQ Control
// ---------------------------------    

enum {
    kD2257_SYSCTL_EVENT     = 0x02d2,
    kD2257_SYSCTL_STATUS    = 0x02d3,
    kD2257_SYSCTL_IRQ_MASK  = 0x02d4,
};

// ---------------------------------    
// Power Supply Control 
// Active,Standby,Hibernate are now encoded here, need TestAccess
// ---------------------------------    
enum {
    kD2257_PWRONOFF_BUCK0_EN = 0x0300,
    kD2257_PWRONOFF_BUCK1_EN = 0x0301,
    kD2257_PWRONOFF_BUCK2_EN = 0x0302,
    kD2257_PWRONOFF_BUCK3_EN = 0x0303,
    kD2257_PWRONOFF_BUCK4_EN = 0x0304,
    kD2257_PWRONOFF_BUCK5_EN = 0x0305,
    kD2257_PWRONOFF_BUCK6_EN = 0x0306,
    kD2257_PWRONOFF_BUCK7_EN = 0x0307,
    kD2257_PWRONOFF_BUCK8_EN = 0x0308,

    kD2257_PWRONOFF_WLEDA_EN  = 0x0309,
    kD2257_PWRONOFF_WLEDB_EN  = 0x030A,

    kD2257_PWRONOFF_LDO1_EN   = 0x030b,
    kD2257_PWRONOFF_LDO2_EN   = 0x030c,
    kD2257_PWRONOFF_LDO3_EN   = 0x030d,
    kD2257_PWRONOFF_LDO4_EN   = 0x030e,
    kD2257_PWRONOFF_LDO5_EN   = 0x030f,
    kD2257_PWRONOFF_LDO6_EN   = 0x0310,
    kD2257_PWRONOFF_LDO7_EN   = 0x0311,
    kD2257_PWRONOFF_LDO8_EN   = 0x0312,
    kD2257_PWRONOFF_LDO9_EN   = 0x0313,
    kD2257_PWRONOFF_LDO10_EN  = 0x0314,
    kD2257_PWRONOFF_LDO11_EN  = 0x0315,
    kD2257_PWRONOFF_LDO13_EN  = 0x0316,
    kD2257_PWRONOFF_LDO14_EN  = 0x0317,
    kD2257_PWRONOFF_LDO15_EN  = 0x0318,
    kD2257_PWRONOFF_LDO16_EN  = 0x0319,

    kD2257_PWRONOFF_LCM_BST_SW_EN   = 0x031a,
    kD2257_PWRONOFF_LCM_BST_EN      = 0x031b,
    kD2257_PWRONOFF_LCM_LDO1_EN     = 0x031c,
    kD2257_PWRONOFF_LCM_LDO2_EN     = 0x031d,

    kD2257_PWRONOFF_CP_EN           = 0x031e,

    kD2257_PWRONOFF_BUCK3_SW1_EN    = 0x031f,
    kD2257_PWRONOFF_BUCK3_SW2_EN    = 0x0320,
    kD2257_PWRONOFF_BUCK3_SW3_EN    = 0x0321,
    kD2257_PWRONOFF_BUCK4_SW1_EN    = 0x0322,
    kD2257_PWRONOFF_BUCK4_SW2_EN    = 0x0323,
    
    kD2257_PWRONOFF_GPIO_32K_EN     = 0x0324,
    kD2257_PWRONOFF_OUT_32K_EN      = 0x0325,
    kD2257_PWRONOFF_SLEEP_32K_EN    = 0x0326,
    kD2257_PWRONOFF_NRESET_EN       = 0x0327,
    kD2257_PWRONOFF_SYS_ALIVE_EN    = 0x0328,
    kD2257_PWRONOFF_PRE_UVLO_EN     = 0x0329,

    kDIALOG_PWRONOFF_WLEDA_EN       = kD2257_PWRONOFF_WLEDA_EN,
    kDIALOG_PWRONOFF_WLEDB_EN       = kD2257_PWRONOFF_WLEDB_EN
};

// ---------------------------------    
// Main FSM, MFSM Events
// ---------------------------------    

enum {
    kD2257_MFSM_CTRL            = 0x0400,
    kD2257_ACT_TO_SLEEP3_DLY    = 0x0401,
    kD2257_ACT_TO_OFF_DLY       = 0x0402,
    kD2257_OFF_TIMER_DLY        = 0x0403,
    kD2257_SLEEP3_TIMER_DLY     = 0x0404,
    kD2257_MFSM_CONF1           = 0x0460,

    kD2257_MFSM_CTRL_PERF_FLOOR_OFF     = (0<<2),
    kD2257_MFSM_CTRL_PERF_FLOOR_SLEEP3  = (1<<2),
    kD2257_MFSM_CTRL_PERF_FLOOR_SLEEP2  = (2<<2),
    kD2257_MFSM_CTRL_PERF_FLOOR_ACTIVE  = (3<<2),
    kD2257_MFSM_CTRL_FORCE_SLEEP3   = (1<<1),
    kD2257_MFSM_CTRL_FORCE_OFF      = (1<<0),

    kDIALOG_SYS_CONTROL_PERF_FLOOR_OFF      = kD2257_MFSM_CTRL_PERF_FLOOR_OFF,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_SLEEP3   = kD2257_MFSM_CTRL_PERF_FLOOR_SLEEP3,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_SLEEP2   = kD2257_MFSM_CTRL_PERF_FLOOR_SLEEP2,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_ACTIVE   = kD2257_MFSM_CTRL_PERF_FLOOR_ACTIVE,
    kDIALOG_SYS_CONTROL_PERF_FLOOR_MASK     = (0x3<<2),
    
    kDIALOG_SYS_CONTROL_HIBERNATE   = kD2257_MFSM_CTRL_FORCE_SLEEP3,
    kDIALOG_SYS_CONTROL_STANDBY     = kD2257_MFSM_CTRL_FORCE_OFF,
    // TODO: verify this
    kDIALOG_SYS_CONTROL_HIBERNATE_ALWAYS    = kDIALOG_SYS_CONTROL_HIBERNATE,

    kDIALOG_SYS_CONTROL     = kD2257_MFSM_CTRL,
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
    kD2257_SPI_CTRL             = 0x0600, // global enable
    kD2257_EN_CMD0              = 0x0601, // buck#_spi_en
    kD2257_EN_CMD1              = 0x0602,
    kD2257_EN_CMD2              = 0x0603, // wled spi access enable
    kD2257_EN_CMD3              = 0x0604, // buck#_spi_dis_master
    kD2257_EN_CMD4              = 0x0605,
    kD2257_SPI_CONFIG           = 0x0660,

    // set when PMU_HAS_SPI
    kDIALOG_SPI_CONTROL            = kD2257_SPI_CTRL,
    kDIALOG_SPI_CONTROL_SPI_EN     = 1,
};

// ---------------------------------    
// Clock Generation
// ---------------------------------    

enum {
    kD2257_SYS_CTRL_CONF0       = 0x0700,
    kD2257_SYS_CTRL_CONF1       = 0x0760,
    kD2257_RTC_CTRL0            = 0x0761,
    kD2257_RTC_CTRL1            = 0x0762,
    kD2257_RTC_CTRL2            = 0x0763,
    kD2257_OSC_CTRL0            = 0x0764,
    kD2257_INT_OSC_TRIM         = 0x0765,
    kD2257_CLK_REQ_FRC          = 0x0766,
    kD2257_BG_CTRL              = 0x0767,

    kD2257_CKLGEN_EVENT         = 0x07c0,
    kD2257_CKLGEN_STATUS        = 0x07c1,
    kD2257_CKLGEN_IRQ_MASK      = 0x07c2,
};

// ---------------------------------    
// WLED
// ---------------------------------    

enum {
    kD2257_WLED_ISET_LSB    = 0x0b00,
    kD2257_WLED_ISET_MSB    = 0x0b01,
    kD2257_WLED_PWM_LSB     = 0x0b02,
    kD2257_WLED_PWM_MSB     = 0x0b03,
    kD2257_WLED_CTRL1       = 0x0b04,
    kD2257_WLED_CTRL2       = 0x0b05,
    kD2257_WLED_CTRL3       = 0x0b06,
    kD2257_WLED_CTRL4       = 0x0b07,
    kD2257_WLED_CTRL4A      = 0x0b08,
    kD2257_WLED_CTRL5       = 0x0b09,
    kD2257_WLED_CTRL6       = 0x0b0a,

    kDIALOG_WLED_ISET       = kD2257_WLED_ISET_MSB,
    kDIALOG_WLED_ISET2      = kD2257_WLED_ISET_LSB,
    kDIALOG_WLED_CONTROL    = kD2257_WLED_CTRL1,
    kDIALOG_WLED_CONTROL1   = kD2257_WLED_CTRL1,
    kDIALOG_WLED_CONTROL2   = kD2257_WLED_CTRL2,
    kDIALOG_WLED_DWI_CONTROL= kD2257_WLED_CTRL2,
    kDIALOG_WLED_OPTIONS    = kD2257_WLED_CTRL2,
};

enum {
    kDIALOG_WLED_CONTROL_WLED_ENABLE1	= (1 << 0),
    kDIALOG_WLED_CONTROL_WLED_ENABLE2	= (1 << 1),
    kDIALOG_WLED_CONTROL_WLED_ENABLE3	= (1 << 2),
    kDIALOG_WLED_CONTROL_WLED_ENABLE4	= (1 << 3),
    kDIALOG_WLED_CONTROL_WLED_ENABLE5	= (1 << 4),
    kDIALOG_WLED_CONTROL_WLED_ENABLE6	= (1 << 5),
};

enum {
    kDIALOG_WLED_CONTROL2_WLED_DITH_RAMP_EN	= (1 << 3),
    kDIALOG_WLED_CONTROL2_WLED_DITH_EN	= (1 << 2),
    kDIALOG_WLED_CONTROL2_WLED_DWI_EN	= (1 << 1),
    kDIALOG_WLED_CONTROL2_WLED_RAMP_EN	= (1 << 0),
    
    kDIALOG_WLED_CONTROL_WLED_DWI_EN	= kDIALOG_WLED_CONTROL2_WLED_DWI_EN,
    
    kDIALOG_WLED_OPTIONS_DEFAULT	= kDIALOG_WLED_CONTROL2_WLED_DITH_EN,
    kDIALOG_WLED_OPTIONS_xWI_EN		= kDIALOG_WLED_CONTROL2_WLED_DWI_EN,
    kDIALOG_WLED_OPTIONS_MASK		= (kDIALOG_WLED_CONTROL2_WLED_RAMP_EN|
                                           kDIALOG_WLED_CONTROL2_WLED_DITH_EN|
                                           kDIALOG_WLED_CONTROL2_WLED_DITH_RAMP_EN),
};

#define WLED_ISET_BITS  11

// ---------------------------------    
// GPIO configuration
// ---------------------------------    

enum {
    kD2257_GPIO1_CONF1          = 0x0900,
    kD2257_GPIO1_CONF2          = 0x0901,
    kD2257_GPIO2_CONF1,
    kD2257_GPIO2_CONF2,
    kD2257_GPIO3_CONF1,
    kD2257_GPIO3_CONF2,
    kD2257_GPIO4_CONF1,
    kD2257_GPIO4_CONF2,
    kD2257_GPIO5_CONF1,
    kD2257_GPIO5_CONF2,
    kD2257_GPIO6_CONF1,
    kD2257_GPIO6_CONF2,
    kD2257_GPIO7_CONF1,
    kD2257_GPIO7_CONF2,
    kD2257_GPIO8_CONF1,
    kD2257_GPIO8_CONF2,
    kD2257_GPIO9_CONF1,
    kD2257_GPIO9_CONF2,
    kD2257_GPIO10_CONF1,
    kD2257_GPIO10_CONF2,
    kD2257_GPIO11_CONF1,
    kD2257_GPIO11_CONF2,
    kD2257_GPIO12_CONF1,
    kD2257_GPIO12_CONF2,
    kD2257_GPIO13_CONF1,
    kD2257_GPIO13_CONF2,
    kD2257_GPIO14_CONF1,
    kD2257_GPIO14_CONF2,
    kD2257_GPIO15_CONF1,
    kD2257_GPIO15_CONF2,
    kD2257_GPIO16_CONF1,
    kD2257_GPIO16_CONF2,
    kD2257_GPIO17_CONF1,
    kD2257_GPIO17_CONF2,
    kD2257_GPIO18_CONF1,
    kD2257_GPIO18_CONF2,
    kD2257_GPIO19_CONF1_SLP1	= 0x0924,
    kD2257_GPIO19_CONF1_SLP2,
    kD2257_GPIO19_CONF1_SLP3,
    kD2257_GPIO19_CONF1_OFF,
    kD2257_GPIO19_CONF1,
    kD2257_GPIO19_CONF2,
    kD2257_GPIO20_CONF1_SLP1	= 0x092A,
    kD2257_GPIO20_CONF1_SLP2,
    kD2257_GPIO20_CONF1_SLP3,
    kD2257_GPIO20_CONF1_OFF,
    kD2257_GPIO20_CONF1,
    kD2257_GPIO20_CONF2,
    kD2257_GPIO21_CONF1_SLP1	= 0x0930,
    kD2257_GPIO21_CONF1_SLP2,
    kD2257_GPIO21_CONF1_SLP3,
    kD2257_GPIO21_CONF1_OFF,
    kD2257_GPIO21_CONF1,
    kD2257_GPIO21_CONF2,
    kD2257_GPIO22_CONF1_SLP1	= 0x0936,
    kD2257_GPIO22_CONF1_SLP2,
    kD2257_GPIO22_CONF1_SLP3,
    kD2257_GPIO22_CONF1_OFF,
    kD2257_GPIO22_CONF1,
    kD2257_GPIO22_CONF2,
    kDIALOG_SYS_GPIO_REG_START  = kD2257_GPIO1_CONF1,

// until GPIO21_CONF1, GPIO21_CONF2 ... kDIALOG_GPIO_COUNT gpios
    
    kD2257_OUT_32K              = 0x093c,
    kD2257_SLEEP_32K            = 0x093d,
    kD2257_ACTIVE_RDY           = 0x093e,
    kD2257_SLEEP1_RDY           = 0x093f,

// GPI  Control
    kD2257_BUTTON1_CONF         = 0x0960,
    kD2257_BUTTON2_CONF         = 0x0961,
    kD2257_BUTTON3_CONF         = 0x0962,
    kD2257_BUTTON4_CONF         = 0x0963,
    kD2257_BUTTON_DBL           = 0x0964,
    kD2257_BUTTON_WAKE          = 0x0965,

    kD2257_RESET_IN1_CONF1      = 0x0966,
    kD2257_RESET_IN1_CONF2      = 0x0967,
    kD2257_RESET_IN2_CONF1      = 0x0968,
    kD2257_RESET_IN2_CONF2      = 0x0969,
    kD2257_RESET_IN3_CONF1      = 0x096a,
    kD2257_RESET_IN3_CONF2      = 0x096b,

    kDIALOG_BUTTON_DBL          = kD2257_BUTTON_DBL,
};

// ---------------------------------    
// Event,Status,IRQ Control
// ---------------------------------    

enum {
    kD2257_IO_EVENT_A         = 0x09c0,
    kD2257_IO_EVENT_B         = 0x09c1,
    kD2257_IO_EVENT_C         = 0x09c2,
    kD2257_IO_EVENT_D         = 0x09c3,

    kD2257_IO_STATUS_A        = 0x09c4,
    kD2257_IO_STATUS_B        = 0x09c5,
    kD2257_IO_STATUS_C        = 0x09c6,
    kD2257_IO_STATUS_D        = 0x09c7,

    kD2257_IO_IRQ_MASK_A      = 0x09c8,
    kD2257_IO_IRQ_MASK_B      = 0x09c9,
    kD2257_IO_IRQ_MASK_C      = 0x09ca,
    kD2257_IO_IRQ_MASK_D      = 0x09cb,
};

// ---------------------------------    
//	Charger / Charging Control
// ---------------------------------    

enum {
	kD2257_CHG_CONTROL	= 0xa00,
	kD2257_SYS_CONF_B	= 0xa01,
	kD2257_SYS_CONF_D	= 0xa02,

	kD2257_ISET_BUCK		= 0x0a03,
	kD2257_ISET_BUCK_SHADOW	= 0x0a04,
	kD2257_ISET_BUCK_ABSMAX	= 0x0a05,
	kD2257_ISET_BUCK_ACTUAL	= 0x0a06,
	kD2257_ISET_BUCK_ALT	= 0x0a07,
        kD2257_ICHG_BAT         = 0x0a08,

	kD2257_CHG_CTRL_A		= 0x0a0a,
	kD2257_CHG_CTRL_B		= 0x0a0b,
	kD2257_CHG_CTRL_C		= 0x0a0c,
	kD2257_CHG_CTRL_D		= 0x0a0d,
	kD2257_CHG_CTRL_E		= 0x0a0e,
	kD2257_CHG_CTRL_F		= 0x0a0f,
	kD2257_CHG_CTRL_G		= 0x0a10,
	kD2257_CHG_CTRL_H		= 0x0a11,
	kD2257_CHG_TIME			= 0x0a12,
	kD2257_CHG_TIME_PRE		= 0x0a13,
	kD2257_CHG_STAT			= 0x0a14,
	
//	kD2207_CHG_VSET_TRIM	= 0x04d0,
//	kD2207_IBUS_GAIN_TRIM	= 0x04d2,
//	kD2207_IBUS100_OFS_TRIM	= 0x04d3,
//	kD2207_IBUS500_OFS_TRIM	= 0x04d4,
//	kD2207_IBUS1500_GAIN_TRIM= 0x04d5,
//	kD2207_IBUS1500_OFS_TRIM= 0x04d6,

	kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND	= (1 << 1),
	kDIALOG_SYS_CONTROL_BUS_PWR_SUSPEND	= (1 << 0),
	kDIALOG_SYS_CONTROL_CHRG_CONTROLS	= kDIALOG_SYS_CONTROL_BAT_PWR_SUSPEND
						| kDIALOG_SYS_CONTROL_BUS_PWR_SUSPEND,

	kDIALOG_CHARGE_BUCK_CONTROL	= kD2257_ISET_BUCK,
	kDIALOG_CHARGE_BUCK_STATUS	= kD2257_ISET_BUCK_ACTUAL,
	kDIALOG_CHARGE_STATUS		= kD2257_CHG_STAT,

	kDIALOG_CHARGE_CONTROL_ICHG_BAT = kD2257_ICHG_BAT,
	kDIALOG_CHARGE_CONTROL_HIB	= kD2257_CHG_CTRL_B,
	kDIALOG_CHARGE_CONTROL_TIME	= kD2257_CHG_CTRL_D,
	kDIALOG_CHARGE_CONTROL_EN	= kD2257_CHG_CTRL_G,
	kDIALOG_OTP_ISET_BAT		= kD2257_CHG_CTRL_C,
};

// ---------------------------------
// IPEAK
// ---------------------------------

enum {
    kD2257_IPK_UV_BUCK1_IPEAK_ANA_CONF_2    = 0x0f87,
    kD2257_IPK_UV_BUCK1_UV_DIG_CONF_1       = 0x0f8a,
    kD2257_IPK_UV_BUCK1_UV_EN_THR           = 0x0f8c,
    kD2257_IPK_UV_BUCK1_UV_ANA_CONF_1       = 0x0f8d,
    kD2257_IPK_UV_BUCK1_UV_OFFSET           = 0x0f8e,
};

// ---------------------------------
// Buck control
// ---------------------------------

enum {
    kD2257_BUCK0_FAST_VSEL      = 0x1000,
    kD2257_BUCK0_VSEL           = 0x1001,
    kD2257_BUCK0_FAST_VSEL_EN	= 0x1002,    
    kD2257_BUCK0_VSEL_ACTUAL	= 0x1005,
    kD2257_BUCK0_VSEL_LOCK		= 0x1006,
    kD2257_BUCK0_MINV,
    kD2257_BUCK0_MAXV,
    kD2257_BUCK0_OFS_V,
    kD2257_BUCK0_MODE           = 0x100a,    
    kD2257_BUCK0_FSM_TRIM6      = 0x1067,
    kD2257_BUCK0_FSM_TRIM7      = 0x1068,

    kD2257_BUCK1_FAST_VSEL      = 0x1100,    
    kD2257_BUCK1_VSEL           = 0x1101,    
    kD2257_BUCK1_FSM_TRIM6      = 0x1167,
    kD2257_BUCK1_FSM_TRIM7      = 0x1168,

    kD2257_BUCK2_FAST_VSEL      = 0x1200,
    kD2257_BUCK2_VSEL           = 0x1201,
    kD2257_BUCK2_FSM_TRIM6      = 0x1267,
    kD2257_BUCK2_FSM_TRIM7      = 0x1268,
    
    kD2257_BUCK3_FAST_VSEL      = 0x1300,
    kD2257_BUCK3_VSEL           = 0x1301,
    kD2257_BUCK3_FSM_TRIM6      = 0x1367,
    kD2257_BUCK3_FSM_TRIM7      = 0x1368,
    
    kD2257_BUCK4_FAST_VSEL      = 0x1400,
    kD2257_BUCK4_VSEL           = 0x1401,
    kD2257_BUCK4_FSM_TRIM6      = 0x1467,
    kD2257_BUCK4_FSM_TRIM7      = 0x1468,
    
    kD2257_BUCK5_FAST_VSEL      = 0x1500,
    kD2257_BUCK5_VSEL           = 0x1501,
    kD2257_BUCK5_FSM_TRIM6      = 0x1567,
    kD2257_BUCK5_FSM_TRIM7      = 0x1568,
    
    kD2257_BUCK6_FAST_VSEL      = 0x1600,
    kD2257_BUCK6_VSEL           = 0x1601,
    kD2257_BUCK6_FSM_TRIM6      = 0x1667,
    kD2257_BUCK6_FSM_TRIM7      = 0x1668,    
    
    kD2257_BUCK7_FAST_VSEL      = 0x1700,
    kD2257_BUCK7_VSEL           = 0x1701,
    kD2257_BUCK7_FSM_TRIM6      = 0x1767,
    kD2257_BUCK7_FSM_TRIM7      = 0x1768,
    
    kD2257_BUCK8_FAST_VSEL      = 0x1800,
    kD2257_BUCK8_VSEL           = 0x1801,
    kD2257_BUCK8_FSM_TRIM6      = 0x1867,
    kD2257_BUCK8_FSM_TRIM7      = 0x1868,

    // LDO control...
    kD2257_LDO1_VSEL            = 0x2100,
    kD2257_LDO1_VSEL_ACTUAL     = 0x2101,
    kD2257_LDO1_MINV            = 0x2160,
    kD2257_LDO1_MAXV            = 0x2161,
    kD2257_LDO1_UOV_LIM_EN_VOUT_PD= 0x2163,

    kD2257_LDO2_VSEL            = 0x2200,
    kD2257_LDO2_VSEL_ACTUAL     = 0x2201,    
    kD2257_LDO2_BYPASS          = 0x2202,
    kD2257_LDO2_MINV            = 0x2260,
    kD2257_LDO2_MAXV            = 0x2261,

    kD2257_LDO3_VSEL            = 0x2300,
    kD2257_LDO3_VSEL_ACTUAL     = 0x2301,    
    kD2257_LDO3_MINV            = 0x2360,
    kD2257_LDO3_MAXV            = 0x2361,

    kD2257_LDO4_VSEL            = 0x2400,
    kD2257_LDO4_VSEL_ACTUAL     = 0x2401,    
    kD2257_LDO4_MINV            = 0x2460,
    kD2257_LDO4_MAXV            = 0x2461,

    kD2257_LDO5_VSEL            = 0x2500,
    kD2257_LDO5_VSEL_ACTUAL     = 0x2501,    
    kD2257_LDO5_MINV            = 0x2560,
    kD2257_LDO5_MAXV            = 0x2561,

    kD2257_LDO6_VSEL            = 0x2600,
    kD2257_LDO6_VSEL_ACTUAL     = 0x2601,    
    kD2257_LDO6_BYPASS          = 0x2602,
    kD2257_LDO6_MINV            = 0x2660,
    kD2257_LDO6_MAXV            = 0x2661,

    kD2257_LDO7_VSEL            = 0x2700,
    kD2257_LDO7_VSEL_ACTUAL     = 0x2701,    
    kD2257_LDO7_MINV            = 0x2760,
    kD2257_LDO7_MAXV            = 0x2761,
    kD2257_LDO7_SOFT_STARTUP    = 0x2764,

    kD2257_LDO8_VSEL            = 0x2800,
    kD2257_LDO8_VSEL_ACTUAL     = 0x2801,    
    kD2257_LDO8_MINV            = 0x2860,
    kD2257_LDO8_MAXV            = 0x2861,

    kD2257_LDO9_VSEL            = 0x2900,
    kD2257_LDO9_VSEL_ACTUAL     = 0x2901,    
    kD2257_LDO9_MINV            = 0x2960,
    kD2257_LDO9_MAXV            = 0x2961,
    kD2257_LDO9_TRIM            = 0x2962,

    kD2257_LDO10_VSEL           = 0x2A00,
    kD2257_LDO10_VSEL_ACTUAL    = 0x2A01,    
    kD2257_LDO10_MINV           = 0x2A60,
    kD2257_LDO10_MAXV           = 0x2A61,
    kD2257_LDO10_TRIM           = 0x2A62,

    kD2257_LDO11_VSEL           = 0x2B00,
    kD2257_LDO11_VSEL_ACTUAL    = 0x2B01,    
    kD2257_LDO11_MINV           = 0x2B60,
    kD2257_LDO11_MAXV           = 0x2B61,
    kD2257_LDO11_TRIM           = 0x2B62,

    kD2257_LDO12_TRIM           = 0x2C62,

    kD2257_LDO13_VSEL           = 0x2D00,
    kD2257_LDO13_VSEL_ACTUAL    = 0x2D01,    
    kD2257_LDO13_MINV           = 0x2D60,
    kD2257_LDO13_MAXV           = 0x2D61,
    kD2257_LDO13_TRIM           = 0x2D62,

    kD2257_LDO14_VSEL           = 0x2E00,
    kD2257_LDO14_VSEL_ACTUAL    = 0x2E01,    
    kD2257_LDO14_MINV           = 0x2E60,
    kD2257_LDO14_MAXV           = 0x2E61,
    kD2257_LDO14_TRIM           = 0x2E62,

    kD2257_LDO15_VSEL           = 0x2F00,
    kD2257_LDO15_VSEL_ACTUAL    = 0x2F01,    
    kD2257_LDO15_BYPASS         = 0x2F02,
    kD2257_LDO15_MINV           = 0x2F60,
    kD2257_LDO15_MAXV           = 0x2F61,
    kD2257_LDO15_TRIM           = 0x2F62,

    kD2257_LDO16_VSEL           = 0x3000,
    kD2257_LDO16_VSEL_ACTUAL    = 0x3001,    
    kD2257_LDO16_MINV           = 0x3060,
    kD2257_LDO16_MAXV           = 0x3061,
    kD2257_LDO16_TRIM           = 0x3062,

    kD2257_LDO_LCM_LDO1_VSEL            = 0x3200,
    kD2257_LDO_LCM_LDO1_VSEL_ACTUAL     = 0x3201,

    kD2257_LDO_LCM_LDO2_VSEL            = 0x3300,
    kD2257_LDO_LCM_LDO2_VSEL_ACTUAL     = 0x3301,

    kD2257_LDO_LCM_BOOST_VSEL           = 0x3700,
    kD2257_LDO_LCM_BOOST_VSEL_ACTUAL    = 0x3701,
    kD2257_LDO_LCM_BOOST_VSEL_MINV      = 0x3760,
    kD2257_LDO_LCM_BOOST_VSEL_MAXV      = 0x3761,
    kD2257_LDO_LCM_BOOST_CONF           = 0x3762,
    
    kD2257_LDO_EVENT            = 0x3FC0,
    kD2257_LDO_STATUS           = 0x3FC1,
    kD2257_LDO_IRQ_MASK         = 0x3FC2,

    kDIALOG_BYPASS_EN           = 1,
    kDIALOG_BYPASS_DIS          = 0,
};

// ---------------------------------
// GPADC control
// ---------------------------------    

enum {
    kD2257_GPADC_MAN_CTRL1      = 0x4000,   // Channel 0..253 (0xfd)
    kD2257_GPADC_MAN_CTRL1_MASK = 0xff,
    kD2257_GPADC_MAN1_RES_LSB   = 0x4001,
    kD2257_GPADC_MAN1_RES_MSB   = 0x4002,

    kD2257_GPADC_MAN_CTRL2      = 0x4004,   // Channel 0..127 (0x7f)
    kD2257_GPADC_MAN_CTRL2_MASK = 0x7F,
    kD2257_GPADC_MAN2_RES_LSB   = 0x4005,
    kD2257_GPADC_MAN2_RES_MSB   = 0x4006,
    
    kD2257_GPADC_ICHG_AVG       = 0x40f8,
    // mapped on (kADC_AVG|kD2257_ADC_CONTROL_MUX_SEL_ICH_ADC_CH) known as kDIALOG_ICHG_AVG
    
    kD2257_GPADC_TDIE1_RISE     = 0x413c,
    // ..
    kD2257_GPADC_TDIE9_RISE     = 0x4147,

    kDIALOG_T_OFFSET_MSB	= 0x4148,
    kDIALOG_T_OFFSET_LSB	= 0x4149, // [0-3]

    kD2257_GPADC_EVENT0        = 0x4180,
    kD2257_GPADC_EVENT1        = 0x4181,
    kD2257_GPADC_STATUS0       = 0x4187,
    kD2257_GPADC_STATUS1       = 0x4188,
};

enum {
    kDIALOG_APP_TMUX            = kD2257_APP_TMUX,
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
    //	Legacy Scratch Pads
    kDIALOG_MEMBYTE0		= 0x5000,
    kDIALOG_MEMBYTE_LAST	= 0x5027,
    
    kDIALOG_TEST_ACCESS         = 0x7000,
    kDIALOG_TEST_ACCESS_ENA = 0x1D,
    kDIALOG_TEST_ACCESS_DIS = 0x00,
    
    //	Scratch Pad RAM
    kDIALOG_RAM0			= 0x8000,
    kDIALOG_RAM_LAST        = 0x87ff,
    
    kDIALOG_EXT_MEM_CAL0_SIZE  = 64,
    kDIALOG_GAP_SCRATCH8_SIZE  = 8, // ex diags, free now
    kDIALOG_EXT_MEM_CAL1_SIZE  = 336,
    kDIALOG_VOLTAGE_KNOBS_SIZE = 32,
    kDIALOG_EXT_MEM_CAL2_SIZE  = 400, // <rdar://problem/14960891>
    // ...
    kDIALOG_DIAG_SCRATCH_SIZE  = 32,
    
    kDIALOG_EXT_MEM_CAL0  = kDIALOG_RAM0,
    kDIALOG_EXT_MEM_CAL1  = kDIALOG_EXT_MEM_CAL0+kDIALOG_EXT_MEM_CAL0_SIZE+kDIALOG_GAP_SCRATCH8_SIZE,
    kDIALOG_VOLTAGE_KNOBS = kDIALOG_EXT_MEM_CAL1+kDIALOG_EXT_MEM_CAL1_SIZE,
    kDIALOG_EXT_MEM_CAL2  = kDIALOG_VOLTAGE_KNOBS+kDIALOG_VOLTAGE_KNOBS_SIZE,
    // reserved, future
    kDIALOG_DIAG_SCRATCH  = 0x87e0,
    
    // handy
    kDIALOG_EXT_MEM_CAL_SIZE = kDIALOG_EXT_MEM_CAL0_SIZE+kDIALOG_EXT_MEM_CAL1_SIZE+kDIALOG_EXT_MEM_CAL2_SIZE,
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
// Event Registers

    // 0: SYSCTL_EVENT 
    kD2257_EVENT_HIGH_TEMP_WARNING  = (1 << 0),
    // 1: MFSM_EVENT
    // 2: RTC_EVENT
    kD2257_EVENT_ALARM              = (1 << 0),
    // 3: SPI_EVENT
    // 4: CLKGEN_EVENT
    // 5: IO_EVENT_A
    kD2257_EVENT_A_BUTTON4_DBL      = (1 << 7),
    kD2257_EVENT_A_BUTTON3_DBL      = (1 << 6),
    kD2257_EVENT_A_BUTTON2_DBL      = (1 << 5),
    kD2257_EVENT_A_BUTTON1_DBL      = (1 << 4),
    kD2257_EVENT_A_BUTTON4          = (1 << 3),
    kD2257_EVENT_A_BUTTON3          = (1 << 2),
    kD2257_EVENT_A_BUTTON2          = (1 << 1),
    kD2257_EVENT_A_BUTTON1          = (1 << 0),
    // 6: IO_EVENT_B
    kD2257_EVENT_B_GPIO8            = (1 << 7),
    kD2257_EVENT_B_GPIO7            = (1 << 6),
    kD2257_EVENT_B_GPIO6            = (1 << 5),
    kD2257_EVENT_B_GPIO5            = (1 << 4),
    kD2257_EVENT_B_GPIO4            = (1 << 3),
    kD2257_EVENT_B_GPIO3            = (1 << 2),
    kD2257_EVENT_B_GPIO2            = (1 << 1),
    kD2257_EVENT_B_GPIO1            = (1 << 0),
    // 7: IO_EVENT_C
    kD2257_EVENT_C_GPIO16           = (1 << 7),
    kD2257_EVENT_C_GPIO15           = (1 << 6),
    kD2257_EVENT_C_GPIO14           = (1 << 5),
    kD2257_EVENT_C_GPIO13           = (1 << 4),
    kD2257_EVENT_C_GPIO12           = (1 << 3),
    kD2257_EVENT_C_GPIO11           = (1 << 2),
    kD2257_EVENT_C_GPIO10           = (1 << 1),
    kD2257_EVENT_C_GPIO9            = (1 << 0),
    // 8: IO_EVENT_D
    kD2257_EVENT_D_GPIO22           = (1 << 5),
    kD2257_EVENT_D_GPIO21           = (1 << 4),
    kD2257_EVENT_D_GPIO20           = (1 << 3),
    kD2257_EVENT_D_GPIO19           = (1 << 2),
    kD2257_EVENT_D_GPIO18           = (1 << 1),
    kD2257_EVENT_D_GPIO17           = (1 << 0),
    // 9: CHG_EVENT_T1
    kD2257_EVENT_CHG1_VBUS_OV       = (1 << 6),
    kD2257_EVENT_CHG1_VBUS_OC       = (1 << 5),
    kD2257_EVENT_CHG1_VBUS_EXT_REM  = (1 << 4),
    kD2257_EVENT_CHG1_CHG_HV_ATT    = (1 << 3),
    kD2257_EVENT_CHG1_VBUS_EXT_DET  = (1 << 1),
    kD2257_EVENT_CHG1_VCENTER_DET   = (1 << 0),
    // 0xa: CHG_EVENT_T2
    kD2257_EVENT_CHG2_BUCK_FLG      =  (1<<6),
    kD2257_EVENT_CHG2_ABCC_ACT      =  (1<<5),
    kD2257_EVENT_CHG2_TIMEOUT       =  (1<<4),
    kD2257_EVENT_CHG2_PRECHG_TIMEOUT=  (1<<3),
    kD2257_EVENT_CHG2_CHG_END       =  (1<<2),
    kD2257_EVENT_CHG2_CHG_FAST      =  (1<<1),
    kD2257_EVENT_CHG2_CHG_PRE       =  (1<<0),
    // 0x17, GPADC_EVENT0
    kD2257_EVENT_EOMC2              =  (1<<1),
    kD2257_EVENT_EOMC1              =  (1<<0),
    // 0x18: GPADC_EVENT1
    kD2257_EVENT_VDDLOW             =  (1<<7),
    kD2257_EVENT_TBAT               =  (1<<6),
};

enum {
    kDIALOG_EVENT_HOLD_BUTTON_MASK  = EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON2),
    kDIALOG_EVENT_MENU_BUTTON_MASK  = EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON1),
    kDIALOG_EVENT_RINGER_BUTTON_MASK= EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON3),
    kDIALOG_EVENT_BUTTON4_MASK      = EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON4),
    
    kDIALOG_EVENT_BUTTONS       = (kD2257_EVENT_A_BUTTON4 |
                       kD2257_EVENT_A_BUTTON3 |
                       kD2257_EVENT_A_BUTTON2 |
                       kD2257_EVENT_A_BUTTON1),
    kDIALOG_EVENT_BUTTONS_MASK  = EVENT_FLAG_MAKE(5, kDIALOG_EVENT_BUTTONS),
    kDIALOG_EVENT_PWR_BUTTON_MASK   = kDIALOG_EVENT_HOLD_BUTTON_MASK,
    
	kDIALOG_EVENT_ALARM_MASK        = EVENT_FLAG_MAKE(2, kD2257_EVENT_ALARM),

    kDIALOG_EVENT_ACC_DET_MASK      = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_EVENT_VBUS_DET_MASK     = EVENT_FLAG_MAKE(9, kD2257_EVENT_CHG1_VBUS_EXT_DET),
    kDIALOG_EVENT_VBUS_EXT_REM_MASK = EVENT_FLAG_MAKE(9, kD2257_EVENT_CHG1_VBUS_EXT_REM),

    // FIXME: forcing ADC1
    kDIALOG_EVENT_EOMC_MASK         = EVENT_FLAG_MAKE(0x17, kD2257_EVENT_EOMC1),
    kDIALOG_EVENT_HIB_MASK          = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_EVENT_ABCC_MASK         = EVENT_FLAG_MAKE(0xa, kD2257_EVENT_CHG2_ABCC_ACT),
    kDIALOG_EVENT_CHG_END_MASK      = EVENT_FLAG_MAKE(0xa, kD2257_EVENT_CHG2_CHG_END),
    kDIALOG_EVENT_TBAT_MASK         = EVENT_FLAG_MAKE(0x18, kD2257_EVENT_TBAT),
    
    kDIALOG_EVENT_GPIO1_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO1),
    kDIALOG_EVENT_GPIO2_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO2),
    kDIALOG_EVENT_GPIO3_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO3),
    kDIALOG_EVENT_GPIO4_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO4),
    kDIALOG_EVENT_GPIO5_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO5),
    kDIALOG_EVENT_GPIO6_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO6),
    kDIALOG_EVENT_GPIO7_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO7),
    kDIALOG_EVENT_GPIO8_MASK    = EVENT_FLAG_MAKE(6, kD2257_EVENT_B_GPIO8),
    
    kDIALOG_EVENT_GPIO9_MASK    = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO9),
    kDIALOG_EVENT_GPIO10_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO10),
    kDIALOG_EVENT_GPIO11_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO11),
    kDIALOG_EVENT_GPIO12_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO12),
    kDIALOG_EVENT_GPIO13_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO13),
    kDIALOG_EVENT_GPIO14_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO14),
    kDIALOG_EVENT_GPIO15_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO15),
    kDIALOG_EVENT_GPIO16_MASK   = EVENT_FLAG_MAKE(7, kD2257_EVENT_C_GPIO16),
    
    kDIALOG_EVENT_GPIO17_MASK   = EVENT_FLAG_MAKE(8, kD2257_EVENT_D_GPIO17),
    kDIALOG_EVENT_GPIO18_MASK   = EVENT_FLAG_MAKE(8, kD2257_EVENT_D_GPIO18),
    kDIALOG_EVENT_GPIO19_MASK   = EVENT_FLAG_MAKE(8, kD2257_EVENT_D_GPIO19),
    kDIALOG_EVENT_GPIO20_MASK   = EVENT_FLAG_MAKE(8, kD2257_EVENT_D_GPIO20),
    kDIALOG_EVENT_GPIO21_MASK   = EVENT_FLAG_MAKE(8, kD2257_EVENT_D_GPIO21),
    kDIALOG_EVENT_GPIO22_MASK   = EVENT_FLAG_MAKE(8, kD2257_EVENT_D_GPIO22),

    kDIALOG_EVENT_RINGER_DBL_MASK   = EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON3_DBL),
    kDIALOG_EVENT_HOLD_DBL_MASK     = EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON2_DBL),
    kDIALOG_EVENT_MENU_DBL_MASK     = EVENT_FLAG_MAKE(5, kD2257_EVENT_A_BUTTON1_DBL),

    kDIALOG_EVENT_VHP_DET_MASK  = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_EVENT_ON_MASK       = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_EVENT_LDO2_EN_MASK  = kDIALOG_NOTEXIST_MASK, // FIXME
};

enum {
    kDIALOG_STATUS_USB_MASK         = STATUS_FLAG_MAKE(9, kD2257_EVENT_CHG1_VCENTER_DET),
    kDIALOG_STATUS_VBUS_MASK        = STATUS_FLAG_MAKE(9, kD2257_EVENT_CHG1_VBUS_EXT_DET),
    kDIALOG_STATUS_FW_MASK          = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_STATUS_ACC_DET_MASK     = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_STATUS_CHARGING_MASK    = kDIALOG_NOTEXIST_MASK, // FIXME
    kDIALOG_STATUS_CHG_TO_MASK      = STATUS_FLAG_MAKE(0xa, kD2257_EVENT_CHG2_TIMEOUT),
    kDIALOG_STATUS_CHG_END_MASK     = STATUS_FLAG_MAKE(0xa, kD2257_EVENT_CHG2_CHG_END),
    kDIALOG_STATUS_TBAT_MASK        = STATUS_FLAG_MAKE(0x18, kD2257_EVENT_TBAT),
    kDIALOG_STATUS_CHG_ATT_MASK     = kDIALOG_NOTEXIST_MASK,
    kDIALOG_STATUS_ABCC_MASK        = STATUS_FLAG_MAKE(0xa, kD2257_EVENT_CHG2_ABCC_ACT),
};

static const statusRegisters kDialogStatusFWMask = { };

enum {
    kD2257_EVENT_AA_WAKEMASK = 0,
    kD2257_EVENT_AB_WAKEMASK = 0,
    kD2257_EVENT_AC_WAKEMASK = kD2257_EVENT_ALARM,
    kD2257_EVENT_AD_WAKEMASK = 0,
    kD2257_EVENT_AE_WAKEMASK = 0,
    kD2257_EVENT_AF_WAKEMASK = kDIALOG_EVENT_BUTTONS,
    kD2257_EVENT_AG_WAKEMASK = 0,
    kD2257_EVENT_AH_WAKEMASK = 0,
    kD2257_EVENT_AI_WAKEMASK = 0,
    kD2257_EVENT_AJ_WAKEMASK = kD2257_EVENT_CHG1_VBUS_EXT_DET | kD2257_EVENT_CHG1_VCENTER_DET, // FIXME (kD2186_EVENT_A_ACC_DET)
    kD2257_EVENT_AK_WAKEMASK = kD2257_EVENT_CHG2_ABCC_ACT,
    kD2257_EVENT_AL_WAKEMASK = 0,
    kD2257_EVENT_AM_WAKEMASK = 0,
    kD2257_EVENT_AN_WAKEMASK = 0,
    kD2257_EVENT_AO_WAKEMASK = 0,
    kD2257_EVENT_AP_WAKEMASK = 0,
    kD2257_EVENT_AQ_WAKEMASK = 0,
    kD2257_EVENT_AR_WAKEMASK = 0,
    kD2257_EVENT_AS_WAKEMASK = 0,
    kD2257_EVENT_AT_WAKEMASK = 0,
    kD2257_EVENT_AU_WAKEMASK = 0,
    kD2257_EVENT_AV_WAKEMASK = 0,
    kD2257_EVENT_AW_WAKEMASK = 0,
    kD2257_EVENT_AX_WAKEMASK = 0,
    kD2257_EVENT_AY_WAKEMASK = 0,
    kD2257_EVENT_AZ_WAKEMASK = 0,
    kD2257_EVENT_BA_WAKEMASK = 0,
    kD2257_EVENT_BB_WAKEMASK = 0,
    kD2257_EVENT_BC_WAKEMASK = 0,
    kD2257_EVENT_BD_WAKEMASK = 0,
    kD2257_EVENT_BE_WAKEMASK = 0,
};

// All events that are masked during shutdown - inverse of the wake mask,
// events that wake up the system
static const eventRegisters kDialogEventIntMasks = {
    ~kD2257_EVENT_AA_WAKEMASK,
    ~kD2257_EVENT_AB_WAKEMASK,
    ~kD2257_EVENT_AC_WAKEMASK,
    ~kD2257_EVENT_AD_WAKEMASK,
    ~kD2257_EVENT_AE_WAKEMASK,
    ~kD2257_EVENT_AF_WAKEMASK,
    ~kD2257_EVENT_AG_WAKEMASK,
    ~kD2257_EVENT_AH_WAKEMASK,
    ~kD2257_EVENT_AI_WAKEMASK,
    ~kD2257_EVENT_AJ_WAKEMASK,
    ~kD2257_EVENT_AK_WAKEMASK,
    ~kD2257_EVENT_AL_WAKEMASK,
    ~kD2257_EVENT_AM_WAKEMASK,
    ~kD2257_EVENT_AN_WAKEMASK,
    ~kD2257_EVENT_AO_WAKEMASK,
    ~kD2257_EVENT_AP_WAKEMASK,
    ~kD2257_EVENT_AQ_WAKEMASK,
    ~kD2257_EVENT_AR_WAKEMASK,
    ~kD2257_EVENT_AS_WAKEMASK,
    ~kD2257_EVENT_AT_WAKEMASK,
    ~kD2257_EVENT_AU_WAKEMASK,
    ~kD2257_EVENT_AV_WAKEMASK,
    ~kD2257_EVENT_AW_WAKEMASK,
    ~kD2257_EVENT_AX_WAKEMASK,
    ~kD2257_EVENT_AY_WAKEMASK,
    ~kD2257_EVENT_AZ_WAKEMASK,
    ~kD2257_EVENT_BA_WAKEMASK,
    ~kD2257_EVENT_BB_WAKEMASK,
    ~kD2257_EVENT_BC_WAKEMASK,
    ~kD2257_EVENT_BD_WAKEMASK,
    ~kD2257_EVENT_BE_WAKEMASK,
};

// All wake events without the buttons
static const eventRegisters kDialogEventNotButtonMasks = {
    kD2257_EVENT_AA_WAKEMASK,
    kD2257_EVENT_AB_WAKEMASK,
    kD2257_EVENT_AC_WAKEMASK,
    kD2257_EVENT_AD_WAKEMASK,
    kD2257_EVENT_AE_WAKEMASK,
    kD2257_EVENT_AF_WAKEMASK & ~kDIALOG_EVENT_BUTTONS ,
    kD2257_EVENT_AG_WAKEMASK,
    kD2257_EVENT_AH_WAKEMASK,
    kD2257_EVENT_AI_WAKEMASK,
    kD2257_EVENT_AJ_WAKEMASK,
    kD2257_EVENT_AK_WAKEMASK,
    kD2257_EVENT_AL_WAKEMASK,
    kD2257_EVENT_AM_WAKEMASK,
    kD2257_EVENT_AN_WAKEMASK,
    kD2257_EVENT_AO_WAKEMASK,
    kD2257_EVENT_AP_WAKEMASK,
    kD2257_EVENT_AQ_WAKEMASK,
    kD2257_EVENT_AR_WAKEMASK,
    kD2257_EVENT_AS_WAKEMASK,
    kD2257_EVENT_AT_WAKEMASK,
    kD2257_EVENT_AU_WAKEMASK,
    kD2257_EVENT_AV_WAKEMASK,
    kD2257_EVENT_AW_WAKEMASK,
    kD2257_EVENT_AX_WAKEMASK,
    kD2257_EVENT_AY_WAKEMASK,
    kD2257_EVENT_AZ_WAKEMASK,
    kD2257_EVENT_BA_WAKEMASK,
    kD2257_EVENT_BB_WAKEMASK,
    kD2257_EVENT_BC_WAKEMASK,
    kD2257_EVENT_BD_WAKEMASK,
    kD2257_EVENT_BE_WAKEMASK,
};

// All events indicating external power supply
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
    kD2257_EVENT_CHG1_VCENTER_DET | kD2257_EVENT_CHG1_VBUS_EXT_DET,
    kD2257_EVENT_CHG2_ABCC_ACT,
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

// should I use *_VBUS_DET_REM
static const eventRegisters kDialogEventUSBMask = {
    0,  // EVENT_AA
    0,  // EVENT_AB
    0,  // EVENT_AC
    0,  // EVENT_AD
    0,  // EVENT_AE
    0,  // EVENT_AF
    0,  // EVENT_AG
    0,  // EVENT_AH
    0,  // EVENT_AI
    kD2257_EVENT_CHG1_VCENTER_DET | kD2257_EVENT_CHG1_VBUS_EXT_DET,
    kD2257_EVENT_CHG2_ABCC_ACT,
    0,  // EVENT_AL
    0,  // EVENT_AM
    0,  // EVENT_AN
    0,  // EVENT_AO
    0,  // EVENT_AP
    0,  // EVENT_AQ
    0,  // EVENT_AR
    0,  // EVENT_AS
    0,  // EVENT_AT
    0,  // EVENT_AU
    0,  // EVENT_AV
    0,  // EVENT_AW
    0,  // EVENT_AX
    0,  // EVENT_AY
    0,  // EVENT_AZ
    0,  // EVENT_BA
    0,  // EVENT_BB
    0,  // EVENT_BC
    0,  // EVENT_BD
    0,  // EVENT_BE
};

static const eventRegisters kDialogEventFWMask = { 0, };
static const statusRegisters kDialogStatusChargingMask = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    kD2257_EVENT_CHG2_CHG_FAST | kD2257_EVENT_CHG2_CHG_PRE,    
};



enum {
    kDialogEventPwrsupplyCount = 2,
    kDialogEventUSBCount = 2,
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
    kD2257_ADC_CONTROL_MUX_SEL_VDD_MAIN     = 0,
    kD2257_ADC_CONTROL_MUX_SEL_BRICK_ID     = 1,
    kD2257_ADC_CONTROL_MUX_SEL_SPARE0       = 2,
    kD2257_ADC_CONTROL_MUX_SEL_APP_MUX_A    = 3,
    kD2257_ADC_CONTROL_MUX_SEL_APP_MUX_B    = 4,
    // ...
    kD2257_ADC_CONTROL_MUX_SEL_VLDO1    = 18,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO1    = 19,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO2,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO2,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO3,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO3,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO4,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO4,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO5,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO5,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO6,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO6,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO7,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO7,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO8,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO8,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO9,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO9,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO10,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO10,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO11,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO11,
    kD2257_ADC_CONTROL_MUX_SEL_VBUF_ON,
    kD2257_ADC_CONTROL_MUX_SEL_ION_BUF,
    // no papi, no12
    kD2257_ADC_CONTROL_MUX_SEL_VLDO13   = 42,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO13	= 43,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO14	= 44,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO14	= 45,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO15   = 46,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO15   = 47,
    kD2257_ADC_CONTROL_MUX_SEL_VLDO16   = 48,
    kD2257_ADC_CONTROL_MUX_SEL_ILDO16   = 49,
    // ReatlimeClock
    kD2257_ADC_CONTROL_MUX_SEL_VRTC = 50,
    kD2257_ADC_CONTROL_MUX_SEL_IRTC = 51,

    // high impedance external input
    kD2257_ADC_CONTROL_MUX_SEL_ADC_IN7   = 82,
    // Junction
    kD2257_ADC_CONTROL_MUX_SEL_TJNCT = 88,
    // NTCs
    kD2257_ADC_CONTROL_MUX_SEL_TCAL =  89, // NTC0
    kD2257_ADC_CONTROL_MUX_SEL_TDEV1 = 90,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV2 = 91,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV3 = 92,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV4 = 93,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV5,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV6,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV7,
    kD2257_ADC_CONTROL_MUX_SEL_TDEV8,
    // ..

    kD2257_ADC_CONTROL_MUX_SEL_TBAT  = 108,

    // TINT, measure TDIE (TDIE1,TDIE9)
    kD2257_ADC_CONTROL_MUX_SEL_LDO10_TEMP   = 112,
    kD2257_ADC_CONTROL_MUX_SEL_LDO5CHGBUCKA_TEMP   = 113,
    kD2257_ADC_CONTROL_MUX_SEL_CHGBUCKB_TEMP   = 114,
    kD2257_ADC_CONTROL_MUX_SEL_BUCK6_TEMP      = 115,
    kD2257_ADC_CONTROL_MUX_SEL_BUCK0_TEMP      = 116,
    kD2257_ADC_CONTROL_MUX_SEL_BUCK28_TEMP,
    kD2257_ADC_CONTROL_MUX_SEL_BUCK74_TEMP,
    kD2257_ADC_CONTROL_MUX_SEL_BUCK1_TEMP,
    kD2257_ADC_CONTROL_MUX_SEL_BUCK35_TEMP,

    kD2257_ADC_CONTROL_MUX_SEL_VBUCK0       = 144,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK1,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK2,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK3,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK4,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK5,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK6,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK7,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK8       = 152,

    kD2257_ADC_CONTROL_MUX_SEL_VBAT  = 157,
    kD2257_ADC_CONTROL_MUX_SEL_VBUS  = 158,
    kD2257_ADC_CONTROL_MUX_SEL_ADC_IN31  = 159,
    
    // Legacy GPADC based iBuck meas
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK0       = 176,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK1,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK2,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK3,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK4,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK5       = 181,
    // no iBuck6
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK7       = 183,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK8       = 184,

    // SAR based IPBuck meas
    // - see BUCKn_ANA_TRIM4 to select power (default) or current measurement
    // - see IBUCK_CONFIG_C[IBUCKn_SAR_IMEAS_TYPE] for digital offset cancellation
    kD2257_ADC_CONTROL_MUX_SEL_SAR_IBUCK0       = 185,
    // SAR Filtered based IPBuck meas
    // - see BUCKn_ANA_TRIM4 to select power (default) or current measurement
    // - see IBUCK_CONFIG_C[IBUCKn_SAR_IMEAS_TYPE] for digital offset cancellation
    // - use IBUCKn_SAR_FILTER_TC to select the filter constant
    kD2257_ADC_CONTROL_MUX_SEL_SAR_FILT_IBUCK0  = 186,
    kD2257_ADC_CONTROL_MUX_SEL_SAR_IBUCK1,
    kD2257_ADC_CONTROL_MUX_SEL_SAR_FILT_IBUCK1,
    kD2257_ADC_CONTROL_MUX_SEL_SAR_IBUCK2,
    kD2257_ADC_CONTROL_MUX_SEL_SAR_FILT_IBUCK2,
    
    kD2257_ADC_CONTROL_MUX_SEL_ICH_ADC_CH           = 201,
    kD2257_ADC_CONTROL_MUX_SEL_IBUS_IN_LG_ADC_CH    = 202,
    kD2257_ADC_CONTROL_MUX_SEL_IBUS_IN_HG_ADC_CH    = 203,

    // cannot use this when IBUCK_CORR_N is set (default)
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK0_OFF   = 208, // offset
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK1_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK2_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK3_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK4_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK5_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK6_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK7_OFF,
    kD2257_ADC_CONTROL_MUX_SEL_IBUCK8_OFF   = 216,

    kD2257_ADC_CONTROL_MUX_SEL_VBUCK3_SW1   = 240,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK3_SW2,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK3_SW3,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK4_SW1   = 243,
    kD2257_ADC_CONTROL_MUX_SEL_VBUCK4_SW2   = 244,

    // ------------------------------------------------
    
    kDIALOG_ADC_CONTROL_MUX_SEL_VDD_OUT     = kD2257_ADC_CONTROL_MUX_SEL_VDD_MAIN,
    kDIALOG_ADC_CONTROL_MUX_SEL_BRICK_ID    = kD2257_ADC_CONTROL_MUX_SEL_BRICK_ID,
    kDIALOG_ADC_CONTROL_MUX_SEL_ACC_ID      = kD2257_ADC_CONTROL_MUX_SEL_ADC_IN7,

    kDIALOG_ADC_CONTROL_MUX_SEL_NTC0        = kD2257_ADC_CONTROL_MUX_SEL_TCAL,
    kDIALOG_ADC_CONTROL_MUX_NUM_NTC         = 8,

    kDIALOG_ADC_CONTROL_MUX_SEL_TINT_START  = kD2257_ADC_CONTROL_MUX_SEL_LDO10_TEMP,
    kDIALOG_ADC_CONTROL_MUX_SEL_TINT_END    = kD2257_ADC_CONTROL_MUX_SEL_BUCK35_TEMP,
    kDIALOG_ADC_CONTROL_MUX_SEL_TJUNC       = kD2257_ADC_CONTROL_MUX_SEL_TJNCT,
    kDIALOG_ADC_CONTROL_MUX_SEL_TBAT        = kD2257_ADC_CONTROL_MUX_SEL_TBAT,
    kDIALOG_ADC_CONTROL_MUX_SEL_VBAT        = kD2257_ADC_CONTROL_MUX_SEL_VBAT,
    kDIALOG_ADC_CONTROL_MUX_SEL_ICH         = kD2257_ADC_CONTROL_MUX_SEL_ICH_ADC_CH,

    // TODO: verify BRICK_ID_OFFSET_MV
    kDIALOG_ADC_BRICK_ID_OFFSET_MV          = 40,
};

enum {

//    kDIALOG_ADC_CONTROL_DEFAULTS = 0,
//    kDIALOG_ADC_CONTROL2_DEFAULTS = 0,
//    kDIALOG_ADC_CONTROL_ADC_REF_EN = 0,

    kDIALOG_ADC_LSB_MANADC_ERROR        = (1 << 7), // MAN1_INVALID
    kDIALOG_ADC_LSB_ADC_OVL             = (1 << 6), // Overflow

    kDIALOG_ADC_MAN_CTL               = kD2257_GPADC_MAN_CTRL1,
    kDIALOG_ADC_CONTROL_MUX_SEL_MASK  = kD2257_GPADC_MAN_CTRL1_MASK,
    kDIALOG_ADC_LSB                   = kD2257_GPADC_MAN1_RES_LSB,
    
    kDIALOG_ADC_FULL_SCALE_MV   = 2500,
    kDIALOG_ADC_RESOLUTION_BITS = 12,
};

// ------------------------------------------------
// GPIO
// ------------------------------------------------

enum {
    kDIALOG_RTC_CONTROL_MONITOR         = (1 << 0),
    kDIALOG_RTC_CONTROL_ALARM_EN        = (1 << 6),
};

enum {
    kD2257_GPIO_IO_CONFIG_IN          = (0<<6),
    kD2257_GPIO_IO_CONFIG_OUT_OD      = (1<<6),
    kD2257_GPIO_IO_CONFIG_OUT_PP      = (2<<6), 
// NOTE: Once a GPIO is set to a safe spare configuration, the safe spare setting must
// be changed  before any changes to the associated GPIO_x_IO_TYPE, GPIOx_PUPD and 
// GPIOx_WAKE_LVL register fields can be done.
    kD2257_GPIO_IO_CONFIG_SAFE_SPARE  = (3<<6),

    kD2257_GPIO_IO_TYPE_OUT_LVL         = (0<<3),
    kD2257_GPIO_IO_TYPE_OUT_32K         = (1<<3),
    kD2257_GPIO_IO_TYPE_OUT_nRST        = (2<<3),
    kD2257_GPIO_IO_TYPE_OUT_TEMP        = (3<<3),
    kD2257_GPIO_IO_TYPE_OUT_LVL2        = (4<<3),
    kD2257_GPIO_IO_TYPE_IN_LEVEL_HIGH   = (0<<3),
    kD2257_GPIO_IO_TYPE_IN_LEVEL_LOW    = (1<<3),
    kD2257_GPIO_IO_TYPE_IN_EDGE_RISING  = (2<<3),
    kD2257_GPIO_IO_TYPE_IN_EDGE_FALLING = (3<<3),
    kD2257_GPIO_IO_TYPE_IN_EDGE_ANY     = (4<<3),

    kDIALOG_SYS_GPIO_CONFIG_MASK          = (0x3<<6),
    kDIALOG_SYS_GPIO_DIRECTION_MASK       = (0x3<<3),
    kDIALOG_SYS_GPIO_VALUE_MASK           = (1<<0),

    kDIALOG_SYS_GPIO_DIRECTION_OUT              = kD2257_GPIO_IO_TYPE_OUT_LVL,
    kDIALOG_SYS_GPIO_DIRECTION_OUT_32KHZ        = kD2257_GPIO_IO_TYPE_OUT_32K,
    kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_HIGH    = kD2257_GPIO_IO_TYPE_IN_LEVEL_HIGH,
    kDIALOG_SYS_GPIO_DIRECTION_IN_LEVEL_LOW     = kD2257_GPIO_IO_TYPE_IN_LEVEL_LOW,

    kDIALOG_SYS_GPIO_OUTPUT_LEVEL_LOW       = (0 << 0),
    kDIALOG_SYS_GPIO_OUTPUT_LEVEL_HIGH      = (1 << 0),
    kDIALOG_SYS_GPIO_INPUT_WAKE             = (1 << 0),
};

// NOTE: it doesn't work with SAFESPARE
#define IS_GPIO_OUTPUT(gpio) \
    (((gpio)&kDIALOG_SYS_GPIO_CONFIG_MASK)!=kD2257_GPIO_IO_CONFIG_IN )

#define kDIALOG_STATUS_GPIO_MASK(gpio) \
    STATUS_FLAG_MAKE(6 + ((gpio)/8), (1 << ((gpio) % 8)) )

// ----------------------------------------------------------------------------
//  LDOS
// ----------------------------------------------------------------------------

#define NUM_LDOS 0x23
static const struct ldo_params ldo_D2257[NUM_LDOS] = {
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO1_VSEL,  kD2257_PWRONOFF_LDO1_EN,  1 },    // 00 - LDO1  =
    { 1200,  5, 0xff, 0xff,    2, kD2257_LDO2_VSEL,  kD2257_PWRONOFF_LDO2_EN,  1 },    // 01 - LDO2  =  bypass
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO3_VSEL,  kD2257_PWRONOFF_LDO3_EN,  1 },    // 02 - LDO3  =
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO4_VSEL,  kD2257_PWRONOFF_LDO4_EN,  1 },    // 03 - LDO4  =
    { 2400,  5, 0xff, 0xff, 0x00, kD2257_LDO5_VSEL,  kD2257_PWRONOFF_LDO5_EN,  1 },    // 04 - LDO5  =
    { 2500, 25, 0x7f, 0x7f,    2, kD2257_LDO6_VSEL,  kD2257_PWRONOFF_LDO6_EN,  1 },    // 05 - LDO6  = bypass 
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO7_VSEL,  kD2257_PWRONOFF_LDO7_EN,  1 },    // 06 - LDO7  =
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO8_VSEL,  kD2257_PWRONOFF_LDO8_EN,  1 },    // 07 - LDO8  =    
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO9_VSEL,  kD2257_PWRONOFF_LDO9_EN,  1 },    // 08 - LDO9  =
    {  600,  5, 0xff, 0xff, 0x00, kD2257_LDO10_VSEL, kD2257_PWRONOFF_LDO10_EN, 1 },    // 09 - LDO10 -
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO11_VSEL, kD2257_PWRONOFF_LDO11_EN, 1 },    // 0a - LDO11 =
    { 1800,  0,    0, 0,    0x00, 0, 0, 0 },                                        // 0b - there is no LDO12 (PP1V8_ALWAYS)    
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO13_VSEL, kD2257_PWRONOFF_LDO13_EN, 1 },    // 0c - LDO13 =
    {  600,  5, 0xff, 0xff, 0x00, kD2257_LDO14_VSEL, kD2257_PWRONOFF_LDO14_EN, 1 },    // 0d - LDO14 =
    {  600,  5, 0xff, 0xff, 0x00, kD2257_LDO15_VSEL, kD2257_PWRONOFF_LDO15_EN, 1 },    // 0e - LDO15 = 
    { 1200,  5, 0xff, 0xff, 0x00, kD2257_LDO16_VSEL, kD2257_PWRONOFF_LDO16_EN, 1 },    // 0f - LDO16 = 

    {  450,  5, 0, 0, 0x00, kD2257_BUCK0_VSEL, kD2257_PWRONOFF_BUCK0_EN, 1 },    // 10 - POWER_RAIL_CPU
    {  450,  5, 0, 0, 0x00, kD2257_BUCK1_VSEL, kD2257_PWRONOFF_BUCK1_EN, 1 },    // 11 - POWER_RAIL_GPU
    {  600,  5, 0, 0, 0x00, kD2257_BUCK2_VSEL, kD2257_PWRONOFF_BUCK2_EN, 1 },    // 12 - POWER_RAIL_SOC
    { 1100,  5, 0, 0, 0x00, kD2257_BUCK3_VSEL, kD2257_PWRONOFF_BUCK3_EN, 1 },    // 13
    {  600,  5, 0, 0, 0x00, kD2257_BUCK4_VSEL, kD2257_PWRONOFF_BUCK4_EN, 1 },    // 14
    {  600,  5, 0, 0, 0x00, kD2257_BUCK5_VSEL, kD2257_PWRONOFF_BUCK5_EN, 1 },    // 15 - POWER_RAIL_VDD_FIXED
    // NOTE: here bypass is the OFFSET of the bypass register from VSEL
    { 1800,  5, 0, 0, 0x00, kD2257_BUCK6_VSEL, kD2257_PWRONOFF_BUCK6_EN, 1 },    // 16 - BUCK6 (Byp)
    {  600,  5, 0, 0, 0x00, kD2257_BUCK7_VSEL, kD2257_PWRONOFF_BUCK7_EN, 1 },    // 17 - POWER_RAIL_CPU_RAM
    {  600,  5, 0, 0, 0x00, kD2257_BUCK8_VSEL, kD2257_PWRONOFF_BUCK8_EN, 1 },    // 18 - POWER_RAIL_GPU_RAM

    {  0,  0, 0, 0, 0x00, 0, kD2257_PWRONOFF_BUCK3_SW1_EN, 1 },                 // 19
    {  0,  0, 0, 0, 0x00, 0, kD2257_PWRONOFF_BUCK3_SW2_EN, 1 },                 // 1a
    {  0,  0, 0, 0, 0x00, 0, kD2257_PWRONOFF_BUCK3_SW3_EN, 1 },                 // 1b
    {  0,  0, 0, 0, 0x00, 0, kD2257_PWRONOFF_BUCK4_SW1_EN, 1 },                 // 1c
    {  0,  0, 0, 0, 0x00, 0, kD2257_PWRONOFF_BUCK4_SW2_EN, 1 },                 // 1d

    {  0,  0, 0, 0, 0x00, 0, kD2257_PWRONOFF_LCM_BST_SW_EN, 1 },                            // 1e
    
    {  5000, 50, 0x1f, 0x1f, 0x00, kD2257_LDO_LCM_BOOST_VSEL, kD2257_PWRONOFF_LCM_BST_EN, 1 },           // 1f
    {  5000, 50, 0x1f, 0x1f, 0x00, kD2257_LDO_LCM_LDO1_VSEL, kD2257_PWRONOFF_LCM_LDO1_EN, 1 },    // 20
    {  5000, 50, 0x1f, 0x1f, 0x00, kD2257_LDO_LCM_LDO2_VSEL, kD2257_PWRONOFF_LCM_LDO2_EN, 1 },    // 21
    {  0,     0,    0,    0, 0x00, 0,                        kD2257_PWRONOFF_LDO6_EN,  7<<1 },    // 22 LDO36 acc_sleep_pwr
    
};
#define LDOP ldo_D2257


// ----------------------------------------------------------------------------
//  Charge control
// ----------------------------------------------------------------------------

// TODO: verify these GUYS
enum {
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MIN  = 0x00,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_100  = 0x02,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_200  = 0x0a,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_300  = 0x12,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_400  = 0x1a,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_500  = 0x22,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_600  = 0x2a,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_700  = 0x32,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_900  = 0x42,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_975  = 0x48,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1000 = 0x4a,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1450 = 0x6e,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1500 = 0x72,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_1900 = 0x92,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2000 = 0x9a,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2100 = 0xa2,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_2300 = 0xb2,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MAX  = 0xff,
	kDIALOG_CHARGE_BUCK_CONTROL_ISET_BUCK_MASK = 0xff,
};

enum {
	kDIALOG_CHARGE_BUCK_CONTROL_MIN		   = 75,
	kDIALOG_CHARGE_BUCK_CONTROL_STEP_PER_100MA = 8,
	kDIALOG_CHARGE_BUCK_CONTROL_MAX		   = 3262,
};

enum {
	kDIALOG_CHARGE_CONTROL_A_ISET_BAT_MASK	 = 0x3f,
	kDIALOG_CHARGE_CONTROL_A_ISET_BAT_SHIFT  = 0,
	kDIALOG_OTP_ISET_BAT_SHIFT		= kDIALOG_CHARGE_CONTROL_A_ISET_BAT_SHIFT,
	kDIALOG_CHARGE_CONTROL_B_CHG_SUSP	= (1 << 6),
	kDIALOG_CHARGE_CONTROL_B_CHG_HIB	= (1 << 7),
	kDIALOG_CHARGE_CONTROL_CHG_HIB		= kDIALOG_CHARGE_CONTROL_B_CHG_HIB,
};

enum {
	kDIALOG_CHARGE_CONTROL_STEP		= 50,
	kDIALOG_CHARGE_CONTROL_MAX		= 3150,
};

enum {
    // CHG_CTRL_G
	kDIALOG_CHARGE_CONTROL_ALT_USB_DIS	= (1 << 6),
	kDIALOG_CHARGE_CONTROL_CHG_BUCK_EN	= (1 << 7),
};

enum {
	kDIALOG_CHARGE_CONTROL_TIME_TCTR_MASK	    = 0xF0,
	kDIALOG_CHARGE_CONTROL_TIME_TCTR_DISABLED   = 0x00,
	kDIALOG_CHARGE_CONTROL_TIME_PCTR_MASK	    = 0x07,
	kDIALOG_CHARGE_CONTROL_TIME_PCTR_DISABLED   = 0x00,
};

// ----------------------------------------------------------------------------
//  Buttons
// ----------------------------------------------------------------------------

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

#endif /* __DIALOG_D2257_H */
