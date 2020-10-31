// *****************************************************************************
//
// File: H2fmi_debug.c
//
// *****************************************************************************
//
// Notes:
//
// *****************************************************************************
//
// Copyright (C) 2008 Apple Computer, Inc. All rights reserved.
//
// This document is the property of Apple Computer, Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form,
// in whole or in part, without the express written permission of
// Apple Computer, Inc.
//
// *****************************************************************************

#include "H2fmi_private.h"

// =============================================================================
// public implementation function definitions
// =============================================================================

#if (H2FMI_TRACE_REG_WRITES)
UInt32 h2fmi_trace_wr(h2fmi_t* fmi,
                      UInt32   reg,
                      UInt32   val)
{
    const UInt32 offset = reg >> 2;

    WMR_PRINT(MISC, "*%p = 0x%08X\n", ((UInt32)fmi->regs) + reg, val);
    fmi->regs[offset] = val;
    return val;
}

UInt8 h2fmi_trace_wr8(h2fmi_t* fmi,
                      UInt32   reg,
                      UInt8    val)
{
    const UInt32 offset = reg;

    WMR_PRINT(MISC, "*%p = 0x%02X\n", ((UInt32)fmi->regs) + reg, val);
    ((UInt8*)fmi->regs)[offset] = val;
    return val;
}

UInt32 h2fmc_trace_wr(volatile UInt32* h2fmc_regs,
                      UInt32           reg,
                      UInt32           val)
{
    const UInt32 offset = reg >> 2;

    WMR_PRINT(MISC, "*%p = 0x%08X\n", ((UInt32)h2fmc_regs) + reg, val);
    h2fmc_regs[offset] = val;
    return val;
}
#endif // H2FMI_TRACE_REG_WRITES

#if (H2FMI_TRACE_REG_READS)
UInt32 h2fmi_trace_rd(h2fmi_t* fmi,
                      UInt32   reg)
{
    const UInt32 offset = reg >> 2;
    const UInt32 val = fmi->regs[offset];

    WMR_PRINT(MISC, "*%p == 0x%08X\n", ((UInt32)fmi->regs) + reg, val);
    return val;
}

UInt8 h2fmi_trace_rd8(h2fmi_t* fmi,
                      UInt32   reg)
{
    const UInt32 offset = reg;
    const UInt8 val = ((volatile UInt8*)fmi->regs)[offset];

    WMR_PRINT(MISC, "*%p == 0x%02X\n", ((UInt32)fmi->regs) + reg, val);
    return val;
}
#endif // H2FMI_TRACE_REG_READS

#if (H2FMI_DEBUG)

void h2fmi_spew_config(h2fmi_t* fmi)
{
    WMR_PRINT(INF, "fmi->regs: %p\n", fmi->regs);
    WMR_PRINT(INF, "fmi->num_of_ce: %p\n", fmi->num_of_ce);
    WMR_PRINT(INF, "fmi->pages_per_block: %p\n", fmi->pages_per_block);
    WMR_PRINT(INF, "fmi->sectors_per_page: %p\n", fmi->sectors_per_page);
    WMR_PRINT(INF, "fmi->bytes_per_spare: %p\n", fmi->bytes_per_spare);
    WMR_PRINT(INF, "fmi->blocks_per_ce: %p\n", fmi->blocks_per_ce);
    WMR_PRINT(INF, "fmi->total_bytes_per_meta: %p\n", fmi->total_bytes_per_meta);
    WMR_PRINT(INF, "fmi->valid_bytes_per_meta: %p\n", fmi->valid_bytes_per_meta);
    WMR_PRINT(INF, "fmi->if_ctrl: %p\n", fmi->if_ctrl);
#if SUPPORT_TOGGLE_NAND
    WMR_PRINT(INF, "fmi->toggle_if_ctrl: %p\n", fmi->toggle_if_ctrl);
#endif
    WMR_PRINT(INF, "fmi->bytes_per_page  : %p\n", fmi->bytes_per_page);
}

void h2fmi_spew_status_regs(h2fmi_t*    fmi,
                            const char* prefix)
{
    WMR_PRINT(INF, "%sFMI_INT_PEND:       0x%08X\n", prefix, h2fmi_rd(fmi, FMI_INT_PEND));
    WMR_PRINT(INF, "%sFMC_STATUS:         0x%08X\n", prefix, h2fmi_rd(fmi, FMC_STATUS));

    // ECC might be clocked off for PPN
    if (!fmi->is_ppn)
    {
        WMR_PRINT(INF, "%sECC_PND:            0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PND));
    }
    else
    {
        WMR_PRINT(INF, "%s: omitting ecc registers for ppn device\n", prefix);
    }
}

void h2fmi_spew_fmi_regs(h2fmi_t*    fmi,
                         const char* prefix)
{
    WMR_PRINT(INF, "%sFMI_CONFIG:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_CONFIG));
    WMR_PRINT(INF, "%sFMI_CONTROL:        0x%08X\n", prefix, h2fmi_rd(fmi, FMI_CONTROL));
    WMR_PRINT(INF, "%sFMI_STATUS:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_STATUS));
    WMR_PRINT(INF, "%sFMI_INT_PEND:       0x%08X\n", prefix, h2fmi_rd(fmi, FMI_INT_PEND));
    WMR_PRINT(INF, "%sFMI_INT_EN:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_INT_EN));

    // can't read from fifos, since that would alter state of block

    WMR_PRINT(INF, "%sFMI_DEBUG0:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_DEBUG0));
#if FMI_VERSION <= 2
    WMR_PRINT(INF, "%sFMI_DEBUG1:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_DEBUG1));
    WMR_PRINT(INF, "%sFMI_DEBUG2:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_DEBUG2));
#endif

#if FMI_VERSION > 0

    WMR_PRINT(INF, "%sFMI_HW_VERSION:     0x%08X\n", prefix, h2fmi_rd(fmi, FMI_HW_VERSION));
#if FMI_VERSION <= 2
    WMR_PRINT(INF, "%sFMI_DEBUG3:         0x%08X\n", prefix, h2fmi_rd(fmi, FMI_DEBUG3));
#endif
    WMR_PRINT(INF, "%sFMI_DATA_SIZE:      0x%08X\n", prefix, h2fmi_rd(fmi, FMI_DATA_SIZE));
#endif
}

void h2fmi_spew_fmc_regs(h2fmi_t*    fmi,
                         const char* prefix)
{
    WMR_PRINT(INF, "%sFMC_ON:             0x%08X\n", prefix, h2fmi_rd(fmi, FMC_ON));
    WMR_PRINT(INF, "%sFMC_VER:            0x%08X\n", prefix, h2fmi_rd(fmi, FMC_VER));
    WMR_PRINT(INF, "%sFMC_IF_CTRL:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_IF_CTRL));
    WMR_PRINT(INF, "%sFMC_CE_CTRL:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_CE_CTRL));
    WMR_PRINT(INF, "%sFMC_RW_CTRL:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_RW_CTRL));
    WMR_PRINT(INF, "%sFMC_CMD:            0x%08X\n", prefix, h2fmi_rd(fmi, FMC_CMD));
    WMR_PRINT(INF, "%sFMC_ADDR0:          0x%08X\n", prefix, h2fmi_rd(fmi, FMC_ADDR0));
    WMR_PRINT(INF, "%sFMC_ADDR1:          0x%08X\n", prefix, h2fmi_rd(fmi, FMC_ADDR1));
    WMR_PRINT(INF, "%sFMC_ADDRNUM:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_ADDRNUM));
    WMR_PRINT(INF, "%sFMC_DATANUM:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_DATANUM));
    WMR_PRINT(INF, "%sFMC_TO_CTRL:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_TO_CTRL));
    WMR_PRINT(INF, "%sFMC_TO_CNT:         0x%08X\n", prefix, h2fmi_rd(fmi, FMC_TO_CNT));
    WMR_PRINT(INF, "%sFMC_INTMASK:        0x%08X\n", prefix, h2fmi_rd(fmi, FMC_INTMASK));
    WMR_PRINT(INF, "%sFMC_STATUS:         0x%08X\n", prefix, h2fmi_rd(fmi, FMC_STATUS));
    WMR_PRINT(INF, "%sFMC_NAND_STATUS:    0x%08X\n", prefix, h2fmi_rd(fmi, FMC_NAND_STATUS));
    WMR_PRINT(INF, "%sFMC_RBB_CONFIG:     0x%08X\n", prefix, h2fmi_rd(fmi, FMC_RBB_CONFIG));
#if SUPPORT_TOGGLE_NAND
    WMR_PRINT(INF, "%sFMC_DQS_TIMING_CTRL:0x%08X\n", prefix, h2fmi_rd(fmi, FMC_DQS_TIMING_CTRL));
    WMR_PRINT(INF, "%sFMC_TOGGLE_CTRL_1:  0x%08X\n", prefix, h2fmi_rd(fmi, FMC_TOGGLE_CTRL_1));
    WMR_PRINT(INF, "%sFMC_TOGGLE_CTRL_2:  0x%08X\n", prefix, h2fmi_rd(fmi, FMC_TOGGLE_CTRL_2));
#endif
}

void h2fmi_spew_ecc_regs(h2fmi_t*    fmi,
                         const char* prefix)
{
    // ECC might be clocked off for PPN
    if (!fmi->is_ppn)
    {
        WMR_PRINT(INF, "%sECC_VERSION:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_VERSION));
        WMR_PRINT(INF, "%sECC_CON0:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_CON0));
        WMR_PRINT(INF, "%sECC_CON1:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_CON1));

        // ECC_RESULT is a fifo head; reading from it changes state, and therefore it won't be
        // included in the spew function
        //WMR_PRINT(INF, "%sECC_RESULT:         0x%08X\n", prefix, h2fmi_rd(fmi, ECC_RESULT));

        WMR_PRINT(INF, "%sECC_PND:            0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PND));
        WMR_PRINT(INF, "%sECC_MASK:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_MASK));
        WMR_PRINT(INF, "%sECC_HISTORY:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_HISTORY));
        WMR_PRINT(INF, "%sECC_CORE_SW_RESET:  0x%08X\n", prefix, h2fmi_rd(fmi, ECC_CORE_SW_RESET));
#if FMI_VERSION <= 2
        WMR_PRINT(INF, "%sECC_LOC0:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC0));
        WMR_PRINT(INF, "%sECC_LOC1:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC1));
        WMR_PRINT(INF, "%sECC_LOC2:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC2));
        WMR_PRINT(INF, "%sECC_LOC3:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC3));
        WMR_PRINT(INF, "%sECC_LOC4:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC4));
        WMR_PRINT(INF, "%sECC_LOC5:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC5));
        WMR_PRINT(INF, "%sECC_LOC6:           0x%08X\n", prefix, h2fmi_rd(fmi, ECC_LOC6));
        WMR_PRINT(INF, "%sECC_PARITY0:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY0));
        WMR_PRINT(INF, "%sECC_PARITY1:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY1));
        WMR_PRINT(INF, "%sECC_PARITY2:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY2));
        WMR_PRINT(INF, "%sECC_PARITY3:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY3));
        WMR_PRINT(INF, "%sECC_PARITY4:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY4));
        WMR_PRINT(INF, "%sECC_PARITY5:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY5));
        WMR_PRINT(INF, "%sECC_PARITY6:        0x%08X\n", prefix, h2fmi_rd(fmi, ECC_PARITY6));
#endif
    }
    else
    {
        WMR_PRINT(INF, "%s: omitting ecc registers for ppn device\n", prefix);
    }
}

void h2fmi_spew_regs(h2fmi_t*    fmi,
                     const char* prefix)
{
    h2fmi_spew_fmi_regs(fmi, prefix);
    h2fmi_spew_fmc_regs(fmi, prefix);
    h2fmi_spew_ecc_regs(fmi, prefix);
}

void h2fmi_spew_buffer(UInt8* buf,
                       UInt32 size)
{
    UInt32 idx;

    for (idx = 0; idx < size; idx++)
    {
        if (!idx)
        {
            WMR_PRINT(INF, "0x%08X: ", (UInt32)buf);
        }
        else if (!(idx % 16))
        {
            WMR_PRINT(INF, "\n0x%08X: ", ((UInt32)buf) + idx);
        }
        else if (!(idx % 4))
        {
            WMR_PRINT(INF, " ");
        }
        WMR_PRINT(INF, "%02X", buf[idx]);
    }
    WMR_PRINT(INF, "\n");
}

#endif // H2FMI_DEBUG

// ********************************** EOF **************************************
