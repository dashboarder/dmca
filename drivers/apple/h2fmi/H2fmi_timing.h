// Copyright (C) 2010 Apple, Inc. All rights reserved.
//
// This document is the property of Apple, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple, Inc.
//
//
// Note: This file is intended to compile for an iBoot and x86 host target

#ifndef _H2FMI_TIMING_H_
#define _H2FMI_TIMING_H_

#include "WMRTypes.h"
#include "H2fmi_private.h"


#define CEIL_DIV(a,b) (((a)/(b)) + (((a) % (b)) ? 1 : 0))

#define PS_PER_NS (1000)
#define NS_PER_S ((UInt64)1000000000)
#define PS_PER_S ((UInt64)1000000000000)

typedef struct _NandRequiredTimings
{
    UInt64 clock_hz;    

    Int32 soc_tx_prop_ns;
    Int32 soc_rx_prop_ns;
    BOOL32 edo_limit;  // TRUE32 if using H2P/H2A/H3P for rdar://7063954
    BOOL32 balance_write_cycle;

    // -- Legacy only
    Int32 soc_to_nand_rise_ns;
    Int32 soc_to_nand_fall_ns;
    Int32 nand_to_soc_rise_ns;
    Int32 nand_to_soc_fall_ns;
    // -- End Legacy only

    Int32 tRC_ns;
    Int32 tRP_ns;
    Int32 tREH_ns;
    Int32 tREA_ns;
    Int32 tRHOH_ns;
    Int32 tRLOH_ns;

    Int32 tWC_ns;
    Int32 tWP_ns;
    Int32 tWH_ns;

#if SUPPORT_TOGGLE_NAND
    UInt32 ddr_tRC_ps;
    UInt32 ddr_tREH_ps;
    UInt32 ddr_tRP_ps;

    UInt32 tDSC_ps;
    UInt32 tDQSH_ps;
    UInt32 tDQSL_ps;
#endif


} NandRequiredTimings;

typedef struct _NandFmcSettings
{
    UInt8 read_setup_clocks;   // 0 is one cycle
    UInt8 read_hold_clocks;    // 0 is one cycle
    UInt8 read_dccycle_clocks; // 0 means rising edge of /RE

    UInt8 write_setup_clocks;  // 0 is one cycle
    UInt8 write_hold_clocks;   // 0 is one cycle

#if SUPPORT_TOGGLE_NAND
    UInt8 ddr_half_cycle_clocks;

    UInt8 read_pre_clocks;
    UInt8 read_post_clocks;
    UInt8 write_pre_clocks;
    UInt8 write_post_clocks;

    UInt8 ce_setup_clocks;
    UInt8 ce_hold_clocks;
    UInt8 adl_clocks;
    UInt8 whr_clocks;
#endif
} NandFmcSettings;

BOOL32
NandRequirementToIFCTRL (
    const NandRequiredTimings *required,
    NandFmcSettings *actual
);

BOOL32
Legacy_NandRequirementToIFCTRL (
    const NandRequiredTimings *required,
    NandFmcSettings *actual
);

#if SUPPORT_TOGGLE_NAND
BOOL32
Toggle_NandRequirementToIFCTRL (
    const NandRequiredTimings *required,
    NandFmcSettings *actual
);
#endif

#endif // _H2FMI_TIMING_H_
