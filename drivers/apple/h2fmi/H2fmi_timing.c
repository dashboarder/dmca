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

#include "H2fmi_timing.h"
#include "WMROAM.h" // WMR_MAX, ROUNDUPTO, 


#if 0
#define DUMP_VAR(variable) WMR_PRINT(ALWAYS, "%s:%d %s : %u\n", \
                         __FUNCTION__, __LINE__, #variable, (UInt32)(variable))
#else
#define DUMP_VAR(variable)
#endif

// timings required by PPN 1.5 spec
#define FPGA_TIMINGS (0)

#if FPGA_TIMINGS  // Based on SAMSUNG PPN 1.5 FPGA (will be removed once we get PPN 1.5 Si)

#define WH_MIN_NS (35)
#define WP_MIN_NS (65)
#define WC_MIN_NS (100)

#define RPRE_MIN_NS (116)
#define WPRE_MIN_NS (76)
#define WPST_MIN_PS (1888000)
#define DQSRE_MAX_PS (50 * 1000)

#define CS_MIN_NS (383)
#define CH_MIN_NS  (5)
#define ADL_MIN_NS (300)
#define WHR_MIN_NS (120)

#else // these are timings specified by PPN 1.5 spec

#define RPRE_MIN_NS (35)
#define WPRE_MIN_NS (15)
#define WPST_MIN_PS (6500)
#define DQSRE_MAX_PS (25 * 1000)

#define CS_MIN_NS (20)
#define CH_MIN_NS  (5)
#define ADL_MIN_NS (300)
#define WHR_MIN_NS (120)

#define RPSTH_MIN_NS (15)
#endif

#if FMI_VERSION >= 5
#define RPST_MAX_CYCLES (5)
#endif // FMI_VERSION >= 5

Int32 HertzToPicoseconds
    (UInt64 hertz)
{
    const UInt32 KHz = hertz / 1000;

    const UInt32 ps = ((UInt32)1000000000) / KHz;
    
    return ps;
}

UInt32
NandRequirementToIFCTRL (
    const NandRequiredTimings *required,
    NandFmcSettings *actual)
{
    DUMP_VAR(required->clock_hz);
    DUMP_VAR(required->tRC_ns);
    DUMP_VAR(required->tRP_ns);
    DUMP_VAR(required->tREH_ns);
    DUMP_VAR(required->tREA_ns);
    DUMP_VAR(required->tRHOH_ns);
    DUMP_VAR(required->tRLOH_ns);
    DUMP_VAR(required->tWC_ns);
    DUMP_VAR(required->tWP_ns);
    DUMP_VAR(required->tWH_ns);
    
    // avoid using 
    const Int32 ps_per_clock = HertzToPicoseconds(required->clock_hz);

    // Minimum time from SoC falling edge until first possible data sample
    const Int32 data_sample_time = ROUNDUPTO((required->tREA_ns + required->soc_tx_prop_ns + required->soc_rx_prop_ns) * PS_PER_NS, ps_per_clock);
    const Int32 data_sample_clocks = data_sample_time / ps_per_clock;
    DUMP_VAR(data_sample_clocks);
    
    const Int32 reb_setup_time = ROUNDUPTO(WMR_MAX((required->tRP_ns * PS_PER_NS), (data_sample_time - (required->tRHOH_ns * PS_PER_NS))), ps_per_clock);
    Int32 reb_setup_clocks = reb_setup_time / ps_per_clock;
    DUMP_VAR(reb_setup_clocks);
    
    Int32 reb_high_estimate_clocks = 0;
    
    // Minimum cycle time as determined by actual setup plus required hold vs. tRC
    const Int32 reb_adjusted_cycle_time = ROUNDUPTO(WMR_MAX((required->tREH_ns * PS_PER_NS) + reb_setup_time, (required->tRC_ns * PS_PER_NS)), ps_per_clock);
    DUMP_VAR(reb_adjusted_cycle_time);
    
    const Int32 tRLOH_clocks = ROUNDUPTO(required->tRLOH_ns * PS_PER_NS, ps_per_clock) / ps_per_clock;
    // Check for EDO mode (data ready time > actual cycle + tRLOH)
    if (data_sample_time > (reb_adjusted_cycle_time + ROUNDUPTO(required->tRLOH_ns * PS_PER_NS, ps_per_clock)))
    {
        WMR_ASSERT(data_sample_clocks > (tRLOH_clocks + reb_setup_clocks));
        reb_high_estimate_clocks = data_sample_clocks - tRLOH_clocks - reb_setup_clocks;
    }
    else
    {
        reb_high_estimate_clocks =  ROUNDUPTO(WMR_MAX((required->tREH_ns * PS_PER_NS), ((required->tRC_ns * PS_PER_NS) - reb_setup_time)), ps_per_clock) / ps_per_clock; 
    }
    DUMP_VAR(reb_high_estimate_clocks);
    
    Int32 reb_high_revised_clocks = 0;
    
    // data sampling is done with a delay relative to the risine edge of REB
    // thus, the sampling cannot occur past the rising edge of the subsequent cycle
    if (((data_sample_clocks - reb_setup_clocks) <= (reb_setup_clocks + reb_high_estimate_clocks - 1)))
    {
        reb_high_revised_clocks = reb_high_estimate_clocks;
    }
    else
    {
        reb_setup_clocks = data_sample_clocks;
    }
    
    DUMP_VAR(reb_high_revised_clocks);
    
    Int32 reb_setup_final_clocks = reb_setup_clocks;
    Int32 reb_high_final_clocks = reb_high_revised_clocks;
    
    // Shift extra cycles to setup phase if possible
    if ((reb_high_revised_clocks > 1) && (((reb_high_revised_clocks - 1) * ps_per_clock) > (required->tREH_ns * PS_PER_NS)))
    {
        reb_high_final_clocks -= 1;
        reb_setup_final_clocks += 1;
    }

    actual->read_setup_clocks = reb_setup_final_clocks > 0 ? reb_setup_final_clocks - 1 : 0;
    actual->read_hold_clocks = reb_high_final_clocks > 0 ? reb_high_final_clocks - 1 : 0;
    actual->read_dccycle_clocks = data_sample_clocks > reb_setup_final_clocks ? data_sample_clocks - reb_setup_final_clocks : 0;

    // Some chips cannot sample after the next byte's /RE - rdar://7063954
    if (required->edo_limit && 
        ((actual->read_setup_clocks + actual->read_hold_clocks + 1) < actual->read_dccycle_clocks))
    {
        return FALSE32;
    }

    const Int32 write_hold_time = ROUNDUPTO(required->tWH_ns * PS_PER_NS, ps_per_clock);
    
    const Int32 write_setup_time = ROUNDUPTO(WMR_MAX(((required->tWC_ns * PS_PER_NS) - write_hold_time), (required->tWP_ns * PS_PER_NS)), ps_per_clock);
    
    Int32 write_hold_revised_time = write_hold_time;
    Int32 write_setup_revised_time = write_setup_time;

    // Balance write setup and hold if balance_write_cycle is set
    if (required->balance_write_cycle && 
        ((write_setup_time - ps_per_clock) > (required->tWP_ns * PS_PER_NS)))
    {
        write_hold_revised_time += ps_per_clock;
        write_setup_revised_time -= ps_per_clock;
    }
    
    actual->write_setup_clocks = (write_setup_revised_time / ps_per_clock) - 1;
    actual->write_hold_clocks = (write_hold_revised_time / ps_per_clock) - 1;

    return TRUE32;
}

static UInt32 
h2fmi_nanos_to_clocks(UInt8 nsPerClock,
                      UInt8 nsPerOperation)
{
    UInt8 rawValue      = (nsPerOperation / nsPerClock) + ((nsPerOperation % nsPerClock) ? 1 : 0);
    UInt8 registerValue = (rawValue ? rawValue - 1 : 0);
    
    return registerValue;    
}


BOOL32
Legacy_NandRequirementToIFCTRL (
    const NandRequiredTimings *required,
    NandFmcSettings *actual)
{

#define timing_nanos(cycles, hz) ((((UInt64)1000000000) * cycles) / hz)
#define SUB_MIN_ZERO(minuend, subtrahend)  (((minuend) > (subtrahend)) ? ((minuend) - (subtrahend)) : 0)
    UInt32        nsAdjustedWriteSetup;
    UInt32        nsAdjustedWriteHold;
    UInt32        nsWriteCycleRemainder;
    UInt32        nsAdjustedReadSetup;
    UInt32        nsAdjustedReadHold;
    UInt32        nsActualReadSetup;
    UInt32        nsAdjustedReadCycle;
    UInt32        nsReadCycleRemainder;
    UInt32        nsAdjustedReadSample;

    UInt32        write_setup_clocks;
    UInt32        write_hold_clocks;
    UInt32        read_setup_clocks;
    UInt32        read_hold_clocks;
    UInt32        edo_delay;
    
    const UInt32  nsNandRiseTime = required->nand_to_soc_rise_ns;
    const UInt32  nsSocRiseTime  = required->soc_to_nand_rise_ns;
    const UInt32  nsSocFallTime  = required->soc_to_nand_fall_ns;
    
    Int32    nsPerClock;

    nsPerClock = HertzToPicoseconds(required->clock_hz) / 1000;

    nsAdjustedWriteSetup = required->tWP_ns + nsSocFallTime;
    write_setup_clocks   = h2fmi_nanos_to_clocks(nsPerClock, nsAdjustedWriteSetup);

    // Check if cycle time remaining or write hold requirement is greater
    nsWriteCycleRemainder = SUB_MIN_ZERO((UInt32)required->tWC_ns,(write_setup_clocks * nsPerClock));
    nsAdjustedWriteHold   = WMR_MAX(nsWriteCycleRemainder, required->tWH_ns + nsSocRiseTime);
    write_hold_clocks     = h2fmi_nanos_to_clocks(nsPerClock, nsAdjustedWriteHold);

    nsAdjustedReadSetup   = required->tRP_ns + nsSocFallTime;
    read_setup_clocks     = h2fmi_nanos_to_clocks(nsPerClock, nsAdjustedReadSetup);    
    nsActualReadSetup     = (read_setup_clocks + 1) * nsPerClock;

    // Since read has round-trip rise/fall delays, account for both
    nsAdjustedReadCycle   = WMR_MAX((UInt32)required->tRC_ns,
                                nsSocFallTime + required->tREA_ns + nsNandRiseTime);
    nsReadCycleRemainder  = SUB_MIN_ZERO((UInt32)nsAdjustedReadCycle, nsActualReadSetup);

    nsAdjustedReadHold    = WMR_MAX(nsReadCycleRemainder, required->tREH_ns + nsSocRiseTime);
    read_hold_clocks      = h2fmi_nanos_to_clocks(nsPerClock, nsAdjustedReadHold);    

    // Make sure we sample tREA from fall, but before tRHOH from rise
    nsAdjustedReadSample = SUB_MIN_ZERO(nsSocFallTime + required->tREA_ns+ nsNandRiseTime,
                                            nsActualReadSetup);
    edo_delay = ROUNDUPTO(nsAdjustedReadSample, nsPerClock) / nsPerClock; // how many cycles after the falling edge to sample

    
    actual->read_setup_clocks = read_setup_clocks;
    actual->read_hold_clocks = read_hold_clocks;
    actual->read_dccycle_clocks = edo_delay;
    actual->write_setup_clocks = write_setup_clocks;
    actual->write_hold_clocks = write_hold_clocks;
    
    return TRUE32;
}

#if SUPPORT_TOGGLE_NAND
BOOL32 Toggle_NandRequirementToIFCTRL (
    const NandRequiredTimings *required,
    NandFmcSettings *actual
)
{

    UInt64 clk_Hz = required->clock_hz;
    UInt32 toggleCycleTimePS = WMR_MAX(WMR_MAX(WMR_MAX(required->ddr_tREH_ps * 2, required->ddr_tRP_ps * 2),
                                               WMR_MAX(required->tDQSH_ps * 2, required->tDQSL_ps * 2)),
                                       WMR_MAX(required->tDSC_ps, required->ddr_tRC_ps));

    actual->ddr_half_cycle_clocks = CEIL_DIV(toggleCycleTimePS * clk_Hz, 2 * PS_PER_S) - 1;
    
    actual->read_pre_clocks = CEIL_DIV(RPRE_MIN_NS * clk_Hz, NS_PER_S) - 1;
#if FMI_VERSION < 5
    actual->read_post_clocks = CEIL_DIV((DQSRE_MAX_PS + toggleCycleTimePS / 2) * clk_Hz, PS_PER_S) - 1;
#else // FMI_VERSION >= 5
    // <rdar://problem/11024771> Change the read post amble delay to workaround Read dma timeout with DQS_TIMING_CTRL::DELTAV_SUSPENDS_READS
    actual->read_post_clocks = RPST_MAX_CYCLES;
#endif // FMI_VERSION >= 5
    actual->write_pre_clocks = CEIL_DIV(WPRE_MIN_NS * clk_Hz, NS_PER_S) - 1;
    actual->write_post_clocks = CEIL_DIV(WPST_MIN_PS * clk_Hz, PS_PER_S) - 1;
    
    actual->ce_setup_clocks = CEIL_DIV(CS_MIN_NS * clk_Hz, NS_PER_S) - 1;
    //H5P violates tRPSTH. SoC team recommends increasing CE Hold Time as a workaround
    //to increase tRPSTH: <rdar://problem/9894530> PPN1.5: tRPSTH is being violated
    actual->ce_hold_clocks = CEIL_DIV(WMR_MAX(CH_MIN_NS, RPSTH_MIN_NS) * clk_Hz, NS_PER_S) - 1;
    actual->adl_clocks = CEIL_DIV(ADL_MIN_NS * clk_Hz, NS_PER_S) - 1;
    actual->whr_clocks = CEIL_DIV(WHR_MIN_NS * clk_Hz, NS_PER_S) - 1;

    return TRUE32;

}
#endif
